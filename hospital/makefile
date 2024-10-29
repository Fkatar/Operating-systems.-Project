#
#Grupo 33
#fc59790 - Francisco Catarino
#fc59803 - Maria Sargaço
#fc59822 - Pedro Simões
#

OBJECTOS = main.o process.o memory.o patient.o doctor.o receptionist.o \
synchronization.o configuration.o hospsignal.o hosptime.o stats.o log.o \
close_sems-private.o

CC = gcc

OBJ_DIR = obj
INC_DIR = include
FLAGS = -Wall -Wextra -I $(INC_DIR)

# executavel final
hOSpital: $(OBJECTOS)
	$(CC) $(addprefix $(OBJ_DIR)/,$(OBJECTOS)) -o bin/hOSpital

%.o: src/%.c $($@)
	$(CC) $(FLAGS) -o $(OBJ_DIR)/$@ -c $< 

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f bin/hOSpital