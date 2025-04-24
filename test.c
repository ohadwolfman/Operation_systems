#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_PORT 8237
#define SERVER_IP "127.0.0.1"
#define NUM_CLIENTS 6

void *client_thread(void *arg) {
    int client_id = *(int *)arg;
    free(arg);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(SERVER_PORT),
    };
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        pthread_exit(NULL);
    }

    printf("Client %d connected.\n", client_id);

    switch (client_id) {
        case 1: {
            // בקשות תקינות
            unsigned char triples[][3] = {
                    {3, 4, 5}, {5, 12, 13}, {8, 15, 17}
            };
            for (int i = 0; i < 3; i++) {
                send(sock, triples[i], 3, 0);
                char response[32] = {0};
                recv(sock, response, sizeof(response)-1, 0);
                printf("Client %d: %d %d %d => %s", client_id, triples[i][0], triples[i][1], triples[i][2], response);
            }
            break;
        }

        case 2: {
            // שליחת פחות מ־3 בתים
            unsigned char short_data[2] = {5, 6};
            send(sock, short_data, 2, 0);
            sleep(1);
            char buf[32] = {0};
            int r = recv(sock, buf, sizeof(buf)-1, 0);
            if (r <= 0) printf("Client %d: Connection closed after short data.\n", client_id);
            else printf("Client %d: Unexpected response: %s\n", client_id, buf);
            break;
        }

        case 3: {
            // שליחת יותר מ־3 בתים
            unsigned char long_data[5] = {3, 4, 5, 6, 7};
            send(sock, long_data, 5, 0);
            sleep(1);
            char buf[32] = {0};
            int r = recv(sock, buf, sizeof(buf)-1, 0);
            if (r <= 0) printf("Client %d: Connection closed after long data.\n", client_id);
            else printf("Client %d: Unexpected response: %s\n", client_id, buf);
            break;
        }

        case 4: {
            // שליחת תווים אקראיים
            char garbage[] = "XYZ";
            send(sock, garbage, 3, 0);
            sleep(1);
            char buf[32] = {0};
            int r = recv(sock, buf, sizeof(buf)-1, 0);
            if (r <= 0) printf("Client %d: Connection closed after garbage.\n", client_id);
            else printf("Client %d: Unexpected response: %s\n", client_id, buf);
            break;
        }

        case 5: {
            // שליחה מהירה של הרבה שלשות
            for (int i = 0; i < 30; i++) {
                unsigned char triple[3] = {i, i+1, i+2};
                send(sock, triple, 3, 0);
                char buf[32] = {0};
                recv(sock, buf, sizeof(buf)-1, 0);
                printf("Client %d: Sent %d %d %d => %s", client_id, triple[0], triple[1], triple[2], buf);
            }
            break;
        }

        case 6: {
            // חיבור ואז ניתוק פתאומי
            printf("Client %d: Simulating abrupt disconnect.\n", client_id);
            close(sock);  // No END_SIGNAL
            return NULL;
        }
    }

    // נסיים עם END_SIGNAL אם לא נותקנו
    if (client_id != 6) {
        unsigned char end = 0;
        send(sock, &end, 1, 0);
        close(sock);
        printf("Client %d disconnected.\n", client_id);
    }

    pthread_exit(NULL);
}

int main() {
    pthread_t clients[NUM_CLIENTS];

    for (int i = 0; i < NUM_CLIENTS; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&clients[i], NULL, client_thread, id);
        usleep(50000);  // קצת פיזור בזמנים
    }

    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }

    printf("All test clients completed.\n");
    return 0;
}
