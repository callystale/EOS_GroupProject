#include "game.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "player.h"
#include "../assets/level1_map.h"
#include "../assets/shoot_chicken.h"
#include "../assets/bullet.h"


#define SCREEN_WIDTH  500
#define SCREEN_HEIGHT 500

#define MAX_BULLETS 20
#define BULLET_SPEED 1
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

// Add these functions to your game.c file

// Function to clear a rectangular area and fill with background data
void clearRectWithBackground(int x, int y, int width, int height, int camera_x) {
    // Bounds checking
    if (x < 0) { width += x; x = 0; }
    if (y < 0) { height += y; y = 0; }
    if (x + width > SCREEN_WIDTH) width = SCREEN_WIDTH - x;
    if (y + height > SCREEN_HEIGHT) height = SCREEN_HEIGHT - y;
    
    if (width <= 0 || height <= 0) return;
    
    // Redraw background pixels in the specified rectangle
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int screen_x = x + dx;
            int screen_y = y + dy;
            int map_x = camera_x + screen_x;
            int map_y = screen_y;
            
            // Bounds check for map
            if (map_x >= MAP_WIDTH) map_x = MAP_WIDTH - 1;
            if (map_y >= SCREEN_HEIGHT) continue;
            
            uint32_t pixel = level1_map[map_y * MAP_WIDTH + map_x];
            
            // Extract RGBA
            unsigned char r = (pixel >> 24) & 0xFF;
            unsigned char g = (pixel >> 16) & 0xFF;
            unsigned char b = (pixel >> 8) & 0xFF;
            unsigned char a = pixel & 0xFF;
            
            drawPixelRGBA32(screen_x, screen_y, r, g, b, a);
        }
    }
}

// Function to move character by clearing old position and drawing at new position
void moveCharacter(int old_x, int old_y, int new_x, int new_y, 
                  int char_width, int char_height, 
                  const uint32_t* char_data, int camera_x) {
    
    // Clear old position with background
    clearRectWithBackground(old_x, old_y, char_width, char_height, camera_x);
    
    // Draw character at new position
    drawImageRGBA32(char_data, char_width, char_height, new_x, new_y);
}

// Function to move bullet (similar concept)
void moveBullet(int old_x, int old_y, int new_x, int new_y, int camera_x) {
    // Clear old position
    clearRectWithBackground(old_x, old_y, BULLET_SIZE, BULLET_SIZE, camera_x);
    
    // Draw bullet at new position
    int screen_x = new_x - camera_x;
    int screen_y = new_y;
    
    if (screen_x >= 0 && screen_x < SCREEN_WIDTH - BULLET_SIZE &&
        screen_y >= 0 && screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
        drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, screen_x, screen_y);
    }
}

void task3_sidescroller() {
    int camera_x = 0;
    int player_x = 100;
    int player_y = 250;
    int old_player_x = player_x;
    int old_player_y = player_y;
    int jumping = 0;
    int jump_velocity = 0;
    
    // Store old bullet positions for clearing - FIXED STRUCTURE
    typedef struct {
        int old_screen_x, old_screen_y;  // Store screen coordinates directly
        int was_active;                   // Track if bullet was active last frame
    } BulletPosition;
    BulletPosition old_bullet_pos[MAX_BULLETS];
    
    initBullets();
    
    // Initialize old bullet positions
    for (int i = 0; i < MAX_BULLETS; i++) {
        old_bullet_pos[i].old_screen_x = -1;
        old_bullet_pos[i].old_screen_y = -1;
        old_bullet_pos[i].was_active = 0;
    }
    
    uart_puts("\r\n--- Game Start ---\r\n");
    uart_puts("Controls: d = move right, a = move left, w = jump up, s / space = shoot q = quit\r\n");
    
    // Draw initial frame
    draw_map(camera_x);
    drawBullets(camera_x);
    drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);

    while (1) {
        char c = uart_read();
        int camera_changed = 0;
        int player_moved = 0;

        if (c == 'q') break;
        
        // Store old positions
        old_player_x = player_x;
        old_player_y = player_y;
        int old_camera_x = camera_x;
        
        if (c == 'd') {
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

        handleJumping(&player_y, &jumping, &jump_velocity);
        
        if (player_y != old_player_y) {
            player_moved = 1;
        }

        // FIXED: Store old bullet SCREEN positions before updating
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
            // Camera moved - full redraw
            draw_map(camera_x);
            drawBullets(camera_x);
            drawImageRGBA32(shoot_chicken, SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT, player_x, player_y);
        } else {
            // Selective redrawing
            
            // FIXED: Handle bullets with proper clearing
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (old_bullet_pos[i].was_active) {
                    // Clear the old bullet position first
                    if (old_bullet_pos[i].old_screen_x >= 0 && 
                        old_bullet_pos[i].old_screen_x < SCREEN_WIDTH - BULLET_SIZE &&
                        old_bullet_pos[i].old_screen_y >= 0 && 
                        old_bullet_pos[i].old_screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
                        
                        clearRectWithBackground(old_bullet_pos[i].old_screen_x, 
                                              old_bullet_pos[i].old_screen_y,
                                              BULLET_SIZE, BULLET_SIZE, camera_x);
                    }
                }
                
                // Draw bullet at new position if still active
                if (bullets[i].active) {
                    int new_screen_x = bullets[i].x - camera_x;
                    int new_screen_y = bullets[i].y;
                    
                    if (new_screen_x >= 0 && new_screen_x < SCREEN_WIDTH - BULLET_SIZE &&
                        new_screen_y >= 0 && new_screen_y < SCREEN_HEIGHT - BULLET_SIZE) {
                        drawImageRGBA32(bullet, BULLET_SIZE, BULLET_SIZE, new_screen_x, new_screen_y);
                    }
                }
            }
            
            // Handle player movement (after bullets so player appears on top)
            if (player_moved) {
                moveCharacter(old_player_x, old_player_y, player_x, player_y,
                            SHOOT_CHICKEN_WIDTH, SHOOT_CHICKEN_HEIGHT,
                            shoot_chicken, camera_x);
            }
        }
        
        wait_msec(1000);
    }
}