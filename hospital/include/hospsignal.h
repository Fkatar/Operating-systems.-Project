
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void escreveAdmissoes(struct data_container *data);

void ctrlC();

int signal_handle(struct data_container *data, struct communication *comm, struct semaphores *sems);