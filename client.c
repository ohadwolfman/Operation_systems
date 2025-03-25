#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        fflush(stdout);
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
        close(client_socket);
        return 1;
    }

    for (int i = 0; i < 6; i++) {
        unsigned char side = (unsigned char)(rand() % 15);
        if (send(client_socket, &side, sizeof(side), 0) < 0) {
            perror("Error sending data");
            close(client_socket);
            return 1;
        }
        sleep(1);
        printf("Sent: %d\n", side);
//        fflush(stdout);
    }

    // Shut down sending side
    if (shutdown(client_socket, SHUT_WR) < 0) {
        perror("Error shutting down write direction");
    }

    char response[256];
    int bytes_received;
    while ((bytes_received = recv(client_socket, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s", response);
        fflush(stdout);
    }

    if (bytes_received == 0) {
        printf("Server closed the connection.\n");
        fflush(stdout);
    } else if (bytes_received < 0) {
        perror("Error receiving data");
    }

    close(client_socket);
    printf("Client socket closed.\n");
    fflush(stdout);
    return 0;
}
