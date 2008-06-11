# Macro definitions
PROJ = udp2tcp  # projectname
LIBOBJ = library.o logging.o # library object files
OBJ = main.o udp2tcp.o $(LIBOBJ) # all the object files that make up the project

## executables ##
CC = gcc		# compiler
RM = rm -f 	# deletion command

DEBUGFLAGS = -DDEBUG # enables debug messages

#### cross-platform macros ####
CFLAGS = -O2 -pipe -Wall -Werror $(DEBUGFLAGS) # compiler switches
 
## Different project targets ##
linux : $(OBJ) # default target
	$(CC) $(CFLAGS) $(OBJ) -o $(PROJ)

## Generic targets ##
clean	: 
	${RM} $(PROJ) *.o *~

