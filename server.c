#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "pythagorean.h"

#define PORT 8237
#define MAX_PENDING 10
#define END_SIGNAL 0

int handle_client(int client_socket) {
    unsigned char buffer[3] = {0};
    int received_count = 0;
    int client_closed = 0;

    while (!client_closed) {
        unsigned char side;
        int bytes_received = recv(client_socket, &side, sizeof(side), 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("Error receiving data");
            }
            close(client_socket);
            return -2;  // error code
        }

        if (side == END_SIGNAL) {
            printf("End signal received from client.\n");
            close(client_socket);
            return END_SIGNAL;
        }

        buffer[received_count % 3] = side;
        received_count++;

        if (received_count % 3 == 0) {
            printf("Received triple: %d %d %d\n", buffer[0], buffer[1], buffer[2]);
            if (is_pythagorean_triple(buffer[0], buffer[1], buffer[2])) {
                if (send(client_socket, "YES\n", 4, 0) < 0) {
                    perror("Error sending data");
                    close(client_socket);
                    return -3;
                }
            } else {
                if (send(client_socket, "NO\n", 3, 0) < 0) {
                    perror("Error sending data");
                    close(client_socket);
                    return -3;
                }
            }
        }
    }
    return 0;
}

int main() {
    printf("Lets start\n");
    fflush(stdout);
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_socket);
        return 1;
    }

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(server_socket, MAX_PENDING) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    printf("Server listening on port %d\n", PORT);
    fflush(stdout);

    int server_closed = 0;
    while (!server_closed) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        printf("Client connected.\n");
        fflush(stdout);
        int result = handle_client(client_socket);
        if (result == END_SIGNAL) {
            printf("Client ended session using END_SIGNAL.\n");
            server_closed=1;
        } else if (result < 0) {
            printf("Client session ended with error code: %d\n", result);
        }
    }
    close(server_socket);
    return 0;
}