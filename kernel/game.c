#include "game.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "player.h"
#include "../assets/level1_map.h"
#include "../assets/shoot_chicken.h"
#include "../assets/bullet.h"


#define SCREEN_WIDTH  500
#define SCREEN_HEIGHT 500

#define MAX_BULLETS 10
#define BULLET_SPEED 7
#define BULLET_SIZE 50

// Bullet structure
typedef struct {
    int x;
    int y;
    int active;
    int direction; // 1 for right, -1 for left
} Bullet;

// Global bullet array
Bullet bullets[MAX_BULLETS];

// ---------------- Side-scroller Demo ----------------
void drawImagePart(const unsigned int *src,
                   int sx, int sy, int w, int h,
                   int dx, int dy) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            unsigned int color = src[(sy + y) * MAP_WIDTH + (sx + x)];
            drawPixelARGB32(dx + x, dy + y, color);
        }
    }
}

void draw_map(int camera_x) {
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            int map_x = camera_x + x;
            int map_y = y;
            if (map_x >= MAP_WIDTH) map_x = MAP_WIDTH - 1;

            uint32_t pixel = level1_map[map_y * MAP_WIDTH + map_x];

            // Extract RGBA directly because both map & framebuffer use RGBA
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
        // Apply velocity to position
        *player_y += *jump_velocity;
        
        // Apply gravity to velocity
        *jump_velocity += GRAVITY;
        
        // Check if landed
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
    // Find an inactive bullet slot
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].active = 1;
            bullets[i].x = player_x + camera_x + (facing_direction > 0 ? SHOOT_CHICKEN_WIDTH : 0);
            bullets[i].y = player_y + SHOOT_CHICKEN_HEIGHT / 3;
            bullets[i].direction = facing_direction;
            
            break;
        }
    }
}

void updateBullets(int camera_x) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            // Move bullet
            bullets[i].x += BULLET_SPEED * bullets[i].direction;
            
            // Check if bullet is out of bounds (in world coordinates)
            if (bullets[i].x < 0 || bullets[i].x > MAP_WIDTH) {
                bullets[i].active = 0;
            }
            
            // Also deactivate if too far from visible area to save processing
            int screen_x = bullets[i].x - camera_x;
            if (screen_x < -100 || screen_x > SCREEN_WIDTH + 100) {
                bullets[i].active = 0;
            }
        }
    }
}

void drawBullets(int camera_x) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            int screen_x = bullets[i].x - camera_x;
            int screen_y = bullets[i].y;
            
            // Only draw if bullet is on screen
            if (screen_x >= 0 && screen_x < SCREEN_WIDTH - BULLET_SIZE &&
                screen_y >= 0 && screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
                
                // Draw bullet using the bullet image
                drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, screen_x, screen_y);
            }
        }
    }
}


void task3_sidescroller() {
    int camera_x = 0;
    int player_x = 100; // screen X position
    int player_y = 250; // screen Y position
    int jumping = 0;    // is player jumping?
    int jump_velocity = 0;
    
    initBullets();
    uart_puts("\r\n--- Game Start ---\r\n");
    uart_puts("\r\nPRESS ANY KEY TO START!\r\n");
    uart_puts("Controls: d = move right, a = move left, w = jump up, s / space = shoot q = quit\r\n");
    // Draw initial frame
    draw_map(camera_x);
    drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);

    

    while (1) {
        // Non-blocking read if your uart_getc supports it
        char c = uart_read();// or implement non-blocking
        int needed_redraw = 0;

        if (c == 'q') break;
        
        if (c == 'd') {
            // Check if player WOULD hit the border if they moved
            if (player_x + PLAYER_WIDTH/2 + 200 > SCREEN_WIDTH- 170) {
                // Moving would put player past border - scroll camera instead
                if (camera_x < MAP_WIDTH - SCREEN_WIDTH - 50) {
                    camera_x += 100;
                    player_x -= 50;
                }
            } else {
                // Safe to move player
                player_x += 100;
            }
            needed_redraw = 1;
        }
        else if (c == 'a') {
            if (player_x > 50)
                player_x -= 100;            // move player left on screen
            else if (camera_x > 0)
                camera_x -= 100;            // scroll map left when at edge
            needed_redraw = 1;

        } else if (c == 'w' && !jumping) {
            jumping = 1;                    // start jump
            jump_velocity = -20;  // Initial upward velocity
            needed_redraw = 1;
            // Handle jumping
        
        } else if( c == 's' || c == ' ') {
            shootBullet(player_x, player_y, camera_x, 1); // Shoot right
            needed_redraw = 1;
        }

        handleJumping( &player_y, &jumping, &jump_velocity );
        updateBullets(camera_x);

        // Check if there are active bullets
        int has_active_bullets = 0;
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                has_active_bullets = 1;
                break;
            }
        }

        if(needed_redraw || jumping|| has_active_bullets) {
            // Redraw only if something changed
            draw_map(camera_x);
            drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
            drawBullets(camera_x);
        }
        
        wait_msec(1000);
    }
}

