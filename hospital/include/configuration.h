#ifndef CONFIGURATION_H_GUARD
#define CONFIGURATION_H_GUARD

#define MAXLINHA 256   // numero maximo de linhas
#define CONFIG_LINES 8 // numero de linhas de config

//Função que devolve os argumentos de entrada através de um caminho para o ficheiro que os guarda
char **get_arguments(char* filename);



//Função que liberta a memória utilizada para passar os argumentos de entrada
void deallocate_args(char **args);

#endif