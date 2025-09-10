#include "../uart/uart1.h"
#include "mbox.h"
#include "framebf.h"
#include "commands.h"
#include "video_bitmap.h"
#include "video_player.h"

#define TAB_KEY 0x09
#define UP_KEY '_'
#define DOWN_KEY '+'

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
"    - s3993986  Hong Thieu Kiet\n" \
"    - s3978567  Le Phuong Ngan\n" \
"    - s3990627  Nguyen Hoang Son\n" \
"    - s3878104  Lee Dohwan \n" \
"    - s3924763  Huynh Nhat Anh\n"


void main()
{   
    // set up serial console
    uart_init(0);

    // say hello
    uart_puts(WELCOME_MSG);
    

    // Initialize frame buffer
    framebf_init();

	// show initial prompt
    uart_puts("OkkOS> ");

    // For testing only: Draw something on the screen (unchanged) 
    drawRectARGB32(100,100,400,400,0x00AA0000,1); //RED
    drawRectARGB32(150,150,400,400,0x0000BB00,1); //GREEN
    drawRectARGB32(200,200,400,400,0x000000CC,1); //BLUE
    drawRectARGB32(250,250,400,400,0x00FFFF00,1); //YELLOW
    drawPixelARGB32(300, 300, 0x00FF0000); //RED
    drawString(0, 0, "HELLO WORLD !!! WELCOME TO OKKOS  ( ^ _ ^ ) ", 0x0000BB00, 1);
    drawPixelRGBA32(100,100,255,0,0,255); // should show RED
    drawPixelRGBA32(101,100,0,255,0,255); // should show GREEN
    drawPixelRGBA32(102,100,0,0,255,255); // should show BLUE
    

    // input buffer
    char buffer[128];
    int index = 0;


    // Replace the while(1) loop in your main() function with this:

    while (1) {
        char c = uart_getc();

        if (c == '\r' || c == '\n') {
            // Enter pressed → end command
            uart_puts("\r\n");
            buffer[index] = '\0';   // terminate string
            // Add command to history here
            add_to_history(buffer);
            if(run_command(buffer) == 1){
                uart_init(1); // reinitialize UART with handshake enabled
                uart_puts("Enabling handshake in UART setting\r\n");
            } else {
                uart_init(0); // reinitialize UART without handshake
            }
            index = 0;
            // reprint prompt immediately
            uart_puts("OkkOS> ");
        }
        else if (c == TAB_KEY) {
            // Handle TAB completion
            buffer[index] = '\0';  // Null terminate for completion
            handle_tab_completion(buffer, &index);
        }
        else if (c == UP_KEY || c == DOWN_KEY) {
        // Handle history navigation
            handle_history_navigation(c, buffer, &index);
            //uart_puts("[DEBUG]UP and DOWN key pressed\r\n");
        }
        else if (c == 127 || c == '\b') {
            // handle backspace
            if (index > 0) {
                index--;
                uart_puts("\b \b");  // erase last char from screen
            }
        }
        else if (c >= 32 && c <= 126) {  
            // printable characters
            if (index < sizeof(buffer)-1) {
                buffer[index++] = c;
                uart_sendc(c);       // echo char
            }
        } 
        // ignore other keys
    }
}
