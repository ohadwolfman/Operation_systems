#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include "thread_pool.h"
#include <errno.h>

#include "pythagorean.h"

#define PORT 8237
#define MAX_PENDING 10
#define MAX_CLIENTS 10
#define END_SIGNAL 0
#define THREAD_POOL_SIZE 10

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} client_info_t;

pthread_mutex_t log_mutex; // for locking while writing log file

void safe_log(const char *format, ...) {
    pthread_mutex_lock(&log_mutex);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}

int log_fd;
void close_log_file() {
    pthread_mutex_lock(&log_mutex);
    dprintf(3, "Server stopped receiving requests. Closing log.");
    fsync(3);
    pthread_mutex_unlock(&log_mutex);
    if (log_fd >= 0) {
        close(log_fd);
    }
}

void handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg); // Freeing memory in passed args

    while (1) {
        unsigned char sides[3];
        int received = 0;

        // receive 3 numbers from the client
        while (received < 3) {
            ssize_t bytes_received = recv(client_socket, &sides[received], 3 - received, 0);

            if (bytes_received < 0) {
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    safe_log("Timeout while receiving data from client (socket %d).\n", client_socket);
                } else {
                    perror("recv error");
                    safe_log("recv error from client (socket %d).\n", client_socket);
                }
                close(client_socket);
                pthread_exit(NULL);
            }

            if (bytes_received == 0) {
                safe_log("Client disconnected abruptly (socket %d).\n", client_socket);
                close(client_socket);
                pthread_exit(NULL);
            }

            // check if END_SIGNAL arrived
            for (int i = 0; i < bytes_received; ++i) {
                if (sides[received + i] == END_SIGNAL) {
                    safe_log("End signal received from client (socket %d).\n", client_socket);
                    close(client_socket);
                    pthread_exit(NULL);
                }
            }

            received += bytes_received;
        }

        // reached here only if 3 full bytes received and none of them were END_SIGNAL
        char response[5];
        if (is_pythagorean_triple(sides[0], sides[1], sides[2])) {
            strcpy(response, "YES\n");
        } else {
            strcpy(response, "NO\n");
        }

        // print to log
        safe_log("Received triple from client (socket %d): %d %d %d. The answer: %s",
                 client_socket, sides[0], sides[1], sides[2], response);

        // send the answer
        if (send(client_socket, response, strlen(response), 0) < 0) {
            perror("send error");
            safe_log("Error sending data to client (socket %d).\n", client_socket);
            close(client_socket);
            pthread_exit(NULL);
        }
    }
}

int main() {
    // automatic closing log file in the end of execution
    atexit(close_log_file);

    log_fd = open("server.log", O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if (log_fd < 0) {
        pthread_mutex_lock(&log_mutex);
        perror("Failed to open log file");
        pthread_mutex_unlock(&log_mutex);
        return 1;
    }

    // redirect stdout and stderr to log file
    dup2(log_fd, STDOUT_FILENO);
    dup2(log_fd, STDERR_FILENO);

    safe_log("Let's start\n");

    // Set up server socket
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        pthread_mutex_lock(&log_mutex);
        perror("Error creating socket");
        pthread_mutex_unlock(&log_mutex);
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        pthread_mutex_lock(&log_mutex);
        perror("setsockopt");
        pthread_mutex_unlock(&log_mutex);
        close(server_fd);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        pthread_mutex_lock(&log_mutex);
        perror("Error binding socket");
        pthread_mutex_unlock(&log_mutex);
        return 1;
    }

    if (listen(server_fd, MAX_PENDING) < 0) {
        pthread_mutex_lock(&log_mutex);
        perror("Error listening on socket");
        pthread_mutex_unlock(&log_mutex);
        return 1;
    }

    safe_log("Server listening on port %d with threads.\n", PORT);

    // initialize mutex
    if (pthread_mutex_init(&log_mutex, NULL) != 0) {
        pthread_mutex_lock(&log_mutex);
        perror("Mutex init failed");
        pthread_mutex_unlock(&log_mutex);
        return 1;
    }

    ThreadPool *pool = thread_pool_create(THREAD_POOL_SIZE);
    if (!pool) {
        perror("Failed to create thread pool");
        exit(EXIT_FAILURE);
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *) &client_addr, &client_len);
        if (new_socket < 0) {
            pthread_mutex_lock(&log_mutex);
            perror("accept error");
            pthread_mutex_unlock(&log_mutex);
            continue;
        }

        // Set timeout of 5 seconds for recv()
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if (setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            safe_log("Failed to set timeout on client socket (socket %d).\n", new_socket);
            close(new_socket);
            continue;
        }

        safe_log("New client connected (socket: %d)\n", new_socket);

        int *client_socket_ptr = malloc(sizeof(int));
        if (client_socket_ptr == NULL) {
            pthread_mutex_lock(&log_mutex);
            perror("malloc");
            pthread_mutex_unlock(&log_mutex);
            close(new_socket);
            continue;
        }
        *client_socket_ptr = new_socket;

        thread_pool_add_task(pool, handle_client, client_socket_ptr);
    }
    pthread_mutex_destroy(&log_mutex);
    close(server_fd);
    return 0;
}
