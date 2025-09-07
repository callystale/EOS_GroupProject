#include "../uart/uart0.h"
#include "../uart/uart1.h"
#include "mbox.h"
#include "framebf.h"
#include "background.h"
#include "menu.h"

#define FONT_HEIGHT 8

extern unsigned int width, height;   // Global variable


void display_backgroundandtitle(void) {
    drawImage(background_bitmap_Chicken_Gunny, 0, 0, BG_WIDTH, BG_HEIGHT);
    render_title_top("CHICKEN GUNNY");
}

/* Function for displaying menu box on the screen */
void show_main_menu(void) {
    // The list of menu
    const char* MAIN_ITEMS[] = { "Start Game", "Options", "Credits", "Exit" };

    // Menu box location/size
    int boxX = 20;
    int boxW = (int)width - 40;
    int boxH = 4 * (FONT_HEIGHT + 10) + 24;   // The height of menu items
    int boxY = (int)height - boxH - 20;

    // Menu initialization
    Menu m;
    menu_init_region(&m, MAIN_ITEMS, 4, boxX, boxY, boxW, boxH);

    // Blocking input loop(whenever inputting, redraw)
    int in_menu = 1;
    while (in_menu) {
        // 1) Background + Title
        display_backgroundandtitle();
        // 2) Menu Box render
        menu_render(&m);

        // 3) input(if not, blocking by uart_getc)
        char c = uart_getc();

        // 4) Processing
        if (c=='w' || c=='k' || c=='s' || c=='j') {
            menu_handle_key(&m, c);   // Move
        } else if (c=='\r' || c=='\n') {
            switch (m.selected) {
                case 0: // Start Game
                    uart_puts("\r\n[MENU] Start Game selected\r\n");
                    in_menu = 0;      // Menu exit → go to the the next step(Game)
                    break;
                case 1: // Options
                    uart_puts("\r\n[MENU] Options selected\r\n");
                    // TODO: Options screeen (it can be Character Selection)
                    break;
                case 2: // Credits
                    uart_puts("\r\n[MENU] Credits selected\r\n");
                    // TODO: Credits screen (if not needed, please get rid of it)
                    break;
                case 3: // Exit
                    uart_puts("\r\n[MENU] Exit selected\r\n");
                    qemu_exit_semihosting(0); // Exit
                    in_menu = 0;
                    break;
            }
        }
        // Ignore the others
    }
    
}

void main(void)
{
    
    uart_init();
    uart_puts("\n\nHello World!\n");

    
    framebf_init();
    // show the main menu
    show_main_menu();

    while (1) {
        char c = uart_getc();
        uart_sendc(c);
    }
}
