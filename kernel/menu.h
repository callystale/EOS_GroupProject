// Global Variable for Font and Screen data
extern unsigned int width, height;
void drawChar(unsigned char ch, int x, int y, unsigned int attr, int zoom);
void drawString(int x, int y, char *str, unsigned int attr, int zoom);
void drawPixelARGB32(int x, int y, unsigned int attr);
int  str_len(const char *s);
void drawRect(int x,int y,int w,int h,unsigned int argb,int t);

// Menu Data
typedef struct {
    const char **items;
    int count;
    int selected;
    // Box Layout
    int x, y, w, h;
    int zoom;         // Zoom the letter(1~2)
    int lineH;        // Line height(px)
    // Colour
    unsigned int boxBg, boxBorder;
    unsigned int itemFg, selBg, selFg;
} Menu;
void qemu_exit_semihosting(int code);
void menu_init_region(Menu *m, const char **items, int count, int x, int y, int w, int h);
void menu_handle_key(Menu *m, char key);         // w/s or j/k, Enter
void menu_render(const Menu *m);
