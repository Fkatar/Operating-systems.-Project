#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

#include "synchronization.h"

/* Function that creates a new semaphore with name 'name' and initial value
* equal to 'value'. Can concatenate the result of the function getpid() to 
* 'name' to make the name unique for the process.
*/
sem_t * semaphore_create(char* name, int value){
    sem_t *sem = sem_open(name, O_CREAT, 0xFFFFFFFF, value);

    if (sem == SEM_FAILED) {
        perror("Error creating semaphore");
        exit(6);
    }
    return sem;
}

/* Function that destroys the semaphore passed as an argument.
*/
void semaphore_destroy(char* name, sem_t* semaphore){
    if (sem_close(semaphore) == -1){
        perror("Failed to close semaphore");
    }
    if (sem_unlink(name) == -1){
        perror("Failed to unlink semaphore");
    }
}

/* Function that starts the producing process, performing sem_wait on the 
* correct semaphores of the structure passed as an argument.
*/
void produce_begin(struct prodcons* pc){
    sem_wait(pc->empty);
    sem_wait(pc->mutex);
}

/* Function that ends the producing process, performing sem_post on the 
* correct semaphores of the structure passed as an argument.
*/
void produce_end(struct prodcons* pc){
    sem_post(pc->mutex);
    sem_post(pc->full);
}

/* Function that starts the consuming process, performing sem_wait on the 
* correct semaphores of the structure passed as an argument.
*/
void consume_begin(struct prodcons* pc){
    sem_wait(pc->full);
    sem_wait(pc->mutex);
}

/* Function that ends the consuming process, performing sem_post on the 
* correct semaphores of the structure passed as an argument.
*/
void consume_end(struct prodcons* pc){
    sem_post(pc->mutex);
    sem_post(pc->empty);
}

/* Function that performs wait on a semaphore.
*/
void semaphore_lock(sem_t* sem){
    sem_wait(sem);
}

/* Function that performs post on a semaphore.
*/
void semaphore_unlock(sem_t* sem){
    sem_post(sem);
}
