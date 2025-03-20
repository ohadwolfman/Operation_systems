//
// שלב א - רק מתמטיקה.
//קבלו 3 אורכי צלעות בעזרת argc, argv.
//החזר YES אם שלושת המספרים מהווים שלשה פיתגורית וNO אחרת.
//הנחות - מדובר רק במשתנים מסוג unsigned char.
//
#include <stdio.h>
#include <stdlib.h>

void CreateOutputFile(){
    FILE *file = fopen("output.txt","w");
    if (file==NULL){
        perror("Error opening file");
    }
    fprintf(file,"Operation Systems Course\nSubmitter: Ohad Wolfman\nId: 316552496\n\n");
    fclose(file);
}

//void PitagoricalThree(int a, int b, int c){
//    FILE *file = fopen("output.txt","a");
//    if (file==NULL){
//        perror("Error opening file");
//    }
//    fprintf(file,"********** Task 1: **********\n");
//    if ((a*a + b*b) == (c*c)){
//        fprintf(file,"Yes\n");
//    }
//    else{
//        fprintf(file,"No\n");
//    }
//    fclose(file);
//}

void PitagoricalThree(int argc, char *argv[]){
    if (argc != 4) { // Check if we have exactly 3 arguments
        FILE *file = fopen("output.txt", "a");
        if(file == NULL){
            perror("Error opening file");
            return;
        }
        fprintf(file, "Error: Invalid number of arguments. Provide 3 integers.\n");
        fclose(file);
        return;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int c = atoi(argv[3]);

    FILE *file = fopen("output.txt", "a");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "********** Task 1: **********\n");
    fprintf(file, "a:%d, b:%d, c:%d: ", a, b, c);

    if ((a * a + b * b) == (c * c)) {
        fprintf(file, "Yes\n");
    } else {
        fprintf(file, "No\n");
    }
    fclose(file);
}
