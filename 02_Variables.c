// Here is how we define variables
// What %d means is that the function should take the next value. Like '"%d", var' means getting the value of the variable 'var'
// '%d' in C means that we should take a decimal
#include <stdio.h>
int main(){
    int var = 3;
    var = 4;
    printf("%d", var);
    return 0;
}

Also, you should define multiple variables simuntaneously
int main(){
    int var1, var2, var3;
    var1 = var2 = var3 = 4;
    printf("%d", var1);
    printf("%d", var2);
    printf("%d", var3);
    return 0;
}

// Use ctrl+/ to comment out bunch of lines in VS Code

int main(){
    int var1, var2, var3;
    var1 = var2 = var3 = 4;
    printf("'var1' Value: %d\n'var2' Value: %d\n'var3' Value: %d", var1, var2, var3);
    return 0;
}