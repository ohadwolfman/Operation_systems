#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8237
#define END_SIGNAL 0

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    unsigned int seed = (unsigned int) atoi(argv[1]);
    srand(seed);

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

    printf("Client connected successfully, waiting before sending data...\n");
    fflush(stdout);
    sleep(1);

    unsigned char sides[3];
    for (int i = 0; i < 2; i++) {
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
//            printf("Sending: %d\n", sides[j]);
//            fflush(stdout);

            if (send(client_socket, &sides[j], sizeof(sides[j]), 0) < 0) {
                perror("Error sending data");
                close(client_socket);
                return 1;
            }
            printf("Sent: %d\n", sides[j]);
            fflush(stdout);
        }

        char response[256];
        int bytes_received = recv(client_socket, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Server response: %s", response);
            fflush(stdout);
        } else if (bytes_received == 0) {
            printf("Server closed the connection.\n");
            fflush(stdout);
            break;
        } else {
            perror("Error receiving server's data");
            break;
        }
    }

    int endSignal = END_SIGNAL;
    if (send(client_socket, &endSignal, sizeof(endSignal), 0) <= 0) {
        perror("Error sending end signal");
    }

    close(client_socket);
    printf("Client socket closed.\n");
    fflush(stdout);
    return 0;
}