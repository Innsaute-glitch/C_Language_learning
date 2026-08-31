/* Demonstration of if statement */
#include <stdio.h>
int main(){
    int num,attempts;
    attempts = 0;
    while(attempts != 10){ // Attempts = 10 would break the loop
        printf( "\nEnter a number less than or equal to 10: " );
        scanf ( "%d", &num );
        if ( num <= 10 ){
            printf ( ">>> What an obedient servant you are!" );
            attempts = 10; // Break the loop
        }else if( num > 10 && attempts <= 2){
            printf(">>> Follow the instructions!\n");
            attempts += 1;
        }else{
            printf(">>> Why won't u follow what I say?!");
            attempts = 10; // Break the loop
        }
    }
    printf("\nThe End"); // The end is always output
}
