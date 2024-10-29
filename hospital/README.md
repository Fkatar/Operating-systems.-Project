

# hOSpital - Operating Systems Project

## Overview

The **hOSpital** project is designed to simulate a hospital admission system, focusing on process synchronization, memory management, and inter-process communication. This project was completed in the context of the Operating Systems course at Ciências ULisboa. This README covers compiling instructions, usage, and the structure of the project.

## Authors

- Francisco Catarino
- Maria Sargaço
- Pedro Simões

## Table of Contents

- [Compilation Instructions](#compilation-instructions)
- [Project Structure](#project-structure)
- [Usage](#usage)
- [Skills Acquired](#skills-acquired)
- [Project Files](#project-files)

## Compilation Instructions

To compile the project, navigate to the root directory where the `makefile` is located and run:

```bash
make
```

This will compile the necessary files and create an executable in the `bin/` directory named `hOSpital`.

## Usage

Once compiled, run the application using:

```bash
./bin/hOSpital <config_file>
```

where `<config_file>` is the path to a configuration file containing initial arguments such as maximum admissions, buffer sizes, and the number of staff.

After starting, the program provides a menu of actions:

```c
    "[Main] Available actions:\n"
    "[Main] ad patient doctor - create a new admission\n"
    "[Main] status - displays the current state of the variables contained in the data_container\n"
    "[Main] info id - check the status of an admission\n"
    "[Main] help - prints information about the available actions.\n"
    "[Main] end - terminates the execution of hOSpital."
```

### Available Commands

- `ad <patient> <doctor>` - Register a new patient admission.
- `status` - View the current state of the hospital system variables.
- `info <id>` - Get details on a specific admission using its ID.
- `help` - Show a list of available commands.
- `end` - Exit the program and release all resources.

## Skills Acquired

During the completion of the hOSpital project, various essential technical and problem-solving skills were developed:

1. **Process Synchronization and Concurrency**: Learned to manage concurrent access to shared resources using semaphores and mutexes, ensuring thread-safe operations and data integrity.

2. **Memory Management**: Gained practical experience in dynamic memory allocation, buffer management, and shared memory handling, improving the project's efficiency and scalability.

3. **File I/O and Logging**: Implemented file operations for configuration loading, logging actions, and generating statistics, which included handling input/output streams, formatting, and error handling.

4. **Inter-process Communication (IPC)**: Utilized IPC mechanisms to enable communication between separate processes in a controlled manner, which is crucial for maintaining the project's integrity.

5. **Signal Handling**: Developed robust signal handling techniques to intercept and process system signals (e.g., SIGINT for program termination), ensuring smooth cleanup and resource release.

6. **Timing and Alarms**: Integrated timing functions to log events and trigger alarms at set intervals, enhancing functionality and enabling automated updates in real-time.

7. **Project Structure and Build Automation**: Learned to organize code into modular components (header and source files) and utilized `makefile` for efficient build automation, promoting good software engineering practices.

## Project Structure

```plaintext
hospital
├── README.md
├── bin
│   ├── hOSpital
│   └── hOSpital_profs
├── hosp_files
│   ├── entrada.txt
│   ├── log.txt
│   └── stats.txt
├── include
│   ├── close_sems-private.h
│   ├── configuration.h
│   ├── doctor.h
│   ├── hospsignal.h
│   ├── hosptime.h
│   ├── log.h
│   ├── main.h
│   ├── memory.h
│   ├── patient.h
│   ├── process.h
│   ├── receptionist.h
│   ├── stats.h
│   └── synchronization.h
├── makefile
├── obj
│   ├── *.o
└── src
    ├── close_sems-private.c
    ├── configuration.c
    ├── doctor.c
    ├── hospsignal.c
    ├── hosptime.c
    ├── log.c
    ├── main.c
    ├── memory.c
    ├── patient.c
    ├── process.c
    ├── receptionist.c
    ├── stats.c
    └── synchronization.c
```

### Important Files

- **`makefile`**: Build instructions to compile the project.
- **`bin/`**: Contains the compiled executable files.
- **`hosp_files/`**: Holds input/output files like logs and statistics.
- **`include/`**: Header files with definitions and declarations.
- **`obj/`**: Contains compiled object files.
- **`src/`**: Source code files for the project's logic.

