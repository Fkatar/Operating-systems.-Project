#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "main.h"
#include "memory.h"
#include "stats.h"
#include "hosptime.h"


/*
 * Writes the statistics to the respective file
 */
void write_stats(struct data_container *data) {


    FILE* fp;

    fp = fopen(data->statistics_filename, "w");
    if (fp == NULL) {
        perror("Unable to open the file");
        exit(1);
    }

    // Processes
    fprintf(fp, "Process Statistics:\n    ");
    for (int i = 0; i < data->n_patients; i++) {
        fprintf(fp,"Pacient %d requested %d admissions!\n    ",
                    i, data->patient_stats[i]);
    }
    for (int i = 0; i < data->n_receptionists; i++) {
        fprintf(fp, "Receptionist %d requested %d admissions!\n    ",
                    i, data->receptionist_stats[i]);
    }
    for (int i = 0; i < data->n_doctors; i++) {
        fprintf(fp, "Doctor %d requested %d admissions!\n    ",
                    i, data->doctor_stats[i]);
    }
    
    int i = 0;
    
    // Admissions
    fprintf(fp,"\nAdmission Statistics:\n");
    while (data->results[i].id > -1) {

        struct admission ad = data->results[i];
        char *st = formatted_time(ad.create_time);
        char *pt = formatted_time(ad.patient_time);
        char *rt = formatted_time(ad.receptionist_time);
        char *dt = formatted_time(ad.doctor_time);
        long secs = ad.doctor_time.tv_sec - ad.create_time.tv_sec;
        long msecs = (ad.doctor_time.tv_nsec > ad.create_time.tv_nsec) ? 
                      ad.doctor_time.tv_nsec - ad.create_time.tv_nsec :
                      ad.create_time.tv_nsec - ad.doctor_time.tv_nsec;

        fprintf(fp, "Admission: %d\n"
                    "Status: %c\n"
                    "Patient id: %d\n"
                    "Receptionist id: %d\n"
                    "Doctor id: %d\n"
                    "Start time: %s\n"
                    "Patient time : %s\n"
                    "Receptionist time : %s\n"
                    "Doctor time: %s\n"
                    "Total time: %ld.%03ld\n\n",
                ad.id, ad.status, ad.receiving_patient, ad.receiving_receptionist,
                ad.receiving_doctor, st, pt, rt, dt, secs, msecs/1000000);

        free(st);
        free(pt);
        free(rt);
        free(dt);

        i++;
    }

    
    fclose(fp);
}
