/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#include "process.h"
#include "memory.h"
#include "main.h"
#include "hospsignal.h"
#include "configuration.h"
#include "hosptime.h"
#include "stats.h"
#include "synchronization.h"
#include "log.h"



int main(int argc, char *argv[]) {
    
    // Initialize data structures
    struct data_container* data = allocate_dynamic_memory(sizeof(struct data_container));
    struct communication* comm = allocate_dynamic_memory(sizeof(struct communication));
    comm->main_patient = allocate_dynamic_memory(sizeof(struct circular_buffer));
    comm->patient_receptionist = allocate_dynamic_memory(sizeof(struct rnd_access_buffer));
    comm->receptionist_doctor = allocate_dynamic_memory(sizeof(struct circular_buffer));

    struct semaphores *sems = allocate_dynamic_memory(sizeof(struct semaphores));
    sems->main_patient = allocate_dynamic_memory(sizeof(struct prodcons));
    sems->patient_receptionist = allocate_dynamic_memory(sizeof(struct prodcons));
    sems->receptionist_doctor = allocate_dynamic_memory(sizeof(struct prodcons));

    // Execute main code
    main_args(argc, argv, data);
    clear_log(data->log_filename);
    allocate_dynamic_memory_buffers(data);
    create_shared_memory_buffers(data, comm);
    create_semaphores(data, sems);
    launch_processes(data, comm, sems);
    user_interaction(data, comm, sems);

    // Release memory before terminating
    deallocate_dynamic_memory(data);
    deallocate_dynamic_memory(comm->main_patient);
    deallocate_dynamic_memory(comm->patient_receptionist);
    deallocate_dynamic_memory(comm->receptionist_doctor);
    deallocate_dynamic_memory(comm);
    deallocate_dynamic_memory(sems->main_patient);
    deallocate_dynamic_memory(sems->patient_receptionist);
    deallocate_dynamic_memory(sems->receptionist_doctor);
    deallocate_dynamic_memory(sems);
}

void print_admission(struct data_container* data, int admission_id) {

    struct admission* ad = &data->results[admission_id];
    printf("admission id: %d\n"\
           "requesting patient id: %d\n"\
           "requested doctor id: %d\n"\
           "status: %c\n"\
           "receiving_patient: %d\n"\
           "receiving_receptionist: %d\n"\
           "receiving_doctor: %d\n",

    ad->id, ad->requesting_patient, ad->requested_doctor, ad->status,
    ad->receiving_patient, ad->receiving_receptionist, ad->receiving_doctor);
}

/* Function that reads the application argument, namely the configuration file
 * with the maximum number of admissions, the size of the shared memory buffers
 * used for communication, and the number of patients, receptionists, and
 * doctors, among others. Stores this information in the appropriate fields of
 * the data_container structure.
 */
void main_args(int argc, char* argv[], struct data_container* data) {
    

    if (argc >= 2) {
        char **args = get_arguments(argv[1]);

        data->max_ads = atoi(args[0]);
        data->buffers_size = atoi(args[1]);
        data->n_patients = atoi(args[2]);
        data->n_receptionists = atoi(args[3]);
        data->n_doctors = atoi(args[4]);
        strcpy(data->log_filename, args[5]);
        strcpy(data->statistics_filename, args[6]);
        data->alarm_time = atoi(args[7]);

        deallocate_args(args);
    } 
    
}
/* Function that reserves the dynamic memory needed for the execution
 * of hOSpital, namely for the *_pids arrays of the data_container structure.
 * For this, the allocate_dynamic_memory function can be used.
 */
void allocate_dynamic_memory_buffers(struct data_container* data) {

    // Memory allocation for process pids
    data->patient_pids = (int *) allocate_dynamic_memory(data->n_patients * sizeof(int));
    data->receptionist_pids = (int *) allocate_dynamic_memory(data->n_receptionists * sizeof(int));
    data->doctor_pids = (int *) allocate_dynamic_memory(data->n_doctors * sizeof(int));
}

/* Function that reserves the shared memory needed for the execution of
 * hOSpital. It is necessary to reserve shared memory for all buffers of the
 * communication structure, including the buffers themselves and respective
 * pointers, as well as for the data->results, *_stats arrays and
 * the data->terminate variable. For this, the create_shared_memory function can be used.
 */
void create_shared_memory_buffers(struct data_container* data, struct communication* comm) {

    // Memory allocation for the elements of the main-patient buffer
    comm->main_patient->ptrs = (struct pointers*) create_shared_memory(STR_SHM_MAIN_PATIENT_PTR,sizeof(struct pointers) * data->buffers_size);
    comm->main_patient->buffer = (struct admission*) create_shared_memory(STR_SHM_MAIN_PATIENT_BUFFER, sizeof(struct admission) * data->buffers_size);

    // Memory allocation for the elements of the patient-receptionist buffer
    comm->patient_receptionist->buffer = (struct admission *)create_shared_memory(STR_SHM_PATIENT_RECEPT_BUFFER, sizeof(struct admission) * data->buffers_size);
    comm->patient_receptionist->ptrs = (int *)create_shared_memory(STR_SHM_PATIENT_RECEPT_PTR, sizeof(struct pointers) * data->buffers_size);

    // Memory allocation for the elements of the receptionist-doctor buffer
    comm->receptionist_doctor->ptrs = (struct pointers*) create_shared_memory(STR_SHM_RECEPT_DOCTOR_PTR,sizeof(struct pointers) * data->buffers_size);
    comm->receptionist_doctor->buffer = (struct admission *)create_shared_memory(STR_SHM_RECEPT_DOCTOR_BUFFER, sizeof(struct admission) * data->buffers_size);

    // patient_stats
    data->patient_stats = (int*)create_shared_memory(STR_SHM_PATIENT_STATS, sizeof(int) * data->n_patients);
    data->receptionist_stats = (int*)create_shared_memory(STR_SHM_RECEPT_STATS, sizeof(int) * data->n_receptionists);
    data->doctor_stats = (int*)create_shared_memory(STR_SHM_DOCTOR_STATS, sizeof(int) * data->n_doctors);

    // Memory allocation for the results
    data->results = (struct admission*)create_shared_memory(STR_SHM_RESULTS, sizeof(struct admission) * data->max_ads);

    // Memory allocation for the termination flag
    data->terminate = (int*)create_shared_memory(STR_SHM_TERMINATE, sizeof(int));

    // Initialize a default id for all future admissions
    for (int i = 0; i < data->max_ads; i++) {
        data->results[i].id = -1;
    }

}

/* Function that starts the processes of patients, receptionists, and
 * doctors. For this, the launch_* functions can be used,
 * storing the resulting pids in the respective arrays
 * of the data_container structure.
 */
void launch_processes(struct data_container* data, struct communication* comm, struct semaphores* sems) {

    // Initialization of patients
    for (int i = 0; i < data->n_patients; i++) {
        int pid = launch_patient(i, data, comm, sems);
        data->patient_pids[i] = pid;
    }

    // Initialization of receptionists
    for (int j = 0; j < data->n_receptionists; j++) {
        int pid = launch_receptionist(j, data, comm, sems);
        data->receptionist_pids[j] = pid;
    }

    // Initialization of doctors
    for (int k = 0; k < data->n_doctors; k++) {
        int pid = launch_doctor(k, data, comm, sems);
        data->doctor_pids[k] = pid;
    }
}

/*
* Displays the interaction options to the console
*/
void menu() {

    printf("[Main] Available actions:\n"
           "[Main] ad patient doctor - create a new admission\n"
           "[Main] status - displays the current state of the variables contained in the data_container\n"
           "[Main] info id - check the status of an admission\n"
           "[Main] help - prints information about the available actions.\n"
           "[Main] end - terminates the execution of hOSpital.");
}

/* Function that handles user interaction, which can receive 5 commands:
 * ad patient doctor - creates a new admission, through the create_request function
 * info - checks the status of an admission through the read_info function
 * status - displays the current state of the variables contained in the data_container
 * help - prints information about the available commands
 * end - terminates the execution of hOSpital through the stop_execution function
 */
void user_interaction(struct data_container *data, struct communication *comm, struct semaphores* sems) {

    int ad_counter_value = 0;
    int *ad_counter = &ad_counter_value;
    // Initialize time
    time_t t;
    time_t t2;
    time(&t);
    
    time_t current_time;
    int interval = data->alarm_time;

    int signal = signal_handle(data, comm, sems);
    menu(); // Prints the interaction options

    while (1) {
        printf("\n[Main] Enter action: ");
        char input[5];
        scanf("%s", input);

        // Signals
        signal = signal_handle(data, comm, sems);

        if(signal == 1) {
            time(&current_time);
            break;
        }
        
        if (strcmp(input, "ad") == 0) {
            create_request(ad_counter, data, comm, sems);
        }
        else if (strcmp(input, "info") == 0) {
            read_info(data, sems);
            write_to_log(data->log_filename, setTime(), "info", "");
        }
        
        else if (strcmp(input, "status") == 0) {   
            print_status(data, sems);
            write_to_log(data->log_filename, setTime(), "status","");
        }
        else if (strcmp(input, "help") == 0) {
            menu();
            write_to_log(data->log_filename, setTime(), "help", "");
        }
        else if (strcmp(input, "end") == 0) {   
            end_execution(data, comm, sems); 
            break; // Exit the loop when "end" is entered
        }

        else printf("Invalid input\n");
        

        // Alarm
        time(&t2);
        if(t + interval < t2){
            t = t2;
            escreveAdmissoes(data);
        }
    }
}
/* Creates a new admission identified by the current value of ad_counter and with the
 * data entered by the user on the command line, writing it to the shared memory buffer
 * between the main and the patients. Prints the admission id and increments the
 * admission counter ad_counter.
 */
void create_request(int* ad_counter, struct data_container* data, struct communication* comm, struct semaphores* sems) {
    // allocate memory for a new admission
    struct admission *ad_mp = (struct admission*) allocate_dynamic_memory(sizeof(struct admission));
    int p, d;

    char* args = malloc(10);
    ad_mp->create_time = setTime();

    // get the requested patient and doctor ids
    scanf("%d %d", &p, &d);
    sprintf(args, "%d %d", p, d);

    ad_mp->id = *ad_counter;
    ad_mp->requesting_patient = p;
    ad_mp->requested_doctor = d;
    ad_mp->status = 'M';

    // write the admission status to the results and display it on the screen
    data->results[*ad_counter] = *ad_mp;
    printf("admission id: %d created.\n", *ad_counter);
    write_to_log(data->log_filename, setTime(), "ad", args);
    // write to the main-patient buffer
    produce_begin(sems->main_patient);
    write_main_patient_buffer(comm->main_patient, data->buffers_size, ad_mp);
    produce_end(sems->main_patient);
    deallocate_dynamic_memory(ad_mp);

    free(args);

    // prepare the id for the next admission
    (*ad_counter)++;
}

/* Function that reads an admission id from the user and checks if it is valid.
 * If valid, prints its information, including its status, the id of the patient
 * who made the request, the id of the requested doctor, and the ids of the patient,
 * receptionist, and doctor who received and processed it.
 */
void read_info(struct data_container* data, struct semaphores* sems) {
    int ad_id;

    // id to check the status
    scanf("%d", &ad_id);

    // check if the requested admission is valid
    semaphore_lock(sems->results_mutex);
    if (ad_id < data->max_ads && (data->results[ad_id].id == ad_id)) {
        print_admission(data, ad_id);
    }
    semaphore_unlock(sems->results_mutex);
}

/* Function that prints the status of the data_container, including all its fields.
 * For arrays, it should be printed in the format [0, 1, 2, ..., N], where N is the last element of the array.
 */
void print_status(struct data_container* data, struct semaphores* sems) {

    printf("Maximum admissions: %d\n", data->max_ads);
    printf("Buffer size: %d\n", data->buffers_size);
    printf("Number of patients: %d\n", data->n_patients);
    printf("Number of receptionists: %d\n", data->n_receptionists);
    printf("Number of doctors: %d\n", data->n_doctors);

    printf("Patient pids: [");
    for (int i = 0; i < data->n_patients; i++) {
        printf("%d", data->patient_pids[i]);
        if (i < data->n_patients - 1) printf(", ");
    }
    printf("]\nReceptionist pids: [");
    for (int i = 0; i < data->n_receptionists; i++) {
        printf("%d ", data->receptionist_pids[i]);
        if (i < data->n_receptionists - 1) printf(", ");
    }
    printf("]\nDoctor pids: [");
    for (int i = 0; i < data->n_doctors; i++) {
        printf("%d ", data->doctor_pids[i]);
        if (i < data->n_doctors - 1) printf(", ");
    }

    semaphore_lock(sems->patient_stats_mutex);
    printf("]\nPatient statistics: [");
    for (int i = 0; i < data->n_patients; i++) {
        printf("%d ", data->patient_stats[i]);
        if (i < data->n_patients - 1) printf(", ");
    }
    semaphore_unlock(sems->patient_stats_mutex);

    semaphore_lock(sems->receptionist_stats_mutex);
    printf("]\nReceptionist statistics: [");
    for (int i = 0; i < data->n_receptionists; i++) {
        printf("%d ", data->receptionist_stats[i]);
        if (i < data->n_receptionists - 1) printf(", ");
    }
    semaphore_unlock(sems->receptionist_stats_mutex);

    semaphore_lock(sems->doctor_stats_mutex);
    printf("]\nDoctor statistics: [");
    for (int i = 0; i < data->n_doctors; i++) {
        printf("%d ", data->doctor_stats[i]);
        if (i < data->n_doctors - 1) printf(", ");
    }
    semaphore_unlock(sems->doctor_stats_mutex);

    semaphore_lock(sems->results_mutex);
    printf("]\nResults: [");
    for (int l = 0; l < data->max_ads; l++) {
        if (data->results[l].id == -1) break;
        print_admission(data, l);
    }
    semaphore_unlock(sems->results_mutex);

    semaphore_lock(sems->terminate_mutex);
    printf("\nFinished: %d ", *data->terminate);
    semaphore_unlock(sems->terminate_mutex);
}
/* Function that terminates the execution of the hOSpital program. It should start by
 * setting the flag data->terminate to 1. Then, in this order, it should wait for the child
 * processes to terminate, write the final statistics of the program, and finally release
 * the previously reserved shared and dynamic memory areas. For this, you can use the other
 * auxiliary functions from main.h.
 */
void end_execution(struct data_container* data, struct communication* comm, struct semaphores* sems) {

    write_to_log(data->log_filename, setTime(), "end", "");

    *data->terminate = 1;       // change the terminate flag and wait for the processes to end

    wakeup_processes(data, sems);
    wait_processes(data);
    write_stats(data);

    destroy_semaphores(sems);
    destroy_memory_buffers(data, comm);
}

/* Function that waits for all previously started processes to terminate,
 * including patients, receptionists, and doctors. For this, you can use the
 * wait_process function from process.h.
 */
void wait_processes(struct data_container* data) {

    // wait for patient processes
    for (int i = 0; i < data->n_patients; i++) {
        wait_process(data->patient_pids[i]);
    }

    // wait for receptionist processes
    for (int j = 0; j < data->n_receptionists; j++) {
        wait_process(data->receptionist_pids[j]);
    }
    // wait for doctor processes
    for (int k = 0; k < data->n_doctors; k++) {
        wait_process(data->doctor_pids[k]);
    }
    
}

/* Function that prints the final statistics of hOSpital, namely how many
 * admissions were requested by each patient, performed by each receptionist,
 * and attended by each doctor.
 */
void write_statistics(struct data_container* data) {

    // patient statistics
    printf("Patients: ");
    for (int i = 0; i < data->n_patients; i++) {
        printf("patient %d : %d\n", data->patient_pids[i], data->patient_stats[i]);
    }

    // receptionist statistics
    printf("\nReceptionists: ");
    for (int i = 0; i < data->n_receptionists; i++) {
        printf("receptionist %d: %d\n", data->receptionist_pids[i], data->receptionist_stats[i]);
    }

    // doctor statistics
    printf("\nDoctors: ");
    for (int i = 0; i < data->n_doctors; i++) {
        printf("doctor %d : %d\n", data->doctor_pids[i], data->doctor_stats[i]);
    }
}

/* Function that releases all previously reserved dynamic and shared memory buffers
 * in the data structure.
 */
void destroy_memory_buffers(struct data_container* data, struct communication* comm) {

    // release shared memory areas
    destroy_shared_memory(STR_SHM_MAIN_PATIENT_PTR, comm->main_patient->ptrs, sizeof(struct pointers) * data->buffers_size);
    destroy_shared_memory(STR_SHM_PATIENT_RECEPT_PTR, comm->patient_receptionist->ptrs,sizeof(struct pointers) * data->buffers_size);    
    destroy_shared_memory(STR_SHM_RECEPT_DOCTOR_PTR, comm->receptionist_doctor->ptrs,sizeof(struct pointers) * data->buffers_size);  
    destroy_shared_memory(STR_SHM_RECEPT_DOCTOR_BUFFER, comm->receptionist_doctor->buffer, sizeof(struct admission) * data->buffers_size);
    destroy_shared_memory(STR_SHM_MAIN_PATIENT_BUFFER, comm->main_patient->buffer, sizeof(struct admission) * data->buffers_size);
    destroy_shared_memory(STR_SHM_PATIENT_RECEPT_BUFFER, comm->patient_receptionist->buffer, sizeof(struct admission) * data->buffers_size);
    destroy_shared_memory(STR_SHM_TERMINATE, data->terminate, sizeof(int));
    destroy_shared_memory(STR_SHM_RESULTS, data->results, sizeof(struct admission) * data->max_ads);
    destroy_shared_memory(STR_SHM_PATIENT_STATS, data->patient_stats, sizeof(int) * data->n_patients);
    destroy_shared_memory(STR_SHM_RECEPT_STATS, data->receptionist_stats, sizeof(int) * data->n_receptionists);
    destroy_shared_memory(STR_SHM_DOCTOR_STATS, data->doctor_stats, sizeof(int) * data->n_doctors);

    // release dynamic memory
    deallocate_dynamic_memory(data->patient_pids);
    deallocate_dynamic_memory(data->receptionist_pids);
    deallocate_dynamic_memory(data->doctor_pids);

    
}

/* Function that initializes the semaphores in the semaphores structure. Semaphores
 * *_full should be initialized with value 0, semaphores *_empty with value
 * equal to the size of the shared memory buffers, and *_mutex with value 1.
 * For this, you can use the semaphore_create function.
 */
void create_semaphores(struct data_container* data, struct semaphores* sems){

    sems->main_patient->full = semaphore_create(STR_SEM_MAIN_PATIENT_FULL, 0);
    sems->main_patient->empty = semaphore_create(STR_SEM_MAIN_PATIENT_EMPTY, data->buffers_size);
    sems->main_patient->mutex = semaphore_create(STR_SEM_MAIN_PATIENT_MUTEX, 1);

    sems->patient_receptionist->full = semaphore_create(STR_SEM_PATIENT_RECEPT_FULL, 0);
    sems->patient_receptionist->empty = semaphore_create(STR_SEM_PATIENT_RECEPT_EMPTY, data->buffers_size);
    sems->patient_receptionist->mutex = semaphore_create(STR_SEM_PATIENT_RECEPT_MUTEX, 1);

    sems->receptionist_doctor->full = semaphore_create(STR_SEM_RECEPT_DOCTOR_FULL, 0);
    sems->receptionist_doctor->empty = semaphore_create(STR_SEM_RECEPT_DOCTOR_EMPTY, data->buffers_size);
    sems->receptionist_doctor->mutex = semaphore_create(STR_SEM_RECEPT_DOCTOR_MUTEX, 1);

    sems->patient_stats_mutex = semaphore_create(STR_SEM_PATIENT_STATS_MUTEX, 1);
    sems->receptionist_stats_mutex = semaphore_create(STR_SEM_RECEPT_STATS_MUTEX, 1);
    sems->doctor_stats_mutex = semaphore_create(STR_SEM_DOCTOR_STATS_MUTEX, 1);

    sems->results_mutex = semaphore_create(STR_SEM_RESULTS_MUTEX, 1);
    sems->terminate_mutex = semaphore_create(STR_SEM_TERMINATE_MUTEX, 1);

}

/* Function that wakes up all processes sleeping on semaphores, so that
 * they realize that the program termination order has been given. For this,
 * you can use the produce_end function on all sets of semaphores where
 * processes might be sleeping, and a number of times equal to the maximum
 * number of processes that might be there.
 */
void wakeup_processes(struct data_container* data, struct semaphores* sems) {

    int p = data->n_patients;
    int r = data->n_receptionists;
    int d = data->n_doctors;

    for (int i = 0; i < p; i++)
        produce_end(sems->main_patient);

    for(int i = 0; i < p + r; i++)
        produce_end(sems->patient_receptionist);

    for (int i = 0; i < d + r; i++)
       produce_end(sems->receptionist_doctor);  
}

/* Function that releases all semaphores in the semaphores structure. */
void destroy_semaphores(struct semaphores* sems){

    semaphore_destroy(STR_SEM_MAIN_PATIENT_FULL, sems->main_patient->full);
    semaphore_destroy(STR_SEM_MAIN_PATIENT_EMPTY, sems->main_patient->empty);
    semaphore_destroy(STR_SEM_MAIN_PATIENT_MUTEX, sems->main_patient->mutex);

    semaphore_destroy(STR_SEM_PATIENT_RECEPT_FULL, sems->patient_receptionist->full);
    semaphore_destroy(STR_SEM_PATIENT_RECEPT_EMPTY, sems->patient_receptionist->empty);
    semaphore_destroy(STR_SEM_PATIENT_RECEPT_MUTEX, sems->patient_receptionist->mutex);

    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_FULL, sems->receptionist_doctor->full);
    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_EMPTY, sems->receptionist_doctor->empty);
    semaphore_destroy(STR_SEM_RECEPT_DOCTOR_MUTEX, sems->receptionist_doctor->mutex);

    semaphore_destroy(STR_SEM_PATIENT_STATS_MUTEX, sems->patient_stats_mutex);
    semaphore_destroy(STR_SEM_RECEPT_STATS_MUTEX, sems->receptionist_stats_mutex);
    semaphore_destroy(STR_SEM_DOCTOR_STATS_MUTEX, sems->doctor_stats_mutex);

    semaphore_destroy(STR_SEM_RESULTS_MUTEX, sems->results_mutex);
    semaphore_destroy(STR_SEM_TERMINATE_MUTEX, sems->terminate_mutex);
}
