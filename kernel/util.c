#include "util.h"
#include "../uart/uart1.h"

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
        "run task<number>",
        "task2: Display names of all members, image and video \t task3: run a small game\n"
        "Example:\r\n  OkkOS> run task2"
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

void clear_screen() {
    uart_puts("\033[2J"); // Clear entire screen
    uart_puts("\033[H");  // Move cursor to top-left
}


void run_command(char *input) {
    char cmd[32];
    char arg[32];
    split_command(input, cmd, arg);

    if (strcmp(cmd, "help") == 0) {
        if (arg[0] == '\0') {
            // no argument → show brief info
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
        } else {
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
    } 
    else if(strcmp(cmd, "clear") == 0){
        clear_screen();
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

