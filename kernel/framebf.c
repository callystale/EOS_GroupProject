// ----------------------------------- framebf.c -------------------------------------
#include "mbox.h"
#include "../uart/uart0.h"
#include "../uart/uart1.h"
#include "font.h"
#include "framebf.h"

//Use RGBA32 (32 bits for each pixel)
#define COLOR_DEPTH 32

//Pixel Order: BGR in memory order (little endian --> RGB in byte order)
#define PIXEL_ORDER 0

//Screen info
unsigned int width, height, pitch;

/* Frame buffer address
 * (declare as pointer of unsigned char to access each byte) */
unsigned char *fb;


/**
 * Set screen resolution to 1024x768
 */
void framebf_init()
{
    mBuf[0] = 35*4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;

    mBuf[2] = MBOX_TAG_SETPHYWH; //Set physical width-height
    mBuf[3] = 8; // Value size in bytes
    mBuf[4] = 0; // REQUEST CODE = 0
    mBuf[5] = 500; // Value(width)
    mBuf[6] = 500; // Value(height)

    mBuf[7] = MBOX_TAG_SETVIRTWH; //Set virtual width-height
    mBuf[8] = 8;
    mBuf[9] = 0;
    mBuf[10] = 500;
    mBuf[11] = 500;

    mBuf[12] = MBOX_TAG_SETVIRTOFF; //Set virtual offset
    mBuf[13] = 8;
    mBuf[14] = 0;
    mBuf[15] = 0; // x offset
    mBuf[16] = 0; // y offset

    mBuf[17] = MBOX_TAG_SETDEPTH; //Set color depth
    mBuf[18] = 4;
    mBuf[19] = 0;
    mBuf[20] = COLOR_DEPTH; //Bits per pixel

    mBuf[21] = MBOX_TAG_SETPXLORDR; //Set pixel order
    mBuf[22] = 4;
    mBuf[23] = 0;
    mBuf[24] = PIXEL_ORDER;

    mBuf[25] = MBOX_TAG_GETFB; //Get frame buffer
    mBuf[26] = 8;
    mBuf[27] = 0;
    mBuf[28] = 16; //alignment in 16 bytes
    mBuf[29] = 0;  //will return Frame Buffer size in bytes

    mBuf[30] = MBOX_TAG_GETPITCH; //Get pitch
    mBuf[31] = 4;
    mBuf[32] = 0;
    mBuf[33] = 0; //Will get pitch value here

    mBuf[34] = MBOX_TAG_LAST;

    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP) //mailbox call is successful ?
    	&& mBuf[20] == COLOR_DEPTH //got correct color depth ?
		&& mBuf[24] == PIXEL_ORDER //got correct pixel order ?
		&& mBuf[28] != 0 //got a valid address for frame buffer ?
		) {

    	/* Convert GPU address to ARM address (clear higher address bits)
    	 * Frame Buffer is located in RAM memory, which VideoCore MMU
    	 * maps it to bus address space starting at 0xC0000000.
    	 * Software accessing RAM directly use physical addresses
    	 * (based at 0x00000000)
    	*/
    	mBuf[28] &= 0x3FFFFFFF;

        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mBuf[28]);
        uart_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart_hex(mBuf[28]);
        uart_puts("\n");

        uart_puts("Frame Buffer Size (bytes): ");
        uart_dec(mBuf[29]);
        uart_puts("\n");

        width = mBuf[5];     	// Actual physical width
        height = mBuf[6];     	// Actual physical height
        pitch = mBuf[33];       // Number of bytes per line

    } else {
    	uart_puts("Unable to get a frame buffer with provided setting\n");
    }
}



void drawPixelARGB32(int x, int y, unsigned int attr)
{
	int offs = (y * pitch) + (COLOR_DEPTH/8 * x);

/*	//Access and assign each byte
    *(fb + offs    ) = (attr >> 0 ) & 0xFF; //BLUE  (get the least significant byte)
    *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
    *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
    *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
*/

	//Access 32-bit together
	*((unsigned int*)(fb + offs)) = attr;
}

void drawPixelRGBA32(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) 
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);

    // Write in BGRA order for Pi framebuffer
    *(fb + offs    ) = b; // Blue
    *(fb + offs + 1) = g; // Green
    *(fb + offs + 2) = r; // Red
    *(fb + offs + 3) = a; // Alpha
}



void drawRectARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill)
{
	for (int y = y1; y <= y2; y++ )
		for (int x = x1; x <= x2; x++) {
			if ((x == x1 || x == x2) || (y == y1 || y == y2))
				drawPixelARGB32(x, y, attr);
			else if (fill)
				drawPixelARGB32(x, y, attr);
		}
}




// Function to draw line
void drawLine(int x1, int y1, int x2, int y2, unsigned int attr)
{
	for (int x = x1; x <= x2; x++ ){
        int y = (float)(y1 - y2)/(x1 - x2)*(x - x1) + y1;
        drawPixelARGB32(x, y, attr);
    }
}


// Function to calculate the square root of a number using the Newton-Raphson method
double sqrt(double number) {
    if (number < 0) {
        return -1; // Return -1 for negative inputs as square root of negative is not defined in real numbers
    }
    
    double tolerance = 0.000001; // Define the tolerance for the result
    double guess = number / 2.0; // Initial guess (can be any positive number, here half of the number)
    double result = 0.0;
    
    while (1) {
        result = 0.5 * (guess + number / guess); // Calculate the next approximation
        
        // Check if the difference between the current guess and the new result is within the tolerance
        int diff = (result > guess) ? (result - guess) : (guess - result);
        if (diff < tolerance) {
            break;
        }
        
        guess = result; // Update the guess for the next iteration
    }
    
    return result;
}


// Function to draw circle
void drawLCircle(int center_x, int center_y, int radius, unsigned int attr, int fill)
{
    //Draw the circle when going on x side
    for (int x = center_x - radius; x <= center_x + radius; x++) {
        // Calculate the corresponding y values using the circle equation
        int dy = sqrt(radius * radius - (x - center_x) * (x - center_x)); 
        int upper_y = center_y + dy;
        int lower_y = center_y - dy;

        drawPixelARGB32(x, upper_y, attr);
        drawPixelARGB32(x, lower_y, attr);

        // Fill the circle, draw a line between lower_y and upper_y
        if (fill) {
            for (int y = lower_y; y <= upper_y; y++) {
                drawPixelARGB32(x, y, attr);
            }
        }
    }

    /* Also draw the circle border when going on y side (
    since some points may be missing due to inaccurate calculation above) */

    for (int y = center_y - radius; y <= center_y + radius; y++) {
        // Calculate the corresponding x values using the circle equation
        int dx = sqrt(radius * radius - (y - center_y) * (y - center_y)); 

        int left_x = center_x - dx;
        int right_x = center_x + dx;

        drawPixelARGB32(left_x, y, attr);
        drawPixelARGB32(right_x, y, attr);
    }
}



/* Functions to display text on the screen */
// NOTE: zoom = 0 will not display the character
void drawChar(unsigned char ch, int x, int y, unsigned int attr, int zoom)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i = 1; i <= (FONT_HEIGHT*zoom); i++) {
		for (int j = 0; j< (FONT_WIDTH*zoom); j++) {
			unsigned char mask = 1 << (j/zoom);
            if (*glyph & mask) { //only draw pixels belong to the character glyph
			    drawPixelARGB32(x + j, y + i, attr);
            }
		}
		glyph += (i % zoom) ? 0 : FONT_BPL;
    }
}


void drawString(int x, int y, char *str, unsigned int attr, int zoom)
{
    while (*str) {
        if (*str == '\r') {
            x = 0;
        } else if (*str == '\n') {
            x = 0; 
			y += (FONT_HEIGHT*zoom);
        } else {
            drawChar(*str, x, y, attr, zoom);
            x += (FONT_WIDTH*zoom);
        }
        str++;
    }
}
/* Example: Show green HELLO WORLD text on the screen */
// drawString(0, 0, "HELLO WORLD !!!", 0x0000BB00, 1);

/* Functions to display image on the screen */
void drawImage(const unsigned int pixel_data[], int pos_x, int pos_y, int width, int height){
    for (int i = 0; i < width*height; i++){
        int x = pos_x + (i % width);
        int y = pos_y + (i / width);
        drawPixelARGB32(x, y, pixel_data[i]);
    }
}


void drawImageRGBA32(const unsigned int *img, int img_w, int img_h, int pos_x, int pos_y) {
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            unsigned int pixel = img[y * img_w + x];

            // Extract RGBA
            unsigned char r = (pixel >> 24) & 0xFF;
            unsigned char g = (pixel >> 16) & 0xFF;
            unsigned char b = (pixel >> 8)  & 0xFF;
            unsigned char a = pixel & 0xFF;

            // Skip fully transparent pixels
            if (a == 0) continue;

            drawPixelRGBA32(pos_x + x, pos_y + y, r, g, b, a);
        }
    }
}

// Display an image for video player
void displayMultipleImages(const unsigned int image[], int startX, int startY, int w, int h)
{
    int color = 0;
    for (int y = startY; y < h + startY; y++)
    {
        for (int x = startX; x < w + startX; x++)
        {
            drawPixelARGB32(x, y, image[color]); // Draw a pixel at the current (x, y) position using the color from the image array
            color++;                             // Move to the next color in the image data
        }
    }
}

/* Functions to delay, set/wait timer */
void wait_msec(unsigned int n)
{
    register unsigned long f, t, r, expiredTime;

    // Get the current counter frequency (Hz)
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));

    // Read the current counter value
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));

    // Calculate expire value for counter
    expiredTime = t + ((f / 1000) * n) / 1000;
    do
    {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r < expiredTime);
}

void clear_screen() {
    for (int y = 0; y < 500; y++) {
        for (int x = 0; x < 500; x++) {
            drawPixelARGB32(x, y, 0x0);
        }
    }
    uart_puts("Screen cleared.\r\n");
}