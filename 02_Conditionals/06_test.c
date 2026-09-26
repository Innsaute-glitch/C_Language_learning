// Prog to write x terms of 1/1! + 2/2! + 3/3! + 4/4!...
#include <stdio.h>

int get_int(char *prompt, int *var)
{
    printf("%s", prompt);
    scanf("%d", var);
    return 0;
}

int get_factorial(int var)
{
    int ans = 1;
    for (int value = var; value > 1; value--)
    {
        ans *= value;
    }
    return ans;
}

int main()
{
    int sequence, print_terms;
    print_terms = 0;
    double answer = 0;
    while (1)
    {
        if (get_int("Do u want to see indiviual terms? (0 for No, 1 for Yes): ", &print_terms) != 0)
        {
            printf("Please enter either 0 or 1!\n");
            continue;
        }
        if (print_terms != 0 && print_terms != 1)
        {
            printf("Please enter either 0 or 1!\n");
        }
        else
        {
            break;
        }
    }
    if (get_int("Enter the number of terms: ", &sequence) != 0 || sequence < 1)
    {
        printf("Please enter a positive integer.\n");
        return 1;
    }
    for (int term = 1; term <= sequence; term++)
    {
        double temp;
        temp = (term*1.0) / get_factorial(term); // Manually converting to float
        answer += temp;
        if (print_terms)
        {
            printf("Term %d or %d/%d! is %f\n", term, term, term, temp);
        }
    }
    printf("The answer of 1/1! + 2/2! + 3/3! ... upto %d terms is: %f", sequence, answer);
    return 0;
}
