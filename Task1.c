#include <stdio.h>
#include <stdlib.h>
#include "pythagorean.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <side1> <side2> <side3>\n", argv[0]);
        return 1;
    }

    unsigned char a = (unsigned char) atoi(argv[1]);
    unsigned char b = (unsigned char) atoi(argv[2]);
    unsigned char c = (unsigned char) atoi(argv[3]);

    if (is_pythagorean_triple(a, b, c))
        printf("YES\n");
    else
        printf("NO\n");

    return 0;
}
//void CreateOutputFile(){
//    FILE *file = fopen("output.txt","w");
//    if (file==NULL){
//        perror("Error opening file");
//    }
//    fprintf(file,"Operation Systems Course\nSubmitter: Ohad Wolfman\nId: 316552496\n\n");
//    fclose(file);
//}

//void PitagoricalThree(int argc, char *argv[]){
//    if (argc != 4) { // Check if we have exactly 3 arguments
//        FILE *file = fopen("output.txt", "a");
//        if(file == NULL){
//            perror("Error opening file");
//            return;
//        }
//        fprintf(file, "Error: Invalid number of arguments. Provide 3 integers.\n");
//        fclose(file);
//        return;
//    }
//
//    int a = atoi(argv[1]);
//    int b = atoi(argv[2]);
//    int c = atoi(argv[3]);
//
//    FILE *file = fopen("output.txt", "a");
//    if (file == NULL) {
//        perror("Error opening file");
//        return;
//    }
//    fprintf(file, "********** Task 1: **********\n");
//
//    fprintf(file, "a:%d, b:%d, c:%d: ", a, b, c);
//
//    if (is_pythagorean(a,b,c)) {
//        fprintf(file, "Yes\n");
//        printf("a:%d, b:%d, c:%d: Yes\n", a, b, c);
//    } else {
//        fprintf(file, "No\n");
//        printf("a:%d, b:%d, c:%d: No\n", a, b, c);
//    }
//    fclose(file);
//}
