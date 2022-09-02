CC = gcc
CFLAGS = -Wall -g

all: main.o string_utils.o echo.o cd.o jobs.o
	$(CC) $(CFLAGS) main.o string_utils.o echo.o cd.o jobs.o -o bush

main.o: main.c string_utils.h string_utils.o
	$(CC) $(CFLAGS) -c main.c

cd.o: cd.h cd.c string_utils.h string_utils.o
	$(CC) $(CFLAGS) -c cd.c

string_utils.o: string_utils.c string_utils.h
	$(CC) $(CFLAGS) -c string_utils.c

echo.o: echo.h echo.c
	$(CC) $(CFLAGS) -c echo.c

jobs.o: jobs.h jobs.c
	$(CC) $(CFLAGS) -c jobs.c


clean:
	rm *.o
	rm bush