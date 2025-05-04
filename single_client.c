#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8237
#define END_SIGNAL 0

void send_and_receive(int client_socket, unsigned char a, unsigned char b, unsigned char c) {
    unsigned char sides[3] = {a, b, c};

    printf("Sending triple: %d, %d, %d\n", a, b, c);
    fflush(stdout);

    // Send all three numbers at once
    for (int j = 0; j < 3; j++) {
        if (send(client_socket, &sides[j], sizeof(sides[j]), 0) < 0) {
            perror("Error sending data");
            close(client_socket);
            exit(1);
        }
    }

    // Receive response
    char response[256];
    int bytes_received = recv(client_socket, response, sizeof(response) - 1, 0);
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("Server response: %s", response);
        fflush(stdout);
    } else if (bytes_received == 0) {
        printf("Server closed the connection.\n");
        fflush(stdout);
        close(client_socket);
        exit(0);
    } else {
        perror("Error receiving server's data");
        close(client_socket);
        exit(1);
    }
}

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

    printf("Client connected successfully.\n");
    fflush(stdout);

    // Send random triple (avoid 0)
    unsigned char a = (unsigned char)(1 + rand() % 31);
    unsigned char b = (unsigned char)(1 + rand() % 31);
    unsigned char c = (unsigned char)(1 + rand() % 31);
    send_and_receive(client_socket, a, b, c);

    // Send another triple
    a = (unsigned char)(1 + rand() % 31);
    b = (unsigned char)(1 + rand() % 31);
    c = (unsigned char)(1 + rand() % 31);
    send_and_receive(client_socket, a, b, c);

    // Send known valid triple 3,4,5
    send_and_receive(client_socket, 3, 4, 5);

    // Send another known valid triple 6,8,10
    send_and_receive(client_socket, 6, 8, 10);

    // Send END_SIGNAL to close session
    unsigned char end_signal = END_SIGNAL;
    if (send(client_socket, &end_signal, sizeof(end_signal), 0) <= 0) {
        perror("Error sending end signal");
    }

    close(client_socket);
    printf("Client socket closed.\n");
    fflush(stdout);
    return 0;
}
