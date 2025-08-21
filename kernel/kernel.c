#include "../uart/uart0.h"
#include "../uart/uart1.h"
#include "mbox.h"
#include "framebf.h"
#define WELCOME_MSG \
" ####### ####### ####### #######  #####  #        #####    ###      \n" \
" #       #       #          #    #     # #    #  #     #  #   #     \n" \
" #       #       #          #          # #    #  #     # #     #    \n" \
" #####   #####   #####      #     #####  #    #   ###### #     #    \n" \
" #       #       #          #    #       #######       # #     #    \n" \
" #       #       #          #    #            #  #     #  #   #     \n" \
" ####### ####### #######    #    #######      #   #####    ###      \n" \
"                                                                    \n" \
" ######     #    ######  #######    #######  #####                  \n" \
" #     #   # #   #     # #          #     # #     #                 \n" \
" #     #  #   #  #     # #          #     # #                       \n" \
" ######  #     # ######  #####      #     #  #####                  \n" \
" #     # ####### #   #   #          #     #       #                 \n" \
" #     # #     # #    #  #          #     # #     #                 \n" \
" ######  #     # #     # #######    #######  #####                  \n" \
"                                                                    \n" \
"- Developed by Group 2\n" \
"  Members:\n" \
"    - s3993986  Hồng Thiệu Kiệt\n" \
"    - s3978567  Lê Phương Ngân\n" \
"    - s3990627  Nguyễn Hoàng Sơn\n" \
"    - s3878104  Lee Dohwan \n" \
"    - s3924763  Huynh Nhat Anh\n"

void main()
{
    // set up serial console
	uart_init();

	// say hello
	uart_puts(WELCOME_MSG);

	// Initialize frame buffer
	framebf_init();

	// Draw something on the screen
	drawRectARGB32(100,100,400,400,0x00AA0000,1); //RED
	drawRectARGB32(150,150,400,400,0x0000BB00,1); //GREEN
	drawRectARGB32(200,200,400,400,0x000000CC,1); //BLUE
	drawRectARGB32(250,250,400,400,0x00FFFF00,1); //YELLOW
	drawPixelARGB32(300, 300, 0x00FF0000); //RED


	// echo everything back
	while(1) {
		//read each char
		char c = uart_getc();

		//send back 
		uart_sendc(c);
	}
}