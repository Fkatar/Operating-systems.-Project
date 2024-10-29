/**
Group 33
fc59790 - Francisco Catarino
fc59803 - Maria Sargaço
fc59822 - Pedro Simões
*/

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#include "memory.h"
#include "main.h"
#include "process.h"
#include "doctor.h"
#include "patient.h"
#include "receptionist.h"

/* Function that starts a new Patient process using the fork function of the OS. The new
* process will execute the respective execute_patient function, exiting with the return value.
* The parent process returns the pid of the created process.
*/
int launch_patient(int patient_id, struct data_container* data, struct communication* comm, struct semaphores* sems)  {

    int pid;

    //error
    if ((pid = fork()) == -1) exit(1);

    //child process
    if (pid == 0) {
        exit(execute_patient(patient_id, data, comm, sems));
    }
    
    //parent process
    else return pid;

}

/* Function that starts a new Receptionist process using the fork function of the OS. The new
* process will execute the execute_receptionist function, exiting with the return value.
* The parent process returns the pid of the created process.
*/
int launch_receptionist(int receptionist_id, struct data_container* data, struct communication* comm, struct semaphores* sems)  {

    int pid;

    //error
    if ((pid = fork()) == -1) exit(1);

    //child process
    if (pid == 0) exit(execute_receptionist(receptionist_id, data, comm, sems));
    
    //parent process
    else return pid;
}

/* Function that starts a new Doctor process using the fork function of the OS. The new
* process will execute the execute_doctor function, exiting with the return value.
* The parent process returns the pid of the created process.
*/
int launch_doctor(int doctor_id, struct data_container* data, struct communication* comm, struct semaphores* sems) {

    int pid;

    //error
    if ((pid = fork()) == -1) exit(1);

    //child process
    if (pid == 0) exit(execute_doctor(doctor_id, data, comm, sems));
    
    //parent process
    else return pid;
}

/* Function that waits for a process to terminate using the waitpid function. 
* Returns the return value of the process, if it has terminated normally.
*/
int wait_process(int process_id) {

    int* status = 0;

    return waitpid(process_id, status, 0);
}