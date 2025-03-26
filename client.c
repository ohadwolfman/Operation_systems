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

    // Send 3 random numbers and receive the response
    for (int i = 0; i < 2; i++) { //loop twice, once for random numbers, once for good numbers
        unsigned char sides[3];
        if (i == 0) {
            for (int j = 0; j < 3; j++) {
                sides[j] = (unsigned char)(rand() % 15);
            }
        } else {
            sides[0] = 5;
            sides[1] = 12;
            sides[2] = 13;
        }

        for (int j = 0; j < 3; j++) {
            if (send(client_socket, &sides[j], sizeof(sides[j]), 0) < 0) {
                perror("Error sending data");
                close(client_socket);
                return 1;
            }
            printf("Sent: %d\n", sides[j]);
            fflush(stdout);
        }

        // Receive the server's response
        char response[256];
        int bytes_received = recv(client_socket, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Server response: %s", response);
            fflush(stdout);
        } else if (bytes_received == 0) {
            printf("Server closed the connection.\n");
            fflush(stdout);
            break; // Exit the loop if the server closes the connection
        } else {
            perror("Error receiving data");
            break; // Exit the loop on error
        }
    }

    // Shut down sending side
    if (shutdown(client_socket, SHUT_WR) < 0) {
        perror("Error shutting down write direction");
    }

    close(client_socket);
    printf("Client socket closed.\n");
    fflush(stdout);
    return 0;
}
