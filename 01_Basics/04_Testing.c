/* Testing what happs when u try different variations of different functions u learnt? */

# include <stdio.h>
int main(){
    printf("%d", printf("%s", "Hello World!"));
    return 0;
} /* This gives "Hello World12" because prinf not only prints, but also returns the value. So the function becomes smth like printf("%d", 12) after hello world is printed. (Since "Hello World!" is 12 characters - %d printed is 12) */

// Function for powers: (x**y doesn't work here - unlike python)
#include <math.h>
main( )
{
 int a ;
 a = pow ( 3, 2 ) ;
 printf ("%d", a) ;
}

// How does math work in C? () have most priotity (Priority of 0, u can say). /, % and * enjoy priority 1, while +, - enjoy priority of 2. Finally, = enjoys priority of 3. If there is a tie, for example 2/3*4, first would be 2/3 since both / and * enjoy left to right associativity (Tho, honestly? Just use parenthesis for most priority and clarity)

// IMPORTANT POINT: if 6 and 4 are both int, unlike python - the value is also int. So, the answer would be 1, not 1.5
