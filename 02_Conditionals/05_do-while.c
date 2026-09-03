// Example of a do=while loop in C!
#include <stdio.h>

int main()
{
    int num;
    char try_again;
    do
    {
        printf("Enter a number: ");
        scanf("%d", &num);
        printf("The square of the number %d is %d!\n", num, num * num);
        printf("Try again? (y/n): ");
        scanf(" %c", &try_again);
    }while (try_again == 'y' || try_again == 'Y');
}
