/* Motive: Make a script that takes a, b and c from a quadratic equation, calculates the determinanats and tells you the roots of the same. It should have a retry function, and an attempts counter for how many times u played.*/

// Import the required headers
#include <stdio.h>
#include <math.h>

// Define the global variables
int attempts, get_retry;
float D, root1, root2, a, b, c;
char get_retry_helper;

// This function writes the user's input into the variable supplied by address.
// It does not need to return the value because scanf already stores it through
// the pointer. Returning value here would return a float pointer (float *),
// which does not match the function's original float return type.
void get_values(const char *prompt, float *value)
{
    printf("%s", prompt);
    scanf("%f", value);
}

// Start the main loop
int main()
{
    get_retry = 1;
    while (get_retry)
    {
        // Increment once at the start of every complete run, including retries.
        ++attempts; // attempts++ works too
        if (attempts == 1)
        {
            printf("--- Starting the script: Please keep ax**2 + bx + c = 0 form in mind! ---\n");
        }
        else
        {
            printf("--- Restarting the script: Please keep ax**2 + bx + c = 0 form in mind! Attempt: %d ---\n", attempts);
        }
        get_values("Enter the value of a: ", &a);
        get_values("Enter the value of b: ", &b);
        get_values("Enter the value of c: ", &c);
        D = (b * b) - (4 * a * c);
        // Compute the roots based on the value of D
        if (D > 0)
        {
            printf("The roots are real and different.\n");
            root1 = (-b + sqrt(D)) / (2 * a);
            root2 = (-b - sqrt(D)) / (2 * a);
            printf("The roots are: %.2f and %.2f\n", root1, root2);
        }
        else if (D == 0)
        {
            printf("The roots are real and same.\n");
            root1 = root2 = -b / (2 * a);
            printf("The roots are both: %.2f and %.2f\n", root1, root2);
        }
        else
        {
            printf("The roots are complex and different.\n");
            float realPart = -b / (2 * a);
            // D is negative in this branch, so sqrt(D) is not a real number.
            // sqrt(-D) gives the positive magnitude of the imaginary part;
            // the two roots then use +i and -i versions of that magnitude.
            float imaginaryPart = sqrt(-D) / (2 * a);
            printf("The roots are: %.2f + %.2fi and %.2f - %.2fi\n", realPart, imaginaryPart, realPart, imaginaryPart); // Add i as a suffix to the imaginary part
        }
        while(1)
        {
            printf("Do you want to retry? (Enter 'y' for Yes, or 'n' for No): ");
            scanf(" %c", &get_retry_helper);
            if (get_retry_helper == 'y' || get_retry_helper == 'Y')
            {
                // Keep the outer loop running for another complete attempt.
                get_retry = 1;
                break;
            }
            else if (get_retry_helper == 'n' || get_retry_helper == 'N')
            {
                printf("Thank you for using the script! You played %d times.\n", attempts);
                return 0;
            }
            else
            {
                printf("Invalid input. Please retry, and make sure to enter either 'y' or 'n'!\n");
            }
        }
    }
}
