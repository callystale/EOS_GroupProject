// ----------------------------------- framebf.h -------------------------------------
void framebf_init();
void drawPixelARGB32(int x, int y, unsigned int attr);
void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void drawPixelRGBA32(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
void drawImageRGBA32(const unsigned int *img, int img_w, int img_h, int pos_x, int pos_y);
void drawString(int x, int y, char *str, unsigned int attr, int zoom);
void drawChar(unsigned char ch, int x, int y, unsigned int attr, int zoom);
void drawImage(const unsigned int pixel_data[], int pos_x, int pos_y, int width, int height);
void drawLine(int x1, int y1, int x2, int y2, unsigned int attr);
double sqrt(double number);
void displayMultipleImages(const unsigned int image[], int startX, int startY, int w, int h);
void wait_msec(unsigned int n);