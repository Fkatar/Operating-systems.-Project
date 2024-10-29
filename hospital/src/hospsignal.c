/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "main.h"
#include "hospsignal.h"

struct data_container* main_data;
struct communication* communication_buffers;
struct semaphores* semaphores;
int used_flag = 0;

/*
 * Formatted print of the admission
 */
void print_admissao(struct admission* ad) {
    printf("add:%d status:%c start_time: %ld ",ad->id ,ad->status ,ad->create_time.tv_sec);
}

/*
 * Formatted print of the patient
 */
void print_paciente(struct admission* ad) {
    printf("patient:%d patient_time: %ld ", ad->receiving_patient, ad->patient_time.tv_sec);
}

/*
 * Formatted print of the receptionist
 */
void print_receptionist(struct admission* ad) {
    printf("receptionist:%d receptionist_time: %ld ", ad->receiving_receptionist, ad->receptionist_time.tv_sec);
}

/*
 * Formatted print of the doctor
 */
void print_doctor(struct admission* ad) {
    printf("doctor:%d doctor_time:%ld ", ad->receiving_doctor, ad->doctor_time.tv_sec);
}

/*
 * Formatted print of an admission according to its status
 */
void escreveAdmissoes(struct data_container *data) {
    for(int i = 0; data->results[i].id != -1 && i < data->max_ads; i++){
        struct admission ad = data->results[i];
        switch (ad.status){
            case 'M':
                print_admissao(&ad);
                break;

            case 'P':
                print_admissao(&ad);
                print_paciente(&ad);
                break;

            case 'R':
                print_admissao(&ad);
                print_paciente(&ad);
                print_receptionist(&ad);
                break;

            case 'A':
                print_admissao(&ad);
                print_paciente(&ad);
                print_receptionist(&ad);
                print_doctor(&ad);
                break;
                
            case 'N':
                print_admissao(&ad);
                print_paciente(&ad);
                print_receptionist(&ad);
                print_doctor(&ad);
                break;

            default:
                break;
            }

        printf("\n");
    }
}

/*
 * Terminates the program
 */
void ctrlC() {
    end_execution(main_data, communication_buffers, semaphores);
    used_flag = 1;
    main_data->terminate = 1;
}

/*
 * Handles the program termination signal
 */
int signal_handle(struct data_container *data, struct communication *comm, struct semaphores *sems) {
    main_data = data;
    communication_buffers = comm;
    semaphores = sems;

    struct sigaction sa;
    sa.sa_handler = ctrlC;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1){
        perror("main:");
        exit(-1);
    }
    return used_flag;
}
