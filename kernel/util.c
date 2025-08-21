#include "util.h"
#include "../uart/uart1.h"

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

void run_command(char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        uart_puts("Available commands:\r\n");
        uart_puts("  help                   - Show brief information of all commands\r\n");
        uart_puts("  help <command_name>    - Show full information of a specific command\r\n");
        uart_puts("  clear                  - Clear screen (in our terminal it will scroll down to current position of the cursor)\r\n");
        uart_puts("  showinfo               - Show board revision (value and information) and board MAC address in correct format\r\n");
        uart_puts("  baudrate               - Allow the user to change the baudrate of current UART being used\r\n");
        uart_puts("  handshake              - Allow the user to turn on/off CTS/RTS handsharking on current UART if possible\r\n");
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