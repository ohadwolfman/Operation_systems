#include <stdio.h>
#include "Task1.c"

void CreateOutputFile();
//void PitagoricalThree(int a, int b, int c);
void PitagoricalThree(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    CreateOutputFile();
    PitagoricalThree(argc, argv);
    printf("Task1 completed\n");
    return 0;
}
