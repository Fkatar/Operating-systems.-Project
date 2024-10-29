/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <stdlib.h> 
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "memory.h"

/* Function that reserves a shared memory area with the size indicated
* by size and name name, fills this memory area with the value 0, and 
* returns a pointer to it. You can concatenate the result of the function
* getuid() to name, to make the name unique for the process.
*/
void* create_shared_memory(char* name, int size) {

    void* ptr;
    int ret;

    int fd = shm_open(name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror(name);
        exit(1);
    }
    ret = ftruncate(fd, size);
    if (ret == -1) {
        perror(name);
        exit(2);
    }
    ptr = mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("shm-mmap");
        exit(3);
    }

    memset(ptr, 0, size);

    return ptr;

}

/* Function that reserves a dynamic memory area with the size indicated
* by size, fills this memory area with the value 0, and returns a 
* pointer to it.
*/
void* allocate_dynamic_memory(int size){
    //check if I can do it in a return 
    //but this will reserve a dynamic memory area
    return malloc(size); 
}

/* Function that frees a previously reserved shared memory area.
*/
void destroy_shared_memory(char* name, void* ptr, int size) {

    int ret;

    ret = munmap(ptr, size);
    if (ret == -1){
        perror(name);
        exit(1);
    }

    ret = shm_unlink(name);

    if (ret == -1){
        perror(name);
        exit(2);
    }
}

/* Function that frees a previously reserved dynamic memory area.
*/
void deallocate_dynamic_memory(void* ptr){
    free(ptr);
}

void write_circular(struct circular_buffer* buffer, int buffer_size, struct admission* ad){
    //if in == out the array is full of admissions
    //TODO -> see if it's better to create an in variable to not always access the same
    if(((buffer->ptrs->in + 1) % buffer_size) == buffer->ptrs->out) {
        return;
    }

    //printf("id:%d rp:%d rd:%d s:%c p:%d r:%d d:%d\n", ad->id, ad->requesting_patient, ad->requested_doctor, ad->status,
    //ad->receiving_patient, ad->receiving_receptionist, ad->receiving_doctor);
    //fflush(stdout);

    //if there is still space then we put the admission there 

    //buffer->buffer[buffer->ptrs->in] = *ad;

    buffer->buffer[buffer->ptrs->in].id = ad->id;
    buffer->buffer[buffer->ptrs->in].requesting_patient = ad->requesting_patient;
    buffer->buffer[buffer->ptrs->in].requested_doctor = ad->requested_doctor;
    buffer->buffer[buffer->ptrs->in].status = ad->status;
    buffer->buffer[buffer->ptrs->in].receiving_patient = ad->receiving_patient;
    buffer->buffer[buffer->ptrs->in].receiving_receptionist = ad->receiving_receptionist;
    buffer->buffer[buffer->ptrs->in].create_time = ad->create_time;
    buffer->buffer[buffer->ptrs->in].patient_time = ad->patient_time;
    buffer->buffer[buffer->ptrs->in].receptionist_time = ad->receptionist_time;
    buffer->buffer[buffer->ptrs->in].doctor_time = ad->doctor_time;

    //then we update the input pointer value
    buffer->ptrs->in = (buffer->ptrs->in + 1) % buffer_size;

    //printf("id:%d rp:%d rd:%d s:%c p:%d r:%d d:%d\n", ad->id, ad->requesting_patient, ad->requested_doctor, ad->status,
        //ad->receiving_patient, ad->receiving_receptionist, ad->receiving_doctor);
    //fflush(stdout);
}

/* Function that writes an admission in the shared memory buffer between the Main
* and the patients. The admission must be written in a free position of the buffer, 
* taking into account the type of buffer and the writing rules in such buffers.
* If there is no free position, it writes nothing.
*/
void write_main_patient_buffer(struct circular_buffer* buffer, int buffer_size, struct admission* ad) {
    write_circular(buffer, buffer_size, ad);
}

/* Function that writes an admission in the shared memory buffer between the patients
* and the receptionists. The admission must be written in a free position of the buffer, 
* taking into account the type of buffer and the writing rules in such buffers.
* If there is no free position, it writes nothing.
*/
void write_patient_receptionist_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct admission* ad) {

    for(int i = 0; i < buffer_size; i++){
    
        if (buffer->ptrs[i] == 0) {


            //printf("id: %d  rp: %d  rd:%d s:%c p:%d", ad->id, ad->requesting_patient, ad->requested_doctor, ad->status, ad->receiving_patient);
            //fflush(stdout);

            buffer->buffer[i].id = ad->id;
            buffer->buffer[i].requesting_patient = ad->requesting_patient;
            buffer->buffer[i].requested_doctor = ad->requested_doctor;
            buffer->buffer[i].status = ad->status;
            buffer->buffer[i].receiving_patient = ad->receiving_patient;
            buffer->buffer[i].create_time = ad->create_time;
            buffer->buffer[i].patient_time = ad->patient_time;

            //printf("after writing in the buffer");
            //fflush(stdout);
            buffer->ptrs[i] = 1;
            return;
        }
    }
    
}

/* Function that writes an admission in the shared memory buffer between the receptionists
* and the doctors. The admission must be written in a free position of the buffer, 
* taking into account the type of buffer and the writing rules in such buffers.
* If there is no free position, it writes nothing.
*/
void write_receptionist_doctor_buffer(struct circular_buffer* buffer, int buffer_size, struct admission* ad){
    write_circular(buffer, buffer_size, ad);
}

/* Function that reads an admission from the shared memory buffer between the Main
* and the patients, if there is any available to read that is directed to the specified patient.
* The reading must be done taking into account the type of buffer and the reading rules in such buffers.
* If there is no admission available, it sets ad->id to -1.
*/
void read_main_patient_buffer(struct circular_buffer* buffer, int patient_id, int buffer_size, struct admission* ad){

    if (patient_id == buffer->buffer[buffer->ptrs->out].requesting_patient &&
        buffer->ptrs->in != buffer->ptrs->out) {

        //printf("Read from buffer patient_id: %d", patient_id);
        //fflush(stdout);

        struct admission new_ad = buffer->buffer[buffer->ptrs->out];
        
        ad->id = new_ad.id;
        ad->requesting_patient = new_ad.requesting_patient;
        ad->requested_doctor = new_ad.requested_doctor;
        ad->create_time = new_ad.create_time;

        //printf("id: %d  rp: %d  rd:%d", ad->id, ad->requesting_patient, ad->requested_doctor);
        buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
        return;
    }

    ad->id = -1;
    
}

/* Function that reads an admission from the shared memory buffer between the patients and receptionists,
* if there is any available to read (any receptionist can read any admission).
* The reading must be done taking into account the type of buffer and the reading rules in such buffers.
* If there is no admission available, it sets ad->id to -1.
*/
void read_patient_receptionist_buffer(struct rnd_access_buffer* buffer, int buffer_size, struct admission* ad) {

    for (int k = 0; k < buffer_size; k++) {

        if (buffer->ptrs[k] == 1) {

            ad->id = buffer->buffer[k].id;
            ad->requesting_patient = buffer->buffer[k].requesting_patient;
            ad->requested_doctor = buffer->buffer[k].requested_doctor;
            ad->status = buffer->buffer[k].status;
            ad->receiving_patient = buffer->buffer[k].receiving_patient;
            ad->create_time = buffer->buffer[k].create_time;
            ad->patient_time = buffer->buffer[k].patient_time;

            buffer->ptrs[k] = 0;
            return;
        }
    }

}

/* Function that reads an admission from the shared memory buffer between the receptionists and the doctors,
* if there is any available to read directed to the specified doctor. The reading must
* be done taking into account the type of buffer and the reading rules in such buffers. If there is no
* admission available, it sets ad->id to -1.
*/
void read_receptionist_doctor_buffer(struct circular_buffer* buffer, int doctor_id, int buffer_size, struct admission* ad) {

    if (doctor_id == buffer->buffer[buffer->ptrs->out].requested_doctor &&
        buffer->ptrs->in != buffer->ptrs->out)
    {


        struct admission new_ad = buffer->buffer[buffer->ptrs->out];

        ad->id = new_ad.id;
        ad->requesting_patient = new_ad.requesting_patient;
        ad->requested_doctor = new_ad.requested_doctor;
        ad->status = new_ad.status;
        ad->receiving_patient = new_ad.receiving_patient;
        ad->receiving_receptionist = new_ad.receiving_receptionist;
        ad->receiving_doctor= new_ad.receiving_doctor;
        ad->create_time = new_ad.create_time;
        ad->patient_time = new_ad.patient_time;
        ad->receptionist_time = new_ad.receptionist_time;

        //printf("id: %d  rp: %d  rd:%d", ad->id, ad->requesting_patient, ad->requested_doctor);
        buffer->ptrs->out = (buffer->ptrs->out + 1) % buffer_size;
        return;
    }

    
    ad->id = -1;
}
