#include "main.h"
#include "log.h"
#include "hosptime.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Writes a line to the log
 */
void write_to_log(char* log_file, struct timespec t, char* command, char* args) {

    FILE *fpw = fopen(log_file, "a");
    char* time = formatted_time(t);
    fprintf(fpw, "%s %s %s\n", time, command, args);

    free(time);
    fclose(fpw);
}

/*
 * Clears the log
 */
void clear_log(char *log_file) {

    FILE *fpw = fopen(log_file, "w");
    fclose(fpw);
}
