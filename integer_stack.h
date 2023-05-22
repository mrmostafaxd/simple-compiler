#ifndef INTEGER_STACK_H
#define INTEGER_STACK_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 1000

// Structure representing a stack
typedef struct {
    int top;          // Index of the top element in the stack
    int array[MAX_SIZE];  // Array to store the stack elements
} Stack;

// Function to initialize the stack
void initializeStack(Stack* stack) {
    stack->top = -1;
}

// Function to check if the stack is empty
int isEmpty(Stack* stack) {
    return stack->top == -1;
}

// Function to check if the stack is full
int isFull(Stack* stack) {
    return stack->top == MAX_SIZE - 1;
}

// Function to push an element onto the stack
void push(Stack* stack, int element) {
    if (isFull(stack)) {
        printf("Stack Overflow: Cannot push element onto the stack\n");
        return;
    }
    stack->array[++stack->top] = element;
}

// Function to pop the top element from the stack
int pop(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack Underflow: Cannot pop element from the stack\n");
        return -1;
    }
    return stack->array[stack->top--];
}

// Function to get the top element of the stack without removing it
int peek(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return -1;
    }
    return stack->array[stack->top];
}

// Function to display the elements in the stack
void displayStack(Stack* stack) {
    if (isEmpty(stack)) {
        printf("Stack is empty\n");
        return;
    }
    printf("Stack elements: ");
    for (int i = stack->top; i >= 0; i--) {
        printf("%d ", stack->array[i]);
    }
    printf("\n");
}

// int main() {
//     Stack stack;
//     initializeStack(&stack);

//     push(&stack, 10);
//     push(&stack, 20);
//     push(&stack, 30);

//     displayStack(&stack);  // Stack elements: 30 20 10

//     printf("Top element: %d\n", peek(&stack));  // Top element: 30

//     printf("Popped element: %d\n", pop(&stack));  // Popped element: 30

//     displayStack(&stack);  // Stack elements: 20 10

//     return 0;
// }

Stack blockStack = {.top = 0, .array[0] = 0};

#endif