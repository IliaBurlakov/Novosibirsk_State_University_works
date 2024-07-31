#define _CRT_SECURE_NO_WARNINGS


#include <stdio.h>
#include <stdlib.h>


int Max(int x, int y) {
    return (x > y) ? x : y;
}


void fill_table(int** table, int* weight, int* prices, int n, int w) {
    for (int j = 0; j < w + 1; j++)
        table[0][j] = 0;

    for (int i = 1; i < n + 1; i++) {
        table[i][0] = 0;

        for (int j = 1; j < w + 1; j++) {

            if (j >= weight[i - 1]) //we have space for new item
                table[i][j] = Max(table[i - 1][j], table[i - 1][j - weight[i - 1]] + prices[i - 1]); //whether we take the item or not
            else //we don't have space for new item
                table[i][j] = table[i - 1][j];
        }
    }
}


void get_numbers(int* numbers, int** table, int n, int w, int* weight, int* counter) {
    if (table[n][w] == 0) {
        return;
    }
    if (table[n - 1][w] == table[n][w]) // we don't take item
        get_numbers(numbers, table, n - 1, w, weight, counter);
    else {                              //we take item
        numbers[(*counter)++] = n;
        get_numbers(numbers, table, n - 1, w - weight[n - 1], weight, counter);
    }
}


int main(void) {
    int n, w;
    if (scanf("%d %d", &n, &w) != 2) {
        printf("N or W error!");
        return 0;
    }
    int* weight = malloc(sizeof(int) * n);
    int* prices = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) {
        if (scanf("%d %d", &weight[i], &prices[i]) != 2) {
            printf("An error occurred while getting weigth and price of element number %d\n", i + 1);
            free(weight);
            free(prices);
            return 0;
        }
    }
    int** table = (int**)malloc(sizeof(int*) * (n + 1)); //table with max costs for different capacity and number of items (N+1 strings and W+1 column)
    if (!table) {
        printf("Cannot allocate memory for table\n");
        free(weight);
        free(prices);
        return 0;
    }
    for (int i = 0; i < n + 1; i++) {
        table[i] = (int*)(malloc(sizeof(int) * (w + 1)));
        if (!table[i]) {
            printf("Cannot allocate memory for string in table\n");
            free(weight);
            free(prices);
            free(table);
            return 0;
        }
    }
    fill_table(table, weight, prices, n, w);
    printf("%d\n", table[n][w]); //the best total cost
    int* numbers = malloc(sizeof(int) * n);
    int counter = 0;
    get_numbers(numbers, table, n, w, weight, &counter);
    for (int j = counter - 1; j >= 0; j--)
        printf("%d %d\n", weight[numbers[j] - 1], prices[numbers[j] - 1]);
    for (int i = 0; i < n + 1; i++) {
        free(table[i]);
    }
    free(table);
    free(weight);
    free(prices);
    free(numbers);
    return 0;
}
