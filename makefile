CC = gcc
CFLAGS = -Wall -Wextra -pthread

# relevant files
SERVER_SRC = server.c
CLIENT_SRC = client.c
SINGLE_CLIENT_SRC = single_client.c
PYTHAGOREAN_SRC = pythagorean.c

# object files
SERVER_OBJ = server.o
CLIENT_OBJ = client.o
SINGLE_CLIENT_OBJ = single_client.o
PYTHAGOREAN_OBJ = pythagorean.o

# Creating executable files
# Creating executable files
all: server client single_client test

server: $(SERVER_OBJ) $(PYTHAGOREAN_OBJ)
	$(CC) $(CFLAGS) $(SERVER_OBJ) $(PYTHAGOREAN_OBJ) -o server

client: $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(CLIENT_OBJ) -o client

single_client: $(SINGLE_CLIENT_OBJ)
	$(CC) $(CFLAGS) $(SINGLE_CLIENT_OBJ) -o single_client

test: test_task1

test_task1: $(SINGLE_CLIENT_OBJ) $(PYTHAGOREAN_OBJ)
	$(CC) $(CFLAGS) $(SINGLE_CLIENT_OBJ) $(PYTHAGOREAN_OBJ) -o test_task1

# Compile source files
server.o: $(SERVER_SRC)
	$(CC) $(CFLAGS) -c $(SERVER_SRC)

client.o: $(CLIENT_SRC)
	$(CC) $(CFLAGS) -c $(CLIENT_SRC)

single_client.o: $(SINGLE_CLIENT_SRC)
	$(CC) $(CFLAGS) -c $(SINGLE_CLIENT_SRC)

pythagorean.o: $(PYTHAGOREAN_SRC)
	$(CC) $(CFLAGS) -c $(PYTHAGOREAN_SRC)

# execute client with multi-threading
run: server client
	pkill server || true
	./server & sleep 1; ./client $(SEED)

# execute old single client version
run_single: server single_client
	pkill server || true
	./server & sleep 1; ./single_client $(SEED)

# execute test program
test_run: test_task1
	./test_task1 3 4 5
	./test_task1 5 12 13
	./test_task1 7 8 9

clean:
	rm -f *.o *.txt server client single_client test_task1