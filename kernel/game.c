#include "game.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "../assets/level1_map.h"
#include "player.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

// ---------------- Turn-based Game ----------------
typedef struct {
    char *name;
    int hp;
    int x;
} TBPlayer;

typedef struct {
    int hp;
    int x;
    int speed;
} TBMonster;

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
            uint32_t color = level1_map[map_y * MAP_WIDTH + map_x];
            drawPixelARGB32(x, y, color);
        }
    }
}

void clearScreen() {
    for (int y = 0; y < SCREEN_HEIGHT; y++)
        for (int x = 0; x < SCREEN_WIDTH; x++)
            drawPixelARGB32(x, y, 0xFF000000); // black background
}


void task3_sidescroller() {
    int camera_x = 0;
    int player_x = 100; // screen X position
    int player_y = 350; // screen Y position

    uart_puts("\r\n--- Side Scroller Demo ---\r\n");
    uart_puts("\r\nPress any key to start!\r\n");
    uart_puts("Controls: d = move right, a = move left, q = quit\r\n");

    while (1) {
        // Non-blocking read if your uart_getc supports it
        char c = uart_getc(); // or implement non-blocking

        if (c == 'q') break;

        if (c == 'd') {
        if (player_x < SCREEN_WIDTH - PLAYER_WIDTH - 10)
            player_x += 100;            // move player on screen
        else if (camera_x < MAP_WIDTH - SCREEN_WIDTH)
            camera_x += 100;            // scroll map when player reaches edge
        }
        else if (c == 'a') {
        if (player_x > 10)
            player_x -= 100;            // move player left on screen
        else if (camera_x > 0)
            camera_x -= 100;            // scroll map left when at edge
        }
        // Redraw everything every iteration
        clearScreen();
        draw_map(camera_x);
        drawImage(player_sprite, 100, 350, PLAYER_WIDTH, PLAYER_HEIGHT);
    }
}

