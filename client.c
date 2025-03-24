#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    srand(atoi(argv[1])); // definition of seed
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    for (int i = 0; i < 10; i++) {
        unsigned char side = (rand() % 50) + 1; // random number from 1 to 50
        send(sock, &side, sizeof(side), 0);

        char response[10];
        recv(sock, response, sizeof(response), 0);
        printf("Server response: %s", response);
    }

    close(sock);
    return 0;
}
