#include <stdio.h>

int get_val(const char *prompt, int *val){
    printf("%s", prompt); // %c doesn't work for some reason?
    scanf("%d", val);
    return 0;
}

int validate(int num){
    if(num < 0)
    {
        printf("Invalid input detected! Please enter positive numbers only!\n");
        return 0;
    }else if(num == 0){
        return 2;
    }
    return 1;
}

int main(){
    int attempts, validation;
    attempts = 0;
    while(1){
        get_val("~ How many times? ", &attempts);
        validation = validate(attempts);
    if(validation == 1){
            break;
        }else if(validation == 2){
            printf("Invalid input detected! Please enter a number greater than 0!\n");
            continue;
        }else{
            continue;
        }
    }
    for (int a = 0; a < attempts; a++)
    {
        int num1, num2, mode;
        while(1){
            get_val("~ Enter the first number: ", &num1);
            if(validate(num1) == 1 || validate(num1) == 2){
                break;
            }else{
                continue;
            }
        }
        while(1){
            get_val("~ Enter the second number: ", &num2);
            if(validate(num2) == 1 || validate(num2) == 2){
                break;
            }else{
                continue;
            }
        }

        while(1){
            get_val("~ Select mode 0 or 1: ", &mode);
            if (mode == 1){
                for (int i = 0; i < num1; i++){
                    for (int j = 0; j < num2; j++){
                        printf(">>> i = %d, j = %d, Sum = %d\n", i, j, i + j);
                    }
                }
                break;
            }else if(mode == 0){
                for (int i = 0; i < num2; i++){
                    for (int j = 0; j < num1; j++){
                        printf(">>> i = %d, j = %d, Sum = %d\n", i, j, i + j);
                    }
                }
                break;
            }else{
                printf(">>> Invalid mode detected! Please select either 0 or 1!\n");
            }
        }
    printf(">>> All done! Exiting program...\n");
    return 0;
    }
}

// This script is a simple demonstration of nested for loops in C. It asks u to input two numbers and a mode (basically a mirror). Depending on the selected mode, it shows all the possible combinations of the two numbers and their sums. The outer loop runs for the number of attempts specified by the user, while the inner loops iterate through the ranges defined by the two input numbers.
