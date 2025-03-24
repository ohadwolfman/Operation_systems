CC = gcc
CFLAGS = -Wall -Wextra

# relevant files
SERVER_SRC = server.c
CLIENT_SRC = client.c
TASK1_SRC = Task1.c
PYTHAGOREAN_SRC = pythagorean.c

# objects files
SERVER_OBJ = server.o
CLIENT_OBJ = client.o
TASK1_OBJ = Task1.o
PYTHAGOREAN_OBJ = pythagorean.o

# Creating executable files

all: server client test

server: $(SERVER_OBJ) $(PYTHAGOREAN_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(PYTHAGOREAN_OBJ) -o server

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o client

test: $(TASK1_OBJ) $(PYTHAGOREAN_OBJ)
	$(CC) $(CFLAGS) $(TASK1_OBJ) $(PYTHAGOREAN_OBJ) -o test_task1



server.o: $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $(SERVER_SRC)

client.o: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $(CLIENT_SRC)

Task1.o: $(TASK1_SRC)
	$(CC) $(CFLAGS) -c $(TASK1_SRC)

pythagorean.o: $(PYTHAGOREAN_SRC)
	$(CC) $(CFLAGS) -c $(PYTHAGOREAN_SRC)

# execute client server
run: server client
	./server & sleep 1; ./client

# execute test
test: test_task1
	./test_task1 3 4 5
	./test_task1 5 12 13
	./test_task1 7 8 9

clean:
	rm -f *.o *.txt server client test_task1
