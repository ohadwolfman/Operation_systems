#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "pythagorean.h"

#define PORT 9090
#define MAX_PENDING 10
#define TIMEOUT_SEC 10

int sample_count = 0;

void handle_client(int client_socket) {
    unsigned char buffer[3] = {0}; // Buffer for the 3 elements
    int received_count = 0;

    while (1) {
        unsigned char side;
        int bytes_received = recv(client_socket, &side, sizeof(side), 0);

        if (bytes_received <= 0){
            perror("Error receiving data");
            close(client_socket);
            break;
        }

        buffer[received_count % 3] = side;
        received_count++;

        // sending answer after 3 numbers
        if(received_count % 3 == 0){
            if (is_pythagorean_triple(buffer[0], buffer[1], buffer[2])) {
                send(client_socket, "YES\n", 4, 0);
                received_count=0;
            } else {
                send(client_socket, "NO\n", 3, 0);
                received_count=0;
            }
        }
    }
    close(client_socket); // ?
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

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error binding socket");
        return 1;
    }

    if (listen(server_socket, MAX_PENDING) < 0) {
        perror("Error listening on socket");
        return 1;
    }

    printf("Server listening...\n");
    fflush(stdout);

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Error accepting connection\n");
            continue;
        }

        printf("Client connected\n");
        fflush(stdout);
        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
