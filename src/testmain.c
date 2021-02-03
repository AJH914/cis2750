#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helper.h"

int TEST1 = 0;
int TEST2 = 1;

void testcase(char* expected, char* actual);
void testcase(char* expected, char* actual){
    printf("Expected output: %s\n", expected);
    printf("Actual output: %s\n", actual);
    printf("\n");
}

int main(int argc, char* argv[]){
    //test 1 tests fileOpener
    if (TEST1){
        //test 1: test with null
        char* tester=fileOpener(NULL);
        printf("Expected output: NULL\n");
        printf("Actual output: %s\n", tester);
        printf("\n");
        //test 2: test with single line of text
        tester=fileOpener(argv[1]);
        printf("Expected output: the the\n");
        printf("Actual output: %s\n", tester);
        free(tester);
        printf("\n");
        //test 3: test with multiple lines of text
        tester=fileOpener(argv[2]);
        printf("Expected output: the the\ncow cow\n");
        printf("Actual output: %s\n", tester);
        free(tester);
        //test 4: test with empty string
        tester=fileOpener("");
        printf("Expected output: NULL\n");
        printf("Actual output: %s\n", tester);
        printf("\n");
    }
    //test 2 tests stringCopy
    if (TEST2){
        char* s = stringCopy(NULL, 1, 5);
        testcase(NULL, s);
    }
}