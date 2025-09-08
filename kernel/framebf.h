// ----------------------------------- framebf.h -------------------------------------
void framebf_init();
extern unsigned int width;
extern unsigned int height;
extern unsigned int pitch;
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawHealthBar(int x1, int y1, int x2, int y2, unsigned int borderColor, unsigned int fillColor, int fill);
