#include <stdio.h>
#include "single_client.c"

void CreateOutputFile();
//void PitagoricalThree(int a, int b, int c);
void PitagoricalThree(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    CreateOutputFile();
    PitagoricalThree(argc, argv);
    return 0;
}
