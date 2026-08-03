CC = cc
CFLAGS = -fsanitize=thread -g

NAME = codexion


FILES = main.c parser.c simulation_routine.c \
		create_resources.c simulation_utiles.c codexion_utiles.c


all:
	$(CC)  $(FILES) -pthread
	./a.out  3 317 100 10 100 2 5 edf 

helgrand:
	$(CC) -g $(FILES) -pthread
	setarch $(uname -m) -R valgrind --tool=helgrind ./a.out 30 9150 90 60 60 5 600 fifo

clean:
	rm a.out *.o
