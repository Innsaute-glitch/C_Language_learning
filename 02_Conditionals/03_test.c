#include <stdio.h>
int main(){
    int retry, attempts;
    retry = 1;
    attempts = 0;
    while(retry){
        int num, i;
        i = 0;
        printf("Enter a number you want to count up to: ");
        scanf("%d", &num);
        if(num<0){
            printf("Invalid value detected\n");
            i = 0;
            continue;
        }
        // while(i++ < num){ // i++ < num means that whech for i<num, and then increase i by one. Also ++i < num would have increased i by 1 first, and then checked num. Only ++ exists for +1, +++ doesn't exist!
        //     printf("%d\n", i);
        // }
        for(i=1; i <= num; i++){ // We can also run a for loop here! for loop needs 3 conditions seperated by a semicolon: initial counter; test counter; increment counter. Example: counter is i. Test if i is less than num, proceed if true. Once u are done, increment i by some amount. That's what it is!
        // We could have written i = i+1 here too, instead of i++
            printf("%d\n", i);
        }
        attempts++;
        while(1){
            printf("\nTry Again? (0 for No, 1 for yes): ");
            scanf("%d", &retry);
            printf((retry == 0 || retry == 1)?"Valid Input Detected, Proceeding...\n":"Please retry - Answer must be 0 or 1!\n");
            if(retry == 0 || retry == 1){
                printf((retry)?"Restarting...\nI":"Exiting Successfully... You played %d times\n", attempts);
                break;
            }else{
                i = 2;
            }
        }
    }
}
