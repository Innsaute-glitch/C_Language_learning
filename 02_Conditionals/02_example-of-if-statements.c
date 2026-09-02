// Any non zero value in if will execute.. For example:
#include <stdio.h>
#include <stdbool.h>

int main(void){
    if(5)
        printf("This works!");
    if(3.2937)
        printf("\nYup, this works too!");
    if(-3)
        printf("\nSatisfied now?");
    if(true)
        printf("\nAnd ofc.. This works too!");
    return 0;
}
