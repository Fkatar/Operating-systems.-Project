/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

#include "memory.h"
#include "main.h"
#include "doctor.h"
//proj2
#include "hosptime.h"
#include "synchronization.h"
#include "close_sems-private.h"

#include <semaphore.h>
#include <fcntl.h>
#include <stdlib.h>

/*
 * Closes the semaphore
 */
void fechar(sem_t* semaphore){
    
    if (sem_close(semaphore) == -1){
        perror("Failed to close the semaphore");
    }
}

/*
 * Closes the semaphores without unlinking (used only in child processes)
 */
void close_sems(struct semaphores* sems, struct data_container* data, struct communication* comm ){
    fechar(sems->main_patient->full);
    fechar(sems->main_patient->empty);
    fechar(sems->main_patient->mutex);

    fechar(sems->patient_receptionist->full);
    fechar(sems->patient_receptionist->empty);
    fechar(sems->patient_receptionist->mutex);
    
    fechar(sems->receptionist_doctor->full);
    fechar(sems->receptionist_doctor->empty);
    fechar(sems->receptionist_doctor->mutex);

    fechar(sems->patient_stats_mutex);
    fechar(sems->receptionist_stats_mutex);
    fechar(sems->doctor_stats_mutex);

    fechar(sems->results_mutex);
    fechar(sems->terminate_mutex);

    deallocate_dynamic_memory(sems->main_patient);
    deallocate_dynamic_memory(sems->patient_receptionist);
    deallocate_dynamic_memory(sems->receptionist_doctor);
    deallocate_dynamic_memory(sems);

    deallocate_dynamic_memory(data->patient_pids);
    deallocate_dynamic_memory(data->receptionist_pids);
    deallocate_dynamic_memory(data->doctor_pids);

    deallocate_dynamic_memory(data);
    deallocate_dynamic_memory(comm->main_patient);
    deallocate_dynamic_memory(comm->patient_receptionist);
    deallocate_dynamic_memory(comm->receptionist_doctor);
    deallocate_dynamic_memory(comm);
}
