// 2 bytes = 16 bits
#include <stdio.h>
#include <limits.h>

int main()
{
    printf("The size of int is %d bytes", sizeof(int));
    int var1 = INT_MAX;
    int var2 = INT_MIN;
    printf("\nThe max value is from %d to %d", var1, var2);
    return 0;
}
