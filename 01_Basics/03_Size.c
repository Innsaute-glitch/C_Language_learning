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
// Int and Float variables have a memory of 4 bytes (Tho it depends machine to machine)
// Char variables have a memory of 1 byte (8 bits = 256 combinations (2**8))
// Boolean also require 1 byte of memory (8 bits) (even tho there is a need of only a single bit)
// Double needs 8 bytes of memory (more accurate that float, but needs more memory...) (can store digits upto the magnitude of 10**308, in comparisson to about 10**38 in float)
