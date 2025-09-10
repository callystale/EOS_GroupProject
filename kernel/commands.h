// commands.h
#ifndef UTIL_H
#define UTIL_H

int run_command(char *buffer);
void handle_tab_completion(char* buffer, int* index);
void task_2i_display_names();
void add_to_history(const char* command);
void handle_history_navigation(char key, char* buffer, int* index);
void show_history(void);
#endif
