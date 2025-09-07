#include "menu.h"
#include "font.h"   
#include "framebf.h"

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
    m->items = items; m->count = count; m->selected = 0;
    m->x=x; m->y=y; m->w=w; m->h=h;

    m->zoom   = 1;                             // Menu letter Size
    m->lineH  = (FONT_HEIGHT * m->zoom) + 10;  // The interval of line

    // Colour
    m->boxBg    = 0xFFF5F5F5;     // Box background
    m->boxBorder= 0xFFBDBDBD;     // Box Outline
    m->itemFg   = 0xFF222222;     // The list of menu items
    m->selBg    = 0xFFFFD54F;     // Select Highlight
    m->selFg    = 0xFF000000;     // Select Text
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
    int hx = m->x + m->w - 6 - str_len(hint)*(FONT_WIDTH);
    int hy = m->y + m->h - (FONT_HEIGHT+6);
    if(hx < m->x + 6) hx = m->x + 6;
    drawString(hx, hy, (char*)hint, 0xFF777777, 1);
}
