// Include for the preprocessor before it's sent to compiler
#include <stdio.h>
#include <conio.h>

// Global variable declaration here
int a = 10;

// Define the functions before use
int fun()
{
    int a = 11; // Inner value takes precendence over globals
    printf("\na value inside fun(): %d", a);
    return 0;
}

// Every fn must have atleast 1 a
int main()
{
    printf("a value inside main(): %d", a);
    fun();
    return 0;
}

/*
===========================
NOTES FOR FUTURE REFERENCE
===========================

1) Why "int" in "int main()"?
   - main() is a function, just like fun() or any other function.
   - The "int" before it means it RETURNS an integer (a whole number)
     to the operating system when the program finishes.
   - return 0;  --> tells the OS "program ran successfully"
   - return 1;  --> tells the OS "something went wrong"
   - If you don't return anything, some compilers give a warning/error.
   - int main() is the STANDARD way to write main in C.

2) What does "void" do?
   - "void" means NOTHING / EMPTY.
   - As a return type:  void fun()  --> this function returns nothing
   - As a parameter:    int main(void) --> this function takes no arguments
   - Example:
       void sayHello() {
           printf("Hello!");
           // no return needed because return type is void
       }
   - In short:
       int  = returns an integer
       void = returns nothing
*/
