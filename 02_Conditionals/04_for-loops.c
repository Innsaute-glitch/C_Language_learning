#include <stdio.h>
int main(){
    int attempts;
    attempts = 0;
    printf("How many times? ");
    scanf("%d", &attempts);
    for(int a = 0; a<attempts; a++){
        int num1, num2, mode;
        printf("Enter the first number: ");
        scanf("%d", &num1);
        printf("Enter the second number: ");
        scanf("%d", &num2);
        printf("select mode 0 or 1: ");
        scanf("%d", &mode);
        if(mode){
            for(int i = 0; i<num1; i++){
                for(int j = 0; j<num2; j++){
                    printf("i = %d, j = %d, Sum = %d\n", i, j, i+j);
                }
            }
        }else{
            for(int i = 0; i<num2; i++){
                for(int j = 0; j<num1; j++){
                    printf("i = %d, j = %d, Sum = %d\n", i, j, i+j);
                }
            }
        }
    }
    return 0;
}

// This script is a simple demonstration of nested for loops in C. It asks u to input two numbers and a mode (basically a mirror). Depending on the selected mode, it shows all the possible combinations of the two numbers and their sums. The outer loop runs for the number of attempts specified by the user, while the inner loops iterate through the ranges defined by the two input numbers.
