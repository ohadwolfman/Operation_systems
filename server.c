#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>

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

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_info_t;

int log_fd;
void close_log_file() {
    dprintf(3, "Server stopped receiving requests. Closing log.");
    fsync(3);
    if (log_fd >= 0) {
        close(log_fd);
    }
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg);

    while (1) {
        unsigned char sides[3];
        int received = 0;

        // receive 3 numbers from the client
        while (received < 3) {
            ssize_t bytes_received = recv(client_socket, &sides[received], 3 - received, 0);

            if (bytes_received < 0) {
                perror("Error receiving data from client\n");
                close(client_socket);
                pthread_exit(NULL);

            } else if (bytes_received == 0) {    // client disconnected
                printf("Client disconnected (socket %d).\n", client_socket);
                fflush(stdout);
                close(client_socket);
                pthread_exit(NULL);
            }
            else{ // bytes_received > 0
                char response[5];
                if (sides[received] == END_SIGNAL) {
                    printf("End signal received from client (socket %d).\n", client_socket);
                    fflush(stdout);
                    close(client_socket);
                    pthread_exit(NULL);
                }
                else{
                    // checking if pythagorean triple
                    if (is_pythagorean_triple(sides[0], sides[1], sides[2])) {
                        strcpy(response, "YES\n");
                    } else {
                        strcpy(response, "NO\n");
                    }

                    received += bytes_received;
                }
                if(received==3){
                    // printing to log
                    printf("Received triple from client (socket %d): %d %d %d. The answer: %s",
                           client_socket, sides[0], sides[1], sides[2], response);
                    fflush(stdout);

                    // send the answer to the client
                    if (send(client_socket, response, strlen(response), 0) < 0) {
                        perror("Error sending data to client");
                        close(client_socket);
                        pthread_exit(NULL);
                    }
                }
            }
        }
    }
}

int main() {
    // automatic closing log file in the end of execution
    atexit(close_log_file);

    log_fd = open("server.log", O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if (log_fd < 0) {
        perror("Failed to open log file");
        return 1;
    }

    // redirect stdout and stderr to log file
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);

    printf("Let's start\n");
    fflush(stdout);

    // Set up server socket
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error creating socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(server_fd, MAX_PENDING) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    printf("Server listening on port %d with threads.\n", PORT);
    fflush(stdout);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        if (new_socket < 0) {
            perror("accept error");
            continue;
        }

        printf("New client connected (socket: %d)\n", new_socket);
        fflush(stdout);

        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            perror("malloc");
            close(new_socket);
            continue;
        }
        *client_socket_ptr = new_socket;

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, client_socket_ptr) != 0) {
            perror("pthread_create");
            free(client_socket_ptr);
            close(new_socket);
        } else {
            pthread_detach(thread);  // So that we don't have to call pthread_join
        }
    }

    close(server_fd);
    return 0;
}