#include <stdio.h>

#include "core.h"

void print_array(int *arr)
{
    printf("arr len=%d, cap=%d ", array_count(arr), array_capacity(arr));
    for (usize i = 0; i < array_count(arr); i++) {
        printf("[%d]", arr[i]);
    }
    printf("\n");
}
int main() {
    printf("Starting test...\n");

    int *arr = NULL;

    print_array(arr);
    array_push(arr, 5);
    print_array(arr);
    array_push(arr, 4);
    print_array(arr);
    array_push(arr, 3);
    print_array(arr);
    array_push(arr, 2);
    print_array(arr);
    array_push(arr, 1);
    print_array(arr);
}
