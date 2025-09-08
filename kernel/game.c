#include "game.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "player.h"
#include "timer.h"
#include "../assets/level1_map.h"
#include "../assets/shoot_chicken.h"
#include "../assets/bullet.h"
#include "../assets/bee.h"  
#include "../assets/fox.h" 
#include "../assets/snake.h" 
#include "../assets/box.h" 
#include "../assets/kfc.h" 
#include "../assets/win.h" 
#include "../assets/lose.h" 

#define SCREEN_WIDTH  500
#define SCREEN_HEIGHT 500

#define MAX_BULLETS 20
#define BULLET_SPEED 1
#define BULLET_SIZE 50

// Ensure PLAYER_WIDTH/HEIGHT are set
#ifndef PLAYER_WIDTH
#define PLAYER_WIDTH  SHOOT_CHICKEN_WIDTH
#endif
#ifndef PLAYER_HEIGHT
#define PLAYER_HEIGHT SHOOT_CHICKEN_HEIGHT
#endif

// Enemy draw size (adjust if your bee asset uses different values)
#define ENEMY_WIDTH  200
#define ENEMY_HEIGHT 200

// Bullet structure
typedef struct {
    int x;      // world x
    int y;      // screen y
    int active;
    int direction; // 1 for right, -1 for left
} Bullet;

// Global bullet array
Bullet bullets[MAX_BULLETS];

// Enemy structure
typedef struct {
    int x;      // world x
    int y;      // screen y
    int hp;
    int active;
    int timer;      // ms if you want
    int move_timer; // ms if you want
} Enemy;

Enemy enemy; // single enemy for now
const uint32_t* ENEMY_SPRITES[5] = { fox, snake, box, kfc, bee }; // array of enemy sprites

// ---------------- Side-scroller Demo ----------------
// clear rect from background using camera_x (world -> screen)
void clearRectWithBackground(int x, int y, int width, int height, int camera_x) {
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    if (width <= 0 || height <= 0) return;

    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int screen_x = x + dx;
            int screen_y = y + dy;
            int map_x = camera_x + screen_x;
            int map_y = screen_y;
            if (map_x < 0) map_x = 0;
            if (map_x >= MAP_WIDTH) map_x = MAP_WIDTH - 1;
            if (map_y < 0 || map_y >= SCREEN_HEIGHT) continue;
            uint32_t pixel = level1_map[map_y * MAP_WIDTH + map_x];
            unsigned char r = (pixel >> 24) & 0xFF;
            unsigned char g = (pixel >> 16) & 0xFF;
            unsigned char b = (pixel >> 8) & 0xFF;
            unsigned char a = pixel & 0xFF;
            drawPixelRGBA32(screen_x, screen_y, r, g, b, a);
        }
    }
}

void draw_map(int camera_x) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int map_x = camera_x + x;
            int map_y = y;
            if (map_x < 0) map_x = 0;
            if (map_x >= MAP_WIDTH) map_x = MAP_WIDTH - 1;

            uint32_t pixel = level1_map[map_y * MAP_WIDTH + map_x];

            unsigned char r = (pixel >> 24) & 0xFF;
            unsigned char g = (pixel >> 16) & 0xFF;
            unsigned char b = (pixel >> 8) & 0xFF;
            unsigned char a = pixel & 0xFF;

            drawPixelRGBA32(x, y, r, g, b, a);
        }
    }
}

void handleJumping(int *player_y, int *jumping, int *jump_velocity) {
    const int GRAVITY = 2;
    const int GROUND_Y = 250;
    if (*jumping) {
        *player_y += *jump_velocity;
        *jump_velocity += GRAVITY;
        if (*player_y >= GROUND_Y) {
            *player_y = GROUND_Y;
            *jumping = 0;
            *jump_velocity = 0;
        }
    }
}

void initBullets() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
        bullets[i].x = 0;
        bullets[i].y = 0;
        bullets[i].direction = 1;
    }
}

void shootBullet(int player_x, int player_y, int camera_x, int facing_direction) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            bullets[i].x = player_x + camera_x + (facing_direction > 0 ? PLAYER_WIDTH : 0) + 150;
            bullets[i].y = (player_y + PLAYER_HEIGHT / 3) + 50;
            bullets[i].direction = facing_direction;
            break;
        }
    }
}

void updateBullets(int camera_x) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            bullets[i].x += BULLET_SPEED * bullets[i].direction;

            // Check bullet off-screen
            if (bullets[i].x < 0 || bullets[i].x > MAP_WIDTH) {
                bullets[i].active = 0;
                continue;
            }

            // --- Check collision with enemy ---
            if (enemy.active) {
                int bullet_screen_x = bullets[i].x - camera_x;
                int bullet_screen_y = bullets[i].y;

                int enemy_screen_x = enemy.x - camera_x;
                int enemy_screen_y = enemy.y;

                // Simple AABB (axis-aligned bounding box) collision
                if (bullet_screen_x + BULLET_SIZE > enemy_screen_x &&
                    bullet_screen_x < enemy_screen_x + ENEMY_WIDTH &&
                    bullet_screen_y + BULLET_SIZE > enemy_screen_y &&
                    bullet_screen_y < enemy_screen_y + ENEMY_HEIGHT) {

                    bullets[i].active = 0;   // bullet stops
                    enemy.hp--;              // reduce HP
                    if (enemy.hp <= 1) {
                        enemy.active = 0;    // enemy disappears if HP 0
                    }
                    continue; // no further bullet movement
                }
            }

            // Deactivate bullets far off-screen
            int screen_x = bullets[i].x - camera_x;
            if (screen_x < -100 || screen_x > SCREEN_WIDTH + 100) {
                bullets[i].active = 0;
            }
        }
    }
}

void drawEnemy(int camera_x, int enemy_type) {
    if (!enemy.active) return;

    int enemy_screen_x = enemy.x - camera_x;
    int enemy_screen_y = enemy.y;
    
    // Calculate clipping bounds
    int src_start_x = 0;  // source sprite x offset
    int dst_start_x = enemy_screen_x;  // destination screen x
    int draw_width = ENEMY_WIDTH;
    
    // Clip left side
    if (enemy_screen_x < 0) {
        src_start_x = -enemy_screen_x;  // skip pixels from left of sprite
        dst_start_x = 0;                // start drawing at screen edge
        draw_width += enemy_screen_x;   // reduce width
    }
    
    // Clip right side
    if (dst_start_x + draw_width > SCREEN_WIDTH) {
        draw_width = SCREEN_WIDTH - dst_start_x;
    }
    
    // Draw the clipped sprite pixel by pixel
    if (draw_width > 0 && enemy_screen_y >= 0 && enemy_screen_y < SCREEN_HEIGHT - ENEMY_HEIGHT) {
        for (int y = 0; y < ENEMY_HEIGHT; y++) {
            for (int x = 0; x < draw_width; x++) {
                int src_x = src_start_x + x;
                int src_y = y;
                int dst_x = dst_start_x + x;
                int dst_y = enemy_screen_y + y;
                
                // Get pixel 
                uint32_t pixel = ENEMY_SPRITES[enemy_type][src_y * ENEMY_WIDTH + src_x];
                unsigned char r = (pixel >> 24) & 0xFF;
                unsigned char g = (pixel >> 16) & 0xFF;
                unsigned char b = (pixel >> 8) & 0xFF;
                unsigned char a = pixel & 0xFF;
                
                // Only draw if pixel is not fully transparent
                if (a > 0) {
                    drawPixelRGBA32(dst_x, dst_y, r, g, b, a);
                }
            }
        }
    }
    
    // Draw HP bar above enemy
    if (enemy.hp > 1) {
        int bar_width = draw_width; // use clipped width
        int bar_height = 10;
        int bar_x = dst_start_x;    // use clipped x position
        int bar_y = enemy_screen_y - 20;
        
        if (bar_y >= 0 && bar_y < SCREEN_HEIGHT - bar_height) {
            for (int x = 0; x < bar_width; x++) {
                for (int y = 0; y < bar_height; y++) {
                    if (x < (bar_width * enemy.hp) / 10)
                        drawPixelRGBA32(bar_x + x, bar_y + y, 0, 255, 0, 255);
                    else
                        drawPixelRGBA32(bar_x + x, bar_y + y, 255, 0, 0, 255);
                }
            }
        }
    } 
}
void drawBullets(int camera_x) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            int screen_x = bullets[i].x - camera_x;
            int screen_y = bullets[i].y;
            if (screen_x >= 0 && screen_x < SCREEN_WIDTH - BULLET_SIZE &&
                screen_y >= 0 && screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
                drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, screen_x, screen_y);
            }
        }
    }
}


void moveCharacter(int old_x, int old_y, int new_x, int new_y, 
                  int char_width, int char_height, 
                  const uint32_t* char_data, int camera_x) {
    // Note: this clears using camera_x; when clearing old position after camera moved,
    // caller should pass old_camera_x to properly restore background.
    clearRectWithBackground(old_x, old_y, char_width, char_height, camera_x);
    drawImageRGBA32(char_data, char_width, char_height, new_x, new_y);
}

void moveBullet(int old_x, int old_y, int new_x, int new_y, int camera_x) {
    clearRectWithBackground(old_x, old_y, BULLET_SIZE, BULLET_SIZE, camera_x);
    int screen_x = new_x - camera_x;
    int screen_y = new_y;
    if (screen_x >= 0 && screen_x < SCREEN_WIDTH - BULLET_SIZE &&
        screen_y >= 0 && screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
        drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, screen_x, screen_y);
    }
}

int isCollidingWithEnemy(int next_x, int player_y, Enemy *enemy) {
    if (!enemy->active) return 0; // no collision if enemy is dead

    // Player and enemy are rectangles; check overlap
    int player_left = next_x;
    int player_right = next_x + PLAYER_WIDTH;
    int player_top = player_y;
    int player_bottom = player_y + PLAYER_HEIGHT;

    int enemy_left = enemy->x;
    int enemy_right = enemy->x + ENEMY_WIDTH;
    int enemy_top = enemy->y;
    int enemy_bottom = enemy->y + ENEMY_HEIGHT;

    return !(player_right < enemy_left || player_left > enemy_right ||
             player_bottom < enemy_top || player_top > enemy_bottom);
}

void drawCountdownOnScreen(int camera_x) {
    unsigned int remaining = get_remaining_time_ms();
    unsigned int seconds = remaining / 1000;
    unsigned int tenths = (remaining % 1000) / 100;

    // Build timer string
    char buf[50];
    char *p = buf;
    p = strcopy(p, "Time left: ");
    
    char secStr[10];
    intToStr(seconds, secStr);
    p = strcopy(p, secStr);

    *p++ = '.';

    char tenthsStr[10];
    intToStr(tenths, tenthsStr);
    p = strcopy(p, tenthsStr);

    p = strcopy(p, " sec");
    *p = '\0';

    // Draw background to clear old text
    clearRectWithBackground(10, 10, 250, 20, camera_x);

    // Draw the new timer text on top
    drawString(10, 10, buf, 0xFFFFFFFF, 2);  // white text, scale 2
}



int task3_sidescroller(int timer_value) {
    int camera_x = 0;
    int player_x = 50;
    int player_y = 250;
    int old_player_x = player_x;
    int old_player_y = player_y;
    int jumping = 0;
    int jump_velocity = 0;
    int next_enemy_spawn_x = 300;  // first spawn after 350px
    int enemy_type = 0; // default to first enemy sprite
    int hp_increment = 5; // HP increase per enemy
    int timer_active = 0;
    int restart = 0;
    int lost = 0;
    

    typedef struct {
        int old_screen_x, old_screen_y;
        int was_active;
    } BulletPosition;
    BulletPosition old_bullet_pos[MAX_BULLETS];

    initBullets();
    for (int i = 0; i < MAX_BULLETS; i++) {
        old_bullet_pos[i].old_screen_x = -1;
        old_bullet_pos[i].old_screen_y = -1;
        old_bullet_pos[i].was_active = 0;
    }

    // Initialize enemy so it appears on-screen immediately
    enemy.active = 1;
    enemy.hp = 10;
    enemy.timer = 6000;
    enemy.move_timer = 2000;
    // place enemy near the right side of the visible screen (world coords)
    enemy.x = camera_x + SCREEN_WIDTH - ENEMY_WIDTH - 10; // visible on-screen at start
    enemy.y = player_y - 30; // same baseline as chicken
    int enemyClear = 0;

    uart_puts("\r\n--- Game Start ---\r\n");
    uart_puts("Controls: d = move right, a = move left, w = jump up, s / space = shoot q = quit\r\n");

    // initial draw
    draw_map(camera_x);
    drawBullets(camera_x);
    // draw player and enemy
    drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
    set_shooting_timer(timer_value); // 4 seconds to shoot
    
    while (1) {
        char c = uart_read();
        if (c == 'q') break;

        // drawCountdownOnScreen(camera_x);
        if (enemy.active) {
            timer_active = 1;
            int enemy_screen_x = enemy.x - camera_x;
            if (enemy_screen_x >= -ENEMY_WIDTH && enemy_screen_x < SCREEN_WIDTH){
                drawEnemy(camera_x, enemy_type);
            }
            drawCountdownOnScreen(camera_x);
        } 
        if (timer_active) {
            if (check_shooting_timer_expired() && enemy.active) {
                timer_active = 0;   // Stop the timer
                uart_puts("Time up! Enemy survived!\r\n");
                uart_puts("You lose!\r\n");
                uart_puts("Press 't' to continue and 'q' to quit\r\n");
                clear_screen();
                drawImageRGBA32(lose,500,500,0,0);
                drawString(100, 50, "Time up! You Lose!", 0xFFFFFFFF, 2);
                drawString(100, 450, "Press 't' to try again", 0xFFFFFFFF, 2);
                lost = 1;
                break;
            } else{
                drawCountdownOnScreen(camera_x);
            }
            
        }
        
        int camera_changed = 0;
        int player_moved = 0;
        int old_camera_x = camera_x;
        old_player_x = player_x;
        old_player_y = player_y;
        int world_x = player_x + camera_x;  // player position in world coords

       

        if (c == 'd') {
            if(isCollidingWithEnemy(world_x + 100, player_y, &enemy)) {
                uart_puts("Ouch! Collided with enemy!\r\n");
                continue;
            } 
            
            if (player_x + PLAYER_WIDTH/2 + 200 > SCREEN_WIDTH - 170) {
                if (camera_x < MAP_WIDTH - SCREEN_WIDTH - 50) {
                    camera_x += 100;
                    player_x -= 50;
                    camera_changed = 1;
                }
            } else {
                player_x += 100;
                player_moved = 1;
            }
        }
        else if (c == 'a') {
            if (player_x > 50) {
                player_x -= 100;
                player_moved = 1;
            } else if (camera_x > 0) {
                camera_x -= 100;
                camera_changed = 1;
            }
        }
        else if (c == 'w' && !jumping) {
            jumping = 1;
            jump_velocity = -20;
        }
        else if (c == 's' || c == ' ') {
            shootBullet(player_x, player_y, camera_x, 1);
        } 
        
        // Check if player moved far enough to spawn a new enemy
        if (!enemy.active && world_x >= next_enemy_spawn_x) {
            
            uart_puts("\n You have 4 seconds to shoot\r");
            uart_puts("\n New enemy appeared!\r");
            uart_puts("\n HP Level:");
            enemy.active = 1;
            enemy.hp = 10 + (enemy_type * hp_increment); // reset to 10 then increase by 5 each time
            uart_dec(enemy.hp);
            enemy.x = world_x + SCREEN_WIDTH - ENEMY_WIDTH; // spawn ahead of player
            enemy.y = player_y - 30;
            enemy_type++;
            next_enemy_spawn_x = world_x + 300;  // next spawn after another 300px
            enemyClear = 0;  // reset clear counter for the new enemy
            timer_active = 1;
            set_shooting_timer(timer_value); // 4 seconds to shoot
            uart_puts("\r\n");
        }
        

        

        handleJumping(&player_y, &jumping, &jump_velocity);
        if (player_y != old_player_y) player_moved = 1;

        // save old bullet screen positions using the old camera (so clearing is correct)
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                old_bullet_pos[i].old_screen_x = bullets[i].x - old_camera_x;
                old_bullet_pos[i].old_screen_y = bullets[i].y;
                old_bullet_pos[i].was_active = 1;
            } else {
                old_bullet_pos[i].was_active = 0;
            }
        }

        updateBullets(camera_x);

        if (camera_changed) {
            // full redraw when camera changes
            draw_map(camera_x);
            drawBullets(camera_x);
            drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
            if (enemy.active) {
                int enemy_screen_x = enemy.x - camera_x;
                if (enemy_screen_x >= -ENEMY_WIDTH && enemy_screen_x < SCREEN_WIDTH){
                    drawEnemy(camera_x, enemy_type);
                    reset_shooting_timer(timer_value); // reset timer on camera move and enemy alive
                }
            }
        } else {
            // selective redraw: clear old bullets using OLD camera, then draw new bullets
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (old_bullet_pos[i].was_active) {
                    int ox = old_bullet_pos[i].old_screen_x;
                    int oy = old_bullet_pos[i].old_screen_y;
                    if (ox >= 0 && ox < SCREEN_WIDTH - BULLET_SIZE && oy >= 0 && oy < SCREEN_HEIGHT - BULLET_SIZE) {
                        // use old_camera_x so background restored from correct world slice
                        clearRectWithBackground(ox, oy, BULLET_SIZE, BULLET_SIZE, old_camera_x);
                    }
                }
                if (bullets[i].active) {
                    int new_screen_x = bullets[i].x - camera_x;
                    int new_screen_y = bullets[i].y;
                    if (new_screen_x >= 0 && new_screen_x < SCREEN_WIDTH - BULLET_SIZE &&
                        new_screen_y >= 0 && new_screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
                        drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, new_screen_x, new_screen_y);
                    }
                }
            }

            // clear old player using OLD camera and draw new player
            if (player_moved) {
                clearRectWithBackground(old_player_x, old_player_y, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, old_camera_x);
                drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
            }

            // draw enemy at current camera
            if (enemy.active) {
                int enemy_screen_x = enemy.x - camera_x;
                if (enemy_screen_x >= -ENEMY_WIDTH && enemy_screen_x < SCREEN_WIDTH)
                    drawEnemy(camera_x, enemy_type);
            } else {
                enemyClear++;
            }
            if (enemyClear == 1) { // make sure the clearing happens only once
                wait_msec(1000);
                clearRectWithBackground(enemy.x - old_camera_x, enemy.y - 20, ENEMY_WIDTH, ENEMY_HEIGHT + 20, old_camera_x);
                drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
                drawString(player_x, player_y, "Enemy defeated!", 0xFFFFFFFF, 2);
            }
        }

        if(enemy_type > 4){
            // Player wins
            clear_screen();
            drawImageRGBA32(win,500,500,0,0);
            drawString(100, 50, "Congratz!! You Win!", 0xFFFFFFFF, 2);
            drawString(100, 450, "Press 'h' to make go to level 2", 0x00, 1);
            uart_puts("You Win!\r\n");
            uart_puts("Press 'h' to go to level 2 or 'q' to quit\r\n");
            break;
        }

        wait_msec(1000); // keep your original tick; lower to ~16 for smoother
    }
    
    while (1) {
        char c = uart_read();
        if (c == 't' && lost) {
            restart = 1;
            break;
        } else if (c == 'q') {
            restart = 0;
            break;
        } else if (c == 'h' && !lost) {
            restart = 2; // go to level 2
            break;
        }
    }
    
    return restart;
    
}

int game() {
    int restart;
    int increment = 1000;
    int base_time = 5000; // start with 5 seconds
    int max_time = 1000;

    while (1) {
        restart = task3_sidescroller(base_time);

        if (restart == 1) {
            // 't' pressed -> restart same level
            continue;
        } 
        else if (restart == 2) {
            // 'h' pressed -> next level
            if (base_time - increment >= max_time) {
                base_time -= increment;  // decrease time for next level
            } else {
                uart_puts("Maximum difficulty reached!\r\n");
            }
            
            continue;
        } 
        else {
            break;  // 'q' pressed -> quit
        }
    }

    return 0;
}

