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

/* Main function of a Doctor. It should execute an infinite loop where in 
* each iteration it reads an admission and if data->terminate is still 0, processes it and
* writes it to the results. Admissions with id equal to -1 are ignored
* (invalid admission) and if data->terminate is 1 it means the program termination order
* was given, so it should return the number of consultations performed. To perform these steps,
* it can use the other auxiliary methods defined in doctor.h.
*/
int execute_doctor(int doctor_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {

    // process loop
    while (1) {

        // new empty admission
        struct admission *placeholder_ad = (struct admission *)allocate_dynamic_memory(sizeof(struct admission));
        placeholder_ad->id = -1;

        // fill new admission
        doctor_receive_admission(placeholder_ad, doctor_id, data, comm, sems);

        if (placeholder_ad->id > -1) {
            placeholder_ad->doctor_time = setTime();
            doctor_process_admission(placeholder_ad, doctor_id, data, sems); // if the admission read from the buffer is valid
        }
  
        deallocate_dynamic_memory(placeholder_ad); // free memory after being written to the buffer

        // Check the flag to know if it needs to close
        semaphore_lock(sems->terminate_mutex);
        if (*data->terminate == 1) {
            semaphore_unlock(sems->terminate_mutex);
            int temp = data->doctor_stats[doctor_id];
            close_sems(sems, data, comm);
            return temp;
        }
        semaphore_unlock(sems->terminate_mutex);
    }
}

/* Function that reads an admission (from the shared memory buffer between
* the receptionists and the doctors) that is directed to the doctor doctor_id.
* Before attempting to read the admission, the process should
* check if data->terminate has value 1.
* If so, it immediately returns from the function.
*/
void doctor_receive_admission(struct admission* ad, int doctor_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {

    // do not read if the close flag is on
    semaphore_lock(sems->terminate_mutex);
    if (*data->terminate == 1) {
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    // read from receptionist-doctor buffer
    consume_begin(sems->receptionist_doctor);
    read_receptionist_doctor_buffer(comm->receptionist_doctor, doctor_id, data->buffers_size, ad);
    consume_end(sems->receptionist_doctor);

    if (ad->id == -1) {                                     // in case it reads an invalid admission
        produce_begin(sems->receptionist_doctor);           // restore the empty and full values
        produce_end(sems->receptionist_doctor);             // of the buffer semaphore
    }
}

/*
 * Total admissions completed in the hospital
 */
int totalAdmissions(int* doctor_stats, int n_doctors) {

    int total = 0;

    for (int i = 0; i < n_doctors; i++) {
        total += doctor_stats[i];
    }
    return total;
}

/* Function that processes an admission, changing its receiving_doctor field to the id
* passed as an argument, changing its status to 'A' and 
* incrementing the consultation counter in the data_container or to 'N'. 
* It also updates the admission in the data structure.
*/
void doctor_process_admission(struct admission* ad, int doctor_id, struct data_container* data, struct semaphores* sems) {

    ad->receiving_doctor = doctor_id;
    int current_ads = totalAdmissions(data->doctor_stats, data->n_doctors);

    if (current_ads == data->max_ads) {  // do not attend if the maximum number of consultations has been reached
        ad->status = 'N';
    }
    else {
        ad->status = 'A';
        semaphore_lock(sems->doctor_stats_mutex);
        data->doctor_stats[doctor_id]++;
        semaphore_unlock(sems->doctor_stats_mutex);

        semaphore_lock(sems->results_mutex);
        data->results[ad->id] = *ad;
        semaphore_unlock(sems->results_mutex);
    }
}
