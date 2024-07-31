#define MAX_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


typedef struct Stack {
    int value[1024];
    int top;
} Stack;


void syntax_error(void) {
    printf("syntax error");
    exit(0);
}


int IsOperation(char s) {
    if (s == '+' || s == '-' || s == '*' || s == '/')
        return 1;
    else
        return 0;
}


int IsEmpty(Stack* stack) {
    return stack->top == -1;
}


void Push(Stack* stack, int x) {
    stack->top++;
    stack->value[stack->top] = x;
}


int Pop(Stack* stack) {
    if (!IsEmpty(stack))
        return stack->value[stack->top--];
    else
        return 0;
}


int GetTop(Stack* stack) {
    if (!IsEmpty(stack))
        return stack->value[stack->top];
    else
        return 0;
}


int GetNumber(char* string, int* i, int string_len) {
    int number = 0;
    while (*i < string_len && isdigit(string[*i])) {
        number = number * 10 + (string[*i] - '0');
        ++*i;
    }
    -- * i;
    return number;
}


void PerformAnOperation(Stack* numbers, char operation) {
    if (numbers->top < 1)
        syntax_error();
    int a = Pop(numbers);
    int b = Pop(numbers);
    switch (operation)
    {
    case '+':
        Push(numbers, b + a);
        break;
    case '-':
        Push(numbers, b - a);
        break;
    case '*':
        Push(numbers, b * a);
        break;
    case '/':
        if (a == 0) {
            printf("division by zero");
            exit(0);
        }
        else
            Push(numbers, b / a);

        break;
    default:
        syntax_error();
    }
}


int Calculate(char* string, int string_len) {
    if (string[0] == '\n')
        syntax_error();
    Stack numbers;
    Stack operations;
    numbers.top = -1;
    operations.top = -1;
    int operations_priority[50] = { 0 };
    operations_priority['+'] = 1;
    operations_priority['-'] = 1;
    operations_priority['*'] = 2;
    operations_priority['/'] = 2;
    for (int i = 0; i < string_len; i++) {
        if (string[i] == '(')
            Push(&operations, (int)string[i]);
        else if (string[i] == ')') {
            if (i == 0 || string[i - 1] == '(')
                syntax_error();
            while (GetTop(&operations) != '(')
                PerformAnOperation(&numbers, (char)Pop(&operations));
            Pop(&operations);
        }
        else if (IsOperation(string[i])) {
            if ((i > 0 && IsOperation(string[i - 1])) || (i == string_len - 1))
                syntax_error();
            while (!IsEmpty(&operations) && (operations_priority[(int)GetTop(&operations)] >= operations_priority[(int)string[i]]))
                PerformAnOperation(&numbers, (char)Pop(&operations));
            Push(&operations, (int)(string[i]));
        }
        else if (isdigit(string[i]))
            Push(&numbers, GetNumber(string, &i, string_len));
        else
            syntax_error();
    }
    while (!IsEmpty(&operations))
        PerformAnOperation(&numbers, (char)Pop(&operations));
    return Pop(&numbers);
}


int main(void)
{
    FILE* file = fopen("in.txt", "r");
    if (file == NULL)
        return 0;
    char string_in[MAX_SIZE];
    if (fgets(string_in, MAX_SIZE, file) == NULL) {
        fclose(file);
        return 0;
    }
    int string_len = strlen(string_in) - 1;
    int res = Calculate(string_in, string_len);
    printf("%d", res);
    fclose(file);
    return 0;
}