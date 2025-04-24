#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

void* print_myName(void* arg) {
    printf("My name is Ohad!\n");
    return NULL;
}

void run_single_thred(void* print_hello){
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, print_hello, NULL);  // create a thread
    // first parameter, e.g:&thread_id - מצביע למשתנה מסוג pthread_t שבו יישמר מזהה התהליכון שנוצר
    // second parameter, e.g: NULL - הגדרות מיוחדות ליצירת התהליכון (כמו גודל מחסנית), אבל ברוב המקרים פשוט כותבים NULL
    // third parameter, e.g: print_hello - שם הפונקציה שהתהליכון יריץ. היא חייבת להיות מהצורה: void* func(void* arg)
    // fourth parameter, e.g: NULL - מה שתרצה להעביר לפונקציה הזו כארגומנט

    pthread_join(thread_id, NULL);  // wait until the end of the thread
    printf("The thread is finished.\n");
}

int follow_fork() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child. PID: %d\n", getpid());
    } else {
        printf("Parent. PID: %d\n", getpid());
        wait(NULL);
    }

    printf("Done. PID: %d\n", getpid());
    return 0;
}

void* print_received(void* arg) {
    printf("Thread received: %s\n", (char*)arg);
    return NULL;
}

int run_hello() {
    pthread_t t;
    char* msg = "Hello!";
    pthread_create(&t, NULL, print_received, msg);
    pthread_join(t, NULL);
//    sleep(1);
    return 0;
}

int send_ping() {
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        // תהליך בן
        close(fd[1]); // הבן לא כותב
        char buffer[10];
        read(fd[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
    } else {
        // תהליך אב
        close(fd[0]); // האב לא קורא
        write(fd[1], "ping", strlen("ping") + 1);
        wait(NULL);
    }
    return 0;
}

int memory_allocation() {
    int* ptr = malloc(sizeof(int));
    *ptr = 42;
    printf("Value: %d\n", *ptr);
    free(ptr);
    return 0;
}

int dup2_useage() {
    int fd = open("log.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    //"log.txt" – שם הקובץ שתרצה לפתוח.
    //O_WRONLY – פתיחה לכתיבה בלבד.
    //O_CREAT – אם הקובץ לא קיים – צור אותו.
    //O_TRUNC – אם הקובץ כבר קיים – תמחק את התוכן שלו.
    //O_APPEND – כל כתיבה תתווסף בסוף הקובץ, בלי למחוק את מה שכבר נמצא בו.
    //0644 – הרשאות לקובץ:
    //בעלים: קריאה + כתיבה (6)
    //קבוצה: קריאה (4)
    //אחרים: קריאה (4)

    dup2(fd, STDOUT_FILENO);
    printf("Hello, file!\n");
    printf("My name is ohad\n");
    printf("I'm learning operation systems\n");
    close(fd);
    return 0;
}

int main() {
    run_single_thred(&print_myName);
    run_single_thred(&follow_fork);
    run_single_thred(&run_hello);
    run_single_thred(&send_ping);
    run_single_thred(&memory_allocation);
    run_single_thred(&dup2_useage);
    run_single_thred(&dup2_useage);

    return 0;
}
