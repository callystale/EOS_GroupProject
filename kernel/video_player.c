#include "mbox.h"
#include "framebf.h"
#include "video_bitmap.h"
#include "video_player.h"
#include "../uart/uart1.h"

// This is additional function for playing video
void video_player(){
    uart_puts("Start of the video\n");
    volatile unsigned int *res_data;
    // Set up a mailbox buffer for configuring display properties
    mbox_set_physical_wh(500, 500, &res_data);
    // Make a mailbox call to set physical width and height
    mbox_call(ADDR(mBuf), MBOX_CH_PROP);
    for (int i = 1; i <= 3; i++){

        if(i == 1){
            for (int j = 0; j < 20; j++){
                // Display a frame from video 1 at position (0, 0) with size 682x384 pixels
                displayMultipleImages(video_bitmap_0allArray[j], 0, 0, 684, 384);
                // Wait for approximately 0.1 seconds (1000 milliseconds)
                wait_msec(FRAME_DELAY_US);
            }
            // Print a message indicating video 1 is done
            uart_puts("The first sequence is done\n");
        }
        else if(i == 2){
            for (int j = 0; j < 20; j++){
                // Display a frame from video 2 at position (0, 0) with size 682x384 pixels
                displayMultipleImages(video_bitmap_1allArray[j], 0, 0, 684, 384);
                // Wait for approximately 0.1 seconds (1000 milliseconds)
                wait_msec(FRAME_DELAY_US);
            }
            // Print a message indicating video 2 is done
            uart_puts("The second sequence is done\n");
        }
        else if(i == 3){
            for (int j = 0; j < 20; j++){
                 // Display a frame from video 3 at position (0, 0) with size 682x384 pixels
                displayMultipleImages(video_bitmap_2allArray[j], 0, 0, 684, 384);
                // Wait for approximately 0.1 seconds (1000 milliseconds)
                wait_msec(FRAME_DELAY_US);
            }
            // Print a message indicating video 3 is done
            uart_puts("The last sequence is done\n");
        }
        // Wait for approximately 0.1 seconds (100 milliseconds) before moving to the next video
        wait_msec(FRAME_DELAY_US);
        // Print a message indicating that 10 frames are done for the current video
        uart_puts("20 frames done\n");

    }
    uart_puts("End of the video\n");
}