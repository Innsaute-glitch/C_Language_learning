/* To calculate the least number of notes needed to form a specific amount. Written in C and by myself, before checking the book version! */
// Notes are defined to be of currencies: 1, 5, 10, 50, and 100

// Getting the standard input output header module...
#include <stdio.h>

// Main function..
int price, note1, note5, note10, note50, note100;
int main(void){
    printf("What is the amount you wish to enter? ");
    scanf("%d", &price);
    if(price%100 != 0){
        note100 = price/100;
        price = price%100;
    }else{
        note100 = 0;}
    if(price%50 != 0){
        note50 = price/50;
        price = price%50;
    }else{
        note50 = 0;}
    if(price%10 != 0){
        note10 = price/10;
        price = price%10;
    }else{
        note10 = 0;}
    if(price%5 != 0){
        note5 = price/5;
        price = price%5;
    }else{
        note5 = 0;}
    note1 = price/1;
    printf("Number of notes of 100: %d\nNumber of note(s) of 50:  %d\nNumber of notes of 10:  %d\nNumber of notes of 5:   %d\nNumber of notes of 1:   %d", note100, note50, note10, note5, note1);
    printf("\nTotal number of notes is: %d\nThank you for using the script!", note100 + note50 + note10 + note5 + note1);
}
