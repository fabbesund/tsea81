all: set_clock

clock: main.c si_comm.c si_ui.c si_ui.h si_comm.h clock.c clock.h display.h display.c
	gcc main.c display.c si_comm.c si_ui.c clock.c -g  -o clock -Wall -lpthread

clean:
	rm -f clock *~ *.o
