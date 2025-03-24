#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    unsigned int seed = (unsigned int) atoi(argv[1]);
    srand(seed); // Set the random seed for client

    // Creating the socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Error creating socket");
        return 1;
    }

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        unsigned char side = (unsigned char)(rand() % 15);
        if (send(client_socket, &side, sizeof(side), 0) < 0) {
            perror("Error sending data");
            return 1;
        }
        printf("Sent: %d\n", side);
        sleep(1);
    }

    char response[256];
    int bytes_received;
    while ((bytes_received = recv(client_socket, response, sizeof(response)-1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s", response);
    }

    close(client_socket);
    return 0;
}
