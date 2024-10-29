/**
Grupo 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "hosptime.h"


/*
 * Returns a timespec structure with the current time
 */
struct timespec setTime() {
    static struct timespec t;

    clock_gettime(CLOCK_REALTIME, &t);
    return t;
}

/*
 * Formats a timespec structure to dd/mm/yyyy_hh:mm:ss
 */
char* formatted_time(struct timespec ts) {

    char *formatted = malloc(50); 
    char temp[15];
    struct tm *tm;
    
    //convert ts to tm
    tm = localtime(&ts.tv_sec);

    //format
    strftime(formatted, 50, "%d/%m/%Y_%H:%M:%S", tm);
    snprintf(temp, 15, ".%03ld", ts.tv_nsec / 1000000);
    strcat(formatted, temp);

    return formatted;
}