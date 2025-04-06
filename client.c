#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define PORT 8237
#define END_SIGNAL 0
#define NUM_THREADS 5
#define TRIANGLES_PER_THREAD 2

void send_triangle(int sockfd, unsigned char a, unsigned char b, unsigned char c) {
    unsigned char sides[3] = {a, b, c};
    for (int i = 0; i < 3; i++) {
        if (send(sockfd, &sides[i], sizeof(sides[i]), 0) < 0) {
            perror("Error sending data");
            exit(1);
        }
        printf("Thread %lu sent: %d\n", pthread_self(), sides[i]);
        fflush(stdout);
    }

    char response[256];
    int bytes_received = recv(sockfd, response, sizeof(response) - 1, 0);
    if (bytes_received > 0) {
        response[bytes_received] = '\0';
        printf("Thread %lu received: %s", pthread_self(), response);
        fflush(stdout);
    } else if (bytes_received == 0) {
        printf("Server closed the connection for thread %lu.\n", pthread_self());
        fflush(stdout);
        exit(0);
    } else {
        perror("Error receiving data");
        exit(1);
    }
}

void* client_thread(void* arg) {
    int base_seed = *((int*)arg);
    int thread_id = (int)(size_t)pthread_self();  // for unique seeds
    srand(base_seed + thread_id);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        pthread_exit(NULL);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sockfd);
        pthread_exit(NULL);
    }

    for (int i = 0; i < TRIANGLES_PER_THREAD; i++) {
        unsigned char a = rand() % 30 + 1;
        unsigned char b = rand() % 30 + 1;
        unsigned char c = rand() % 30 + 1;

        send_triangle(sockfd, a, b, c);

        char response[256];
        int bytes_received = recv(sockfd, response, sizeof(response) - 1, 0);
        if (bytes_received > 0) {
            response[bytes_received] = '\0';
            printf("Thread %ld received: %s", pthread_self(), response);
            fflush(stdout);
        } else if (bytes_received == 0) {
            printf("Thread %ld: Server closed connection.\n", pthread_self());
            break;
        } else {
            perror("Error receiving data");
            break;
        }

        usleep(200000); // 0.2 שניות הפסקה
    }

    int endSignal = END_SIGNAL;
    send(sockfd, &endSignal, sizeof(endSignal), 0);
    close(sockfd);
    printf("Thread %lu done.\n", pthread_self());
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <seed>\n", argv[0]);
        return 1;
    }

    int base_seed = atoi(argv[1]);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, client_thread, &base_seed) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("All threads finished.\n");
    return 0;
}