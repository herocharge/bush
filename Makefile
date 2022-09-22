CC = gcc
# CFLAGS = -g -fsanitize=address
CFLAGS = -g

all: bin/main.o bin/string_utils.o bin/echo.o bin/cd.o bin/jobs.o bin/ls.o bin/print_table.o bin/path_utils.o bin/discover.o bin/pinfo.o bin/history.o bin/input.o
	$(CC) $(CFLAGS) bin/main.o bin/string_utils.o bin/echo.o bin/cd.o bin/jobs.o bin/ls.o bin/print_table.o bin/path_utils.o bin/discover.o bin/pinfo.o bin/history.o bin/input.o -o bush

bin/main.o: src/main.c include/string_utils.h bin/string_utils.o
	$(CC) $(CFLAGS) -c src/main.c -o bin/main.o

bin/cd.o: include/cd.h src/cd.c include/string_utils.h bin/string_utils.o
	$(CC) $(CFLAGS) -c src/cd.c -o bin/cd.o

bin/string_utils.o: src/string_utils.c include/string_utils.h
	$(CC) $(CFLAGS) -c src/string_utils.c -o bin/string_utils.o

bin/echo.o: include/echo.h src/echo.c
	$(CC) $(CFLAGS) -c src/echo.c -o bin/echo.o

bin/jobs.o: include/jobs.h src/jobs.c
	$(CC) $(CFLAGS) -c src/jobs.c -o bin/jobs.o

bin/ls.o: include/ls.h src/ls.c
	$(CC) $(CFLAGS) -c src/ls.c -o bin/ls.o

bin/print_table.o: include/print_table.h src/print_table.c
	$(CC) $(CFLAGS) -c src/print_table.c -o bin/print_table.o

bin/path_utils.o: include/path_utils.h src/path_utils.c
	$(CC) $(CFLAGS) -c src/path_utils.c -o bin/path_utils.o

bin/discover.o: include/discover.h src/discover.c
	$(CC) $(CFLAGS) -c src/discover.c -o bin/discover.o

bin/pinfo.o: include/pinfo.h src/pinfo.c
	$(CC) $(CFLAGS) -c src/pinfo.c -o bin/pinfo.o

bin/history.o: include/history.h src/history.c
	$(CC) $(CFLAGS) -c src/history.c -o bin/history.o

bin/input.o: include/input.h src/input.c
	$(CC) $(CFLAGS) -c src/input.c -o bin/input.o

clean:
	rm bin/*.o
	rm bush