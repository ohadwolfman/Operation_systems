CC = gcc
CFLAGS = -Wall -Wextra

combined_program: main.o
	$(CC) $(CFLAGS) main.o -o combined_program

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

Task1.o: Task1.c
	$(CC) $(CFLAGS) -c Task1.c

clean:
	rm -f *.o output.txt combined_program
