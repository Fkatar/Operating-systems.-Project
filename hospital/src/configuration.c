#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuration.h"

/*
 * Reads the arguments for program initialization from the input file
 */
char** get_arguments(char *filename) {
    
    char linha[MAXLINHA];                                           // buffer to read line
    FILE *txtfile;                                                  
    char **args = (char **)malloc(CONFIG_LINES * sizeof(char*));   // array of strings with the args
    int i = 0;

    txtfile = fopen(filename, "r");

    if (txtfile == NULL) {
        perror("Failed to open the file with the arguments");
        exit(1);
    }
    while (fgets(linha, MAXLINHA, txtfile) != NULL && i < CONFIG_LINES) {

        int last = strlen(linha) - 1;
        if (linha[last] == '\n')            // remove the \n
            linha[last] = '\0'; 

        args[i] = (char *)malloc((strlen(linha) + 1) * sizeof(char));
        strcpy(args[i], linha);
        i++;
    }

    fclose(txtfile);
    return args;
}

/*
 * Frees the memory responsible for reading the arguments
 */
void deallocate_args(char** args) {
    
    for (int i = 0; i < CONFIG_LINES; i++) {
        free(args[i]);
    }

    free(args);
}