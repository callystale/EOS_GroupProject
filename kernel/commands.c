#include "commands.h"
#include "mbox.h"
#include "../uart/uart1.h"
#include "framebf.h"
#include "../assets/background.h"
#include  "video_player.h"
#include "game.h"

#define FONT_HEIGHT 8
#define SYS_CLOCK 250000000
#define TAB_KEY 0x09
#define MAX_MATCHES 10
#define HISTORY_SIZE 10
#define MAX_CMD_LEN 128
#define UP_KEY '_'
#define DOWN_KEY '+'

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
        "task2a",
        "task2a",
        "Display names of all members and image.\r\n"
        "Example:\r\n  OkkOS> task2a"
    },
    {
        "task2b",
        "task2b", 
        "Display names of all members and image with video.\r\n"
        "Example:\r\n  OkkOS> task2b"
    },
    {
        "task3",
        "task3",
        "Run a small game.\r\n"
        "Example:\r\n  OkkOS> task3"
    },
     {
        "history",
        "history",
        "Show user's history of commands\r\n"
        "Example:\r\n  OkkOS> history"
    }
};

int command_count = sizeof(commands) / sizeof(commands[0]);


// Simple string copy function (if you don't have strcpy)
void strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

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




int run_command(char *input) {
    char cmd[32];
    char arg[32];
    split_command(input, cmd, arg);
    int handshake = 0;

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
            uart_puts("Handshake ");
            uart_puts(enable ? "enabled\r\n" : "disabled\r\n");
            if(enable) handshake = 1;
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
    else if(strcmp(cmd, "history") == 0){
        show_history();
    }
    else if (cmd[0] == '\0') {
        // empty input → do nothing
    } 
    else {
        uart_puts("Unknown command: ");
        uart_puts(cmd);
        uart_puts("\r\n");
    }

    return handshake;
}


// Find completions for the current input
int find_completions(char* input, char matches[][32]) {
    int match_count = 0;
    int input_len = strlen(input);
    
    // Find all commands that start with the input
    for (int i = 0; i < command_count && match_count < MAX_MATCHES; i++) {
        int matches_prefix = 1;
        
        // Check if command starts with input
        for (int j = 0; j < input_len; j++) {
            if (commands[i].name[j] != input[j]) {
                matches_prefix = 0;
                break;
            }
        }
        
        if (matches_prefix) {
            // Copy matching command to matches array
            int k = 0;
            while (commands[i].name[k] != '\0' && k < 31) {
                matches[match_count][k] = commands[i].name[k];
                k++;
            }
            matches[match_count][k] = '\0';
            match_count++;
        }
    }
    
    return match_count;
}

// Find the longest common prefix among matches
int find_common_prefix(char matches[][32], int count) {
    if (count == 0) return 0;
    if (count == 1) return strlen(matches[0]);
    
    int prefix_len = 0;
    
    while (1) {
        char first_char = matches[0][prefix_len];
        if (first_char == '\0') break;
        
        // Check if all matches have the same character at this position
        for (int i = 1; i < count; i++) {
            if (matches[i][prefix_len] != first_char || matches[i][prefix_len] == '\0') {
                return prefix_len;
            }
        }
        prefix_len++;
    }
    
    return prefix_len;
}

// Handle TAB key press for auto-completion
void handle_tab_completion(char* buffer, int* index) {
    char matches[MAX_MATCHES][32];
    int match_count = find_completions(buffer, matches);
    
    if (match_count == 0) {
        // No matches found - do nothing
        return;
    }
    
    if (match_count == 1) {
        // Exactly one match - complete it
        int current_len = *index;
        int completion_len = strlen(matches[0]);
        
        // Clear current input on screen
        for (int i = 0; i < current_len; i++) {
            uart_puts("\b \b");
        }
        
        // Copy completion to buffer
        for (int i = 0; i < completion_len; i++) {
            buffer[i] = matches[0][i];
        }
        buffer[completion_len] = ' ';  // Add space after command
        buffer[completion_len + 1] = '\0';
        *index = completion_len + 1;
        
        // Display the completed command
        uart_puts(matches[0]);
        uart_puts(" ");
    } else {
        // Multiple matches - ask user which one they mean
        int common_len = find_common_prefix(matches, match_count);
        int current_len = *index;
        
        // If common prefix is longer than current input, complete to common prefix
        if (common_len > current_len) {
            // Clear current input
            for (int i = 0; i < current_len; i++) {
                uart_puts("\b \b");
            }
            
            // Show common prefix
            for (int i = 0; i < common_len; i++) {
                buffer[i] = matches[0][i];
                uart_sendc(matches[0][i]);
            }
            buffer[common_len] = '\0';
            *index = common_len;
        } else {
            // Ask user which command they mean
            uart_puts("\r\nDid you mean: ");
            
            for (int i = 0; i < match_count; i++) {
                uart_puts(matches[i]);
                if (i < match_count - 2) {
                    uart_puts(", ");
                } else if (i == match_count - 2) {
                    uart_puts(" or ");
                }
            }
            uart_puts("?\r\n");
            
            // Redisplay prompt and current input
            uart_puts("OkkOS> ");
            for (int i = 0; i < *index; i++) {
                uart_sendc(buffer[i]);
            }
        }
    }
}

// Global variables for command history
static char command_history[HISTORY_SIZE][MAX_CMD_LEN];
static int history_write_index = 0;  // Where to write next command
static int history_count = 0;        // Total commands stored
static int history_browse_index = -1; // Current position when browsing (-1 = not browsing)

// Replace your add_to_history function with this debug version:
void add_to_history(const char* command) {
    uart_puts("[DEBUG] Adding to history: '");
    uart_puts(command);
    uart_puts("', length: ");
    uart_dec(strlen(command));
    uart_puts("\r\n");
    
    // Skip empty commands
    if (strlen(command) == 0) {
        uart_puts("[DEBUG] Skipping empty command\r\n");
        return;
    }
    
    // Don't add if same as last command
    if (history_count > 0) {
        int last_index = (history_write_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        uart_puts("[DEBUG] Comparing with last command: '");
        uart_puts(command_history[last_index]);
        uart_puts("'\r\n");
        
        if (strcmp(command_history[last_index], command) == 0) {
            uart_puts("[DEBUG] Skipping duplicate command\r\n");
            return;
        }
    }
    
    // Add command to history
    int len = strlen(command);
    if (len >= MAX_CMD_LEN) len = MAX_CMD_LEN - 1;
    
    uart_puts("[DEBUG] Adding command at index ");
    uart_dec(history_write_index);
    uart_puts("\r\n");
    
    for (int i = 0; i < len; i++) {
        command_history[history_write_index][i] = command[i];
    }
    command_history[history_write_index][len] = '\0';
    
    history_write_index = (history_write_index + 1) % HISTORY_SIZE;
    if (history_count < HISTORY_SIZE) {
        history_count++;
    }
    
    uart_puts("[DEBUG] History count now: ");
    uart_dec(history_count);
    uart_puts("\r\n");
    
    // Reset browse index
    history_browse_index = -1;
}

int browse_history(int direction, char* result_buffer) {
    if (history_count == 0) {
        return 0; // No history
    }
    
    if (history_browse_index == -1) {
        // Start browsing from the most recent command
        if (direction == -1) { // UP/previous
            history_browse_index = (history_write_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
            strcpy(result_buffer, command_history[history_browse_index]);
            return 1;
        } else {
            return 0; // Can't go forward from current position
        }
    } else {
        // Already browsing
        if (direction == -1) { // UP/previous (older commands)
            int next_index = (history_browse_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
            
            // Check if we've reached the oldest command
            int oldest_index = (history_write_index - history_count + HISTORY_SIZE) % HISTORY_SIZE;
            if (history_browse_index == oldest_index) {
                return 0; // Already at oldest
            }
            
            history_browse_index = next_index;
            strcpy(result_buffer, command_history[history_browse_index]);
            return 1;
        } else { // DOWN/next (newer commands)
            int next_index = (history_browse_index + 1) % HISTORY_SIZE;
            
            // Check if we've reached the newest command
            int newest_index = (history_write_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
            if (history_browse_index == newest_index) {
                history_browse_index = -1; // Reset to "current" position
                result_buffer[0] = '\0'; // Empty string
                return 2; // Signal to clear input
            }
            
            history_browse_index = next_index;
            strcpy(result_buffer, command_history[history_browse_index]);
            return 1;
        }
    }
}

// Handle history navigation
void handle_history_navigation(char key, char* buffer, int* index) {
    char history_cmd[MAX_CMD_LEN];
    int result = 0;
    
    if (key == UP_KEY) {
        result = browse_history(-1, history_cmd); // Previous command
    } else if (key == DOWN_KEY) {
        result = browse_history(1, history_cmd);  // Next command
    }
    
    if (result > 0) { // Command found or clear signal
        // Clear current input
        for (int i = 0; i < *index; i++) {
            uart_puts("\b \b");
        }
        
        // Copy history command to buffer
        int len = strlen(history_cmd);
        for (int i = 0; i < len && i < 127; i++) {
            buffer[i] = history_cmd[i];
        }
        buffer[len] = '\0';
        *index = len;
        
        // Display the command
        uart_puts(history_cmd);
    }
}

// Show command history (for debugging or as a command)
void show_history() {
    uart_puts("Command history:\r\n");
    if (history_count == 0) {
        uart_puts("  (no commands in history)\r\n");
        return;
    }
    
    // Show commands from oldest to newest
    int start_index = (history_write_index - history_count + HISTORY_SIZE) % HISTORY_SIZE;
    
    for (int i = 0; i < history_count; i++) {
        int index = (start_index + i) % HISTORY_SIZE;
        uart_puts("  ");
        uart_dec(i + 1);
        uart_puts(": ");
        uart_puts(command_history[index]);
        uart_puts("\r\n");
    }
}
