// ----------------------------------- framebf.h -------------------------------------
void framebf_init();
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void drawString(int x, int y, char *str, unsigned int attr, int zoom);
void drawImage(const unsigned int pixel_data[], int pos_x, int pos_y, int width, int height);