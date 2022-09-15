CC = gcc
# CFLAGS = -g -fsanitize=address
CFLAGS = -g

all: main.o string_utils.o echo.o cd.o jobs.o ls.o print_table.o path_utils.o discover.o pinfo.o history.o input.o
	$(CC) $(CFLAGS) main.o string_utils.o echo.o cd.o jobs.o ls.o print_table.o path_utils.o discover.o pinfo.o history.o input.o -o bush

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

ls.o: ls.h ls.c
	$(CC) $(CFLAGS) -c ls.c

print_table.o: print_table.h print_table.c
	$(CC) $(CFLAGS) -c print_table.c

path_utils.o: path_utils.h path_utils.c
	$(CC) $(CFLAGS) -c path_utils.c

discover.o: discover.h discover.c
	$(CC) $(CFLAGS) -c discover.c

pinfo.o: pinfo.h pinfo.c
	$(CC) $(CFLAGS) -c pinfo.c

history.o: history.h history.c
	$(CC) $(CFLAGS) -c history.c

input.o: input.h input.c
	$(CC) $(CFLAGS) -c input.c

clean:
	rm *.o
	rm bush