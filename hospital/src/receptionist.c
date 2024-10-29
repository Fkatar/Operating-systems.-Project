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
#include "receptionist.h"
//proj2
#include "hosptime.h"
#include "synchronization.h"
#include "close_sems-private.h"

/* Main function of a Receptionist. It should execute an infinite loop where in 
* each iteration it reads a patient admission and if it has an id 
* different from -1 and if data->terminate is still equal to 0, it processes it and
* forwards it to the doctors. Admissions with id equal to -1 are 
* ignored (invalid admission) and if data->terminate is equal to 1 it means that the 
* program termination order was given, so it should return the
* number of admissions performed. To perform these steps, you can use the
* other auxiliary methods defined in receptionist.h.
*/
int execute_receptionist(int receptionist_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {

    //process loop
    while(1) {

        // new empty admission
        struct admission *placeholder_ad = (struct admission *)allocate_dynamic_memory(sizeof(struct admission));
        placeholder_ad->id = -1;

        // fill new admission
        receptionist_receive_admission(placeholder_ad, data, comm, sems);

        if (placeholder_ad->id > -1) { // valid admission  
            placeholder_ad->receptionist_time = setTime();
            receptionist_process_admission(placeholder_ad, receptionist_id, data, sems); // if the admission read from the buffer is valid
            receptionist_send_admission(placeholder_ad, data, comm, sems);
        }

            deallocate_dynamic_memory(placeholder_ad); // free memory after being written to the buffer

        //Check the flag to see if it needs to close
        semaphore_lock(sems->terminate_mutex);
        if (*data->terminate == 1) {
            semaphore_unlock(sems->terminate_mutex);
            int temp = data->receptionist_stats[receptionist_id];
            close_sems(sems, data, comm);
            return temp;
        }   
        semaphore_unlock(sems->terminate_mutex);
    }
}

/* Function that reads an admission from the shared memory buffer between patients and receptionists.
* Before attempting to read the admission, it should check if data->terminate has value 1.
* If so, it immediately returns from the function.
*/
void receptionist_receive_admission(struct admission* ad, struct data_container* data, struct communication* comm, struct semaphores* sems) {

    // do not read if the close flag is on
    semaphore_lock(sems->terminate_mutex);
    if (*data->terminate == 1) {
        semaphore_unlock(sems->terminate_mutex);
        return;
    }
    semaphore_unlock(sems->terminate_mutex);

    //read from patient-receptionist buffer
    consume_begin(sems->patient_receptionist);
    read_patient_receptionist_buffer(comm->patient_receptionist, data->buffers_size, ad);
    consume_end(sems->patient_receptionist);

    
    if (ad->id == -1) {                                      // in case it reads an invalid admission
        produce_begin(sems->patient_receptionist);           // restore the empty and full values
        produce_end(sems->patient_receptionist);             // of the buffer semaphore
    }
    
}

/* Function that performs an admission, changing its receiving_receptionist field to the id
* passed as an argument, changing its status to 'R' (receptionist), and 
* incrementing the counter of admissions performed by this receptionist in the data_container. 
* It also updates the admission in the data structure.
*/
void receptionist_process_admission(struct admission* ad, int receptionist_id, struct data_container* data, struct semaphores* sems) {

    ad->receiving_receptionist = receptionist_id;
    ad->status = 'R';

    semaphore_lock(sems->receptionist_stats_mutex);
    data->receptionist_stats[receptionist_id]++;
    semaphore_unlock(sems->receptionist_stats_mutex);

    semaphore_lock(sems->results_mutex);
    data->results[ad->id] = *ad;
    semaphore_unlock(sems->results_mutex);
}

/* Function that writes an admission to the shared memory buffer between
* receptionists and doctors.
*/
void receptionist_send_admission(struct admission* ad, struct data_container* data, struct communication* comm, struct semaphores* sems) {
    
    //write to receptionist-doctor buffer
    produce_begin(sems->receptionist_doctor);
    write_receptionist_doctor_buffer(comm->receptionist_doctor, data->buffers_size, ad);
    produce_end(sems->receptionist_doctor);
}   
