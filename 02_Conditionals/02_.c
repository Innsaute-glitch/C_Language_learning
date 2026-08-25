// Any non zero value in if will execute.. For example:
#include <stdio.h>
#include <stdbool.h>

int main(void){
    if(5)
        printf("This works!");
    if(3.2937)
        printf("Yup, this works too!");
    if('hi')
        printf("Satisfied now?");
    if(true)
        printf("And ofc.. This works too!");
    return 0;
}
