/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include "memory.h"
#include "main.h"
#include "patient.h"
#include "hosptime.h"
#include "synchronization.h"
#include "close_sems-private.h"

/* Main function of a Patient. It should execute an infinite loop where in 
* each iteration it reads an admission from main and if data->terminate is still 
* equal to 0, processes it. Admissions with id equal to -1 are ignored
* (invalid admission) and if data->terminate is equal to 1 it means that a termination 
* order has been given, so it should return the number of requested 
* admissions. To perform these steps, you can use the other
* auxiliary methods defined in patient.h.
*/
int execute_patient(int patient_id, struct data_container* data, struct communication* comm, struct semaphores* sems){

    //process loop
    while (1) {

        //new empty admission
        struct admission *placeholder_ad = (struct admission*) allocate_dynamic_memory(sizeof(struct admission));
        placeholder_ad->id = -1;
        //time changes proj2

        //fill new admission
        patient_receive_admission(placeholder_ad, patient_id, data, comm, sems);

        if (placeholder_ad->id > -1) { // if the admission read from the buffer is valid
            placeholder_ad->patient_time = setTime();
            patient_process_admission(placeholder_ad, patient_id, data, sems); // if the admission read from the buffer is valid
            patient_send_admission(placeholder_ad, data, comm, sems);
        }

        deallocate_dynamic_memory(placeholder_ad); // free memory after being written to the buffer

        // Check the flag to see if it needs to close
        semaphore_lock(sems->terminate_mutex);
        if (*data->terminate == 1) {
            semaphore_unlock(sems->terminate_mutex);
            int temp = data->patient_stats[patient_id];
            close_sems(sems, data, comm);
            return temp;
        }
        semaphore_unlock(sems->terminate_mutex);
    }  
    
}

/* Function that reads an admission (from the shared memory buffer between main
* and the patients) that is directed to patient_id. Before attempting to read the admission, it should
* check if data->terminate has value 1. If so, it immediately returns from the function.
*/
void patient_receive_admission(struct admission* ad, int patient_id, struct data_container* data, struct communication* comm, struct semaphores* sems){

    // does not read if the close flag is on
    semaphore_lock(sems->terminate_mutex);
    if(*data->terminate == 1) {
        semaphore_unlock(sems->terminate_mutex);
        return; 
    }
    semaphore_unlock(sems->terminate_mutex);

    //read from main-patient buffer
    consume_begin(sems->main_patient);
    read_main_patient_buffer(comm->main_patient, patient_id, data->buffers_size, ad);
    consume_end(sems->main_patient);

    if (ad->id == -1) {                                 // in case it reads an invalid admission
        produce_begin(sems->main_patient);              // restore the empty and full values
        produce_end(sems->main_patient);                // of the buffer semaphore
    }
}

/* Function that validates an admission, changing its receiving_patient field to the patient_id
* passed as an argument, changing its status to 'P' (patient), and 
* incrementing the counter of admissions requested by this patient in the data_container. 
* It also updates the admission in the data structure.
*/
void patient_process_admission(struct admission* ad, int patient_id, struct data_container* data, struct semaphores* sems) {

    //patient verifies the admission
    ad->receiving_patient = patient_id;
    ad->status = 'P';
    
    semaphore_lock(sems->patient_stats_mutex);
    data->patient_stats[patient_id]++;
    semaphore_unlock(sems->patient_stats_mutex);

    semaphore_lock(sems->results_mutex);
    data->results[ad->id] = *ad;
    semaphore_unlock(sems->results_mutex);
}

/* Function that writes an admission to the shared memory buffer between the
* patients and the receptionists.
*/
void patient_send_admission(struct admission* ad, struct data_container* data, struct communication* comm, struct semaphores* sems){

    //send the admission to the patient-receptionist shared memory
    produce_begin(sems->patient_receptionist);
    write_patient_receptionist_buffer(comm->patient_receptionist, data->buffers_size, ad);
    produce_end(sems->patient_receptionist);
}
