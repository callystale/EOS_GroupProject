#include "menu.h"
//include "font.h"   
#include "framebf.h"
#include "../uart/uart1.h"
#include "menu_background.h"


// Function for Qemu Exit
void qemu_exit_semihosting(int code){
    register long x0 asm("x0") = 0x18;   // SYS_EXIT
    register long x1 asm("x1") = code;  // exit code
    asm volatile("hlt #0xf000" : : "r"(x0), "r"(x1) : "memory");
    for(;;) asm volatile("wfi");
}


void drawRect(int x,int y,int w,int h,unsigned int argb,int t){
    if(t<1) t=1;
    fillRect(x,       y,        w, t, argb);
    fillRect(x,       y+h-t,    w, t, argb);
    fillRect(x,       y,        t, h, argb);
    fillRect(x+w-t,   y,        t, h, argb);
}



// ------------------ API ------------------
void menu_init_region(Menu *m, const char **items, int count, int x, int y, int w, int h){
    uart_puts("[DEBUG] menu_init_region: Starting...\r\n");
    
    // Set one field at a time and test
    m->items = items; 
    uart_puts("[DEBUG] menu_init_region: Set items\r\n");
    
    m->count = count; 
    uart_puts("[DEBUG] menu_init_region: Set count\r\n");
    
    m->selected = 0;
    uart_puts("[DEBUG] menu_init_region: Set selected\r\n");
    
    m->x = x; 
    uart_puts("[DEBUG] menu_init_region: Set x\r\n");
    
    m->y = y; 
    uart_puts("[DEBUG] menu_init_region: Set y\r\n");
    
    m->w = w; 
    uart_puts("[DEBUG] menu_init_region: Set w\r\n");
    
    m->h = h;
    uart_puts("[DEBUG] menu_init_region: Set h\r\n");

    m->zoom = 1;
    uart_puts("[DEBUG] menu_init_region: Set zoom\r\n");
    
    m->lineH = (8 * m->zoom) + 10;
    uart_puts("[DEBUG] menu_init_region: Set lineH\r\n");

    // Try setting colors one by one
    uart_puts("[DEBUG] menu_init_region: About to set boxBg\r\n");
    m->boxBg = 0xFFF5F5F5;
    uart_puts("[DEBUG] menu_init_region: Set boxBg\r\n");
    
    uart_puts("[DEBUG] menu_init_region: About to set boxBorder\r\n");
    m->boxBorder = 0xFFBDBDBD;
    uart_puts("[DEBUG] menu_init_region: Set boxBorder\r\n");
    
    uart_puts("[DEBUG] menu_init_region: About to set itemFg\r\n");
    m->itemFg = 0xFF222222;
    uart_puts("[DEBUG] menu_init_region: Set itemFg\r\n");
    
    uart_puts("[DEBUG] menu_init_region: About to set selBg\r\n");
    m->selBg = 0xFFFFD54F;
    uart_puts("[DEBUG] menu_init_region: Set selBg\r\n");
    
    uart_puts("[DEBUG] menu_init_region: About to set selFg\r\n");
    m->selFg = 0xFF000000;
    uart_puts("[DEBUG] menu_init_region: Set selFg\r\n");
    
    uart_puts("[DEBUG] menu_init_region: Completed successfully\r\n");
}

void menu_handle_key(Menu *m, char key){
    if(key=='w'||key=='k')        m->selected = (m->selected-1 + m->count) % m->count;
    else if(key=='s'||key=='j')   m->selected = (m->selected+1) % m->count;
    // Enter is processed in the outloop m->selected 
}

void menu_render(const Menu *m){
    // 1) Box background/Outline
    fillRect(m->x, m->y, m->w, m->h, m->boxBg);
    drawRect(m->x, m->y, m->w, m->h, m->boxBorder, 2);

    // 2) Render the list of menu items
    int padX = 14, padY = 12;
    int x = m->x + padX;
    int y = m->y + padY;

    for(int i=0;i<m->count;i++){
        if(y + m->lineH > m->y + m->h) break;   // Stop if exceeding box size
        if(i==m->selected){
            // select background line
            fillRect(m->x + 6, y-2, m->w - 12, m->lineH, m->selBg);
            drawString(x, y, (char*)m->items[i], m->selFg, m->zoom);
        }else{
            drawString(x, y, (char*)m->items[i], m->itemFg, m->zoom);
        }
        y += m->lineH;
    }

    // 3) Small guide letter
    const char* hint="[W/S] Move   [Enter] Select";
    int hx = m->x + m->w - 6 - str_len(hint)*(8);
    int hy = m->y + m->h - (8+6);
    if(hx < m->x + 6) hx = m->x + 6;
    drawString(hx, hy, (char*)hint, 0xFF777777, 1);
}


extern unsigned int width, height;   // Global variable


void display_backgroundandtitle(void) {
    drawImage(background_bitmap_Chicken_Gunny, 0, 0, BG_WIDTH, BG_HEIGHT);
    render_title_top("CHICKEN GUNNY");
}

/* Function for displaying menu box on the screen */
void show_main_menu(void) {
    uart_puts("[MENU] Entering Main Menu...\r\n");
    // The list of menu
    const char* MAIN_ITEMS[] = { "Start Game", "Options", "Credits", "Exit" };

    // Menu box location/size
    int boxX = 20;
    int boxW = (int)width - 40;
    int boxH = 4 * (8 + 10) + 24;   // The height of menu items
    int boxY = (int)height - boxH - 20;

    // Menu initialization
    uart_puts("Initializing menu...\r\n");
    Menu m;
    menu_init_region(&m, MAIN_ITEMS, 4, boxX, boxY, boxW, boxH);
    uart_puts("[DEBUG] Menu initialized successfully\r\n");
    
    // Blocking input loop(whenever inputting, redraw)
    int in_menu = 1;
    while (1) {
        // 1) Background + Title
        uart_puts("Displaying background and title...\r\n");
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
                    uart_puts("\r\n[MENU] Characters selected\r\n");
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
