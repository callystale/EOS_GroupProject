#include "commands.h"
#include "mbox.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "../assets/background.h"
#include  "video_player.h"
#include "game.h"

#define FONT_HEIGHT 8
#define SYS_CLOCK 250000000

typedef struct {
    const char *name;
    const char *usage;
    const char *info;
} Command;

Command commands[] = {
    {
        "help",
        "help <command_name>",
        "Show brief information of all commands.\r\n"
        "If a command name is provided, show full information of that command.\r\n"
        "Example:\r\n  OkkOS> help\r\n  OkkOS> help showinfo"
    },
    {
        "clear",
        "clear",
        "Clear screen (in our terminal it will scroll down to current position of the cursor).\r\n"
        "Example:\r\n  OkkOS> clear"
    },
    {
        "showinfo",
        "showinfo",
        "Show board revision (value and information) and board MAC address in correct format.\r\n"
        "Example:\r\n  OkkOS> showinfo"
    },
    {
        "baudrate",
        "baudrate <value>",
        "Allow the user to change the baudrate of the current UART being used.\r\n"
        "Supported baud rates: 9600, 19200, 38400, 57600, 115200.\r\n"
        "Example:\r\n  OkkOS> baudrate 115200"
    },
    {
        "handshake",
        "handshake",
        "Allow the user to turn on/off CTS/RTS handshaking on current UART if possible.\r\n"
        "Example:\r\n  OkkOS> handshake"
    },
    {
        "task",
        "task<number>",
        "\n task2a: Display names of all members and image \n task2b: Display names of all members and image \n task3: Run a small game\n"
        "Example:\r\n  OkkOS> task2"
    }
};

int command_count = sizeof(commands) / sizeof(commands[0]);


int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

// Simple function to split a string into first word and the rest
void split_command(char *input, char *cmd, char *arg) {
    int i = 0, j = 0;

    // extract command
    while (input[i] != ' ' && input[i] != '\0') {
        cmd[j++] = input[i++];
    }
    cmd[j] = '\0';

    // skip spaces
    while (input[i] == ' ') i++;

    // copy the rest as argument
    j = 0;
    while (input[i] != '\0') {
        arg[j++] = input[i++];
    }
    arg[j] = '\0';
}

void brief_help(){
    uart_puts("Available commands:\r\n");
            for (int i = 0; i < command_count; i++) {
                uart_puts("  ");
                uart_puts(commands[i].name);
                // pad spaces so usages align (let's assume max name length = 10)
                int len = strlen(commands[i].name);
                int pad = 12 - len; // adjust 12 based on desired column width
                for (int j = 0; j < pad; j++) {
                    uart_puts(" ");
                }
                uart_puts(" - ");
                uart_puts(commands[i].usage);
                uart_puts("\r\n");
            }
            uart_puts("\r\nType 'help <command>' for more details.\r\n");
}

void detailed_help(char *arg){
    // show details for specific command
    int found = 0;
    for (int i = 0; i < command_count; i++) {
        if (strcmp(arg, commands[i].name) == 0) {
            uart_puts("Command: ");
            uart_puts(commands[i].name);
            uart_puts("\r\nUsage:   ");
            uart_puts(commands[i].usage);
            uart_puts("\r\nInfo:    ");
            uart_puts(commands[i].info);
            uart_puts("\r\n");
            found = 1;
            break;
        }
    }
    if (!found) {
        uart_puts("Unknown command: ");
        uart_puts(arg);
        uart_puts("\r\n");
    }
}
void clear_command() {
    uart_puts("\033[2J\033[H");
    
}



void get_board_info() {
    // mailbox data buffer
    mBuf[0] = 12*4;
    mBuf[1] = MBOX_REQUEST;
    
    mBuf[2] = 0x00010002; // Get board revision
    mBuf[3] = 4;
    mBuf[4] = 0;
    mBuf[5] = 0;

    mBuf[6] = 0x00010003; // Get MAC address
    mBuf[7] = 8;
    mBuf[8] = 0;
    mBuf[9] = 0;
    mBuf[10] = 0;

    mBuf[11] = MBOX_TAG_LAST;

    // send request
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        // print revision
        uart_puts("Board Revision: 0x");
        uart_hex(mBuf[5]);
        uart_puts("\r\n");

        // print MAC address
        uart_puts("Board MAC Address: ");
        unsigned char *mac = (unsigned char *)&mBuf[9];
        for (int i = 0; i < 6; i++) {
            uart_hex_byte(mac[i]);
            if (i < 5) uart_puts(":");
        }
        uart_puts("\r\n");
    } else {
        uart_puts("Failed to get board info\r\n");
    }
}

void task_2a_display_names() 
{   static char *names[] = {
    "Thieu Kiet", "Phuong Ngan", "Hoang Son", "Lee Dohwan", "Nhat Anh"
    };
    static unsigned int colors[] = {
        0x00FFFFFF, 0x00FFD700, 0x00FF0000, 0x0000FFFF, 0x005E44C8
    };
    static int x_positions[] = {20, 110, 210, 300, 400};
    int y_position = 220;  

    drawImage(background_data, 0, 0, BG_WIDTH, BG_HEIGHT);
    uart_puts("Background image displayed.\r\n");
   
    for (int i = 0; i < 5; i++) {
        drawString(x_positions[i], y_position, names[i], colors[i], 1);
    }
    uart_puts("Displayed names on the screen.\r\n");

}

int atoi(const char *str)
{
    int res = 0, sign = 1;
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    while (*str >= '0' && *str <= '9')
    {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res * sign;
}

void set_baudrate(unsigned int baudrate) {
    unsigned int baud_reg = (SYS_CLOCK / (8 * baudrate)) - 1;
    AUX_MU_BAUD = baud_reg;   // no need for *
}

void set_handshake(int enable) {
    unsigned int val = AUX_MU_CNTL;  // read register

    if (enable) {
        val |= (1 << 2);   // Enable RTS auto-flow
    } else {
        val &= ~(1 << 2);  // Disable RTS auto-flow
    }

    AUX_MU_CNTL = val;     // write back to register
}



void run_command(char *input) {
    char cmd[32];
    char arg[32];
    split_command(input, cmd, arg);

    if (strcmp(cmd, "help") == 0) {
        if (arg[0] == '\0') {
            // no argument → show brief info
            brief_help();
        } else {
            detailed_help(arg);
        }
    } 
    else if(strcmp(cmd, "clear") == 0){
        clear_command();;
    }
    else if(strcmp(cmd, "showinfo") == 0){
        get_board_info();
    }
    else if(strcmp(cmd, "baudrate") == 0){
        if (arg[0] != '\0') {
            unsigned int baud = atoi(arg);   // convert argument to int
            set_baudrate(baud);
            uart_puts("Baudrate set to ");
            uart_dec(baud);
            uart_puts("\r\n");
        } else {
            uart_puts("Usage: baudrate <value>\r\n");
        }
    }
    else if(strcmp(cmd, "handshake") == 0){
        if (arg[0] != '\0') {
            int enable = atoi(arg);   // 0 = disable, nonzero = enable
            set_handshake(enable);
            uart_puts("Handshake ");
            uart_puts(enable ? "enabled\r\n" : "disabled\r\n");
        } else {
            uart_puts("Usage: handshake <0|1>\r\n");
        }
    }
    else if(strcmp(cmd, "task2a") == 0){
        clear_screen();
        task_2a_display_names();
    }
    else if(strcmp(cmd, "task2b") == 0){
        clear_screen();
        video_player();
    }
    else if(strcmp(cmd, "task3") == 0){
        clear_screen();
        game();
    }
    else if (cmd[0] == '\0') {
        // empty input → do nothing
    } 
    else {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\r\n");
    }
}

