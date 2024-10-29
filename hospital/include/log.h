#ifndef LOG_H_GUARD
#define LOG_H_GUARD

void write_to_log(char *log_file, struct timespec t, char *command, char *args);

void clear_log(char *log_file);

#endif