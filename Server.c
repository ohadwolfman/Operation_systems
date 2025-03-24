#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "pythagorean.h"

#define PORT 8080

void handle_client(int client_socket) {
    unsigned char buffer[3] = {0}; // Buffer for the 3 elements
    int received_count = 0;

    while (1) {
        unsigned char side;
        int bytes_received = recv(client_socket, &side, sizeof(side), 0);
        if (bytes_received <= 0) break;

        buffer[received_count % 3] = side;
        received_count++;

        if (received_count < 3) {
            send(client_socket, "Not enough samples\n", 19, 0);
        } else {
            if (is_pythagorean_triple(buffer[0], buffer[1], buffer[2])) {
                send(client_socket, "YES\n", 4, 0);
            } else {
                send(client_socket, "NO\n", 3, 0);
            }
        }
    }
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) >= 0) {
        handle_client(new_socket);
    }

    return 0;
}
