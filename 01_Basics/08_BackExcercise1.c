// Take 2 numbers from user and use a C program to get all the trignometric ratios for the two (in progress):

// Include the header file and define variables...
#include <stdio.h>
#include <math.h>
float num1_final, num2_final;

float ask_num(char *prompt)
{
    float number_entered;
    printf("Please type the %s number: ", prompt);
    scanf("%f", &number_entered);
    return number_entered;
}

int main()
{
    float sine;
    num1_final = ask_num("1st");
    num2_final = ask_num("2nd");
    sine = num1_final / sqrt(num1_final * num1_final + num2_final * num2_final);
    printf("sin of %f and %f is %f degrees", num1_final, num2_final, sine);
}
