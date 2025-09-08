#include "../uart/uart0.h"
#include "mbox.h"
#include "framebf.h"

void main()
{
    // set up serial console
	uart_init();

	// print on terminal
	uart_puts("\n\nDisplay Healthbar !\n");

	// Initialize frame buffer
	framebf_init();

	// Draw healthbars 
	//RED with low health
	drawHealthBar(width - 200, height - 40, width - 1, height - 25, 0xFFFFFFFF, 0x00AA0000, 1); //RED with white border

	//GREEN with full health
	drawHealthBar(width - 200, height - 20, width - 100, height - 5, 0xFFFFFFFF, 0x0000AA00, 1); //GREEN with white border


	// echo everything back
	while(1) {
		//read each char
		char c = uart_getc();

		//send back 
		uart_sendc(c);
	}
}