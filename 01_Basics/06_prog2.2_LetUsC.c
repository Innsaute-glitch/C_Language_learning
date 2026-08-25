// Modified the scipt a little to include a little bit of verification

/* Get the needed modules.. */
#include <stdio.h>

/* Reverse digits in a 5 digit number */
int num, revnum, d5, d4, d3, d2, d1;
int main(void){
    printf("Enter a 5 digit number: ");
    scanf("%d", &num);
    d5 = num % 10;
    num = num/10;
    d4 = num % 10;
    num = num/10;
    d3 = num % 10;
    num = num/10;
    d2 = num % 10;
    num = num/10;
    d1 = num % 10;
    num = num/10;
    revnum = d5*10000 + d4*1000 + d3*100 + d2*10 + d1;
    if(num != 0)
        printf("The number is not a valid 5 digit number! Please Retry!");
    else
        printf("The reverse number is %d", revnum);
    printf("\nThanks for using the script!");
}
