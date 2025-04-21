#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "pythagorean.h"

#define PORT 8237
#define MAX_PENDING 10
#define MAX_CLIENTS 10
#define END_SIGNAL 0
#define BUFFER_SIZE 3

typedef struct {
    unsigned char buffer[BUFFER_SIZE];
    int received_count;
} ClientState;

ClientState client_states_Array[MAX_CLIENTS];

int log_fd;
void close_log_file() {
    if (log_fd >= 0) {
        close(log_fd);
        dprintf(log_fd, "Log file closed.\n");
    }
}

int handle_client(int client_socket, ClientState *state) {
    unsigned char side;
    ssize_t bytes_received = recv(client_socket, &side, sizeof(side), 0);


    if (bytes_received > 0) {
        if (side == END_SIGNAL) {
            dprintf(log_fd, "End signal received from client (socket %d).\n", client_socket);
            close(client_socket);
            return END_SIGNAL;
        }
        else {
            state->buffer[state->received_count % BUFFER_SIZE] = side;
            state->received_count++;
            if (state->received_count % BUFFER_SIZE == 0) {
                char response[5];
                if (is_pythagorean_triple(state->buffer[0], state->buffer[1], state->buffer[2])) {
                    strcpy(response, "YES\n");
                } else {
                    strcpy(response, "NO\n");
                }
                dprintf(log_fd, "Received triple from client (socket %d): %d %d %d. The answer: %s\n",
                        client_socket, state->buffer[0], state->buffer[1], state->buffer[2],response);
                if (send(client_socket, response, strlen(response), 0) < 0) {
                    perror("Error sending data to client");
                    return -1;
                }
            }
        }
    }
    else if (bytes_received == 0) {
        dprintf(log_fd, "Client disconnected (socket %d).\n", client_socket);
        return 0;
    } else {
        perror("Error receiving data from client");
        return -1;
    }
    return 1;
}

//struct pollfd { /this struct defined in poll.h. written here for better understanding
//    int   fd;
//    short events; //Events we are interested in
//    short revents; }; //Events that actually happened

int main() {
    // automatic closing log file in the end of execution
    atexit(close_log_file);

    log_fd = open("server_log.txt", O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if (log_fd < 0) {
        perror("Failed to open log file");
        return 1;
    }
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);


    //  Setting the socket
    int server_fd;
    struct sockaddr_in server_addr;
    struct pollfd fds[MAX_CLIENTS + 1];
    int num_clients = 0;

    dprintf(log_fd, "Lets start\n");
    fflush(stdout);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd );
        return 1;
    }

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(server_fd, MAX_PENDING) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    // Initialize pollfd array
    fds[0].fd = server_fd;
    fds[0].events = POLLIN;
    // POLLIN for data available for reading, POLLOUT for ready to write, POLLHUP for disconnecting
    for (int i = 1; i <= MAX_CLIENTS; ++i) {
        fds[i].fd = -1; // Setting all cells in an array as inactive socket
    }

    dprintf(log_fd, "Server listening on port %d using poll()\n", PORT);
    fflush(stdout);

    while (1) {
        int num_ready = poll(fds, num_clients + 1, -1); // Waiting for a file description to be ready
        if (num_ready < 0) {
            perror("poll error");
            exit(EXIT_FAILURE);
        }

        // Checking for a new connection
        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            int new_socket = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
            if (new_socket < 0) {
                perror("accept error");
            } else {
                dprintf(log_fd, "New client connected\n");
                // Adding the new socket to the pollfd arra
                int slot_found = 0;
                for (int i = 1; i <= MAX_CLIENTS; ++i) { //Search the first empty cell in pollfd array
                    if (fds[i].fd == -1) {
                        fds[i].fd = new_socket;
                        fds[i].events = POLLIN;
                        num_clients++;
                        slot_found = 1;
                        break;
                    }
                }
                if (!slot_found) {
                    dprintf(log_fd, "Max clients reached, rejecting more connections\n");
                    close(new_socket);
                }
            }
        }

        // Check if there is data from existing clients
        for (int i = 1; i <= MAX_CLIENTS; ++i) {
            if (fds[i].fd != -1 && (fds[i].revents & POLLIN)) {
                int result = handle_client(fds[i].fd, &client_states_Array[i - 1]);
                if (result == 0 || result == END_SIGNAL || result == -1) { //Detect if a client has disconnected
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    num_clients--;
                }
            }
        }
    }
}