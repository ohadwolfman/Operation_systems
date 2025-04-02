#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "pythagorean.h"

#define PORT 8237
#define MAX_PENDING 10
#define END_SIGNAL -1

void handle_client(int client_socket) {
    unsigned char buffer[3] = {0};
    int received_count = 0;
    int client_closed = 0;

    while (!client_closed) {
        int side; // Change to int to receive END_SIGNAL
        int bytes_received = recv(client_socket, &side, sizeof(side), 0);

        printf("Received: %d (bytes received: %d)\n", side, bytes_received);
        fflush(stdout);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("Error receiving data");
            }
            close(client_socket);
            client_closed = 1;
            break;
        }

        if (side == END_SIGNAL) {
            printf("End signal received from client.\n");
            close(client_socket);
            client_closed = 1;
            break;
        }

        buffer[received_count % 3] = (unsigned char)side;
        received_count++;

        if (received_count % 3 == 0) {
            if (is_pythagorean_triple(buffer[0], buffer[1], buffer[2])) {
                if (send(client_socket, "YES\n", 4, 0) < 0) {
                    perror("Error sending data");
                    close(client_socket);
                    client_closed = 1;
                    break;
                }
            } else {
                if (send(client_socket, "NO\n", 3, 0) < 0) {
                    perror("Error sending data");
                    close(client_socket);
                    client_closed = 1;
                    break;
                }
            }
        }
    }
}

int main() {
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

    printf("Server listening on port %d...\n", PORT);
    fflush(stdout);

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Error accepting connection");
            continue;
        }

        printf("Client connected.\n");
        fflush(stdout);
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}