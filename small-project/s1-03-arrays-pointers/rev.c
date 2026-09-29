#include <stdio.h>

void reverse(int* first, int n) {
    int start = 0;
    int end = n - 1;
    printf("size of first: %zu\n", sizeof first);
    while (start < end) {
        int* getfirst = first + start;
        int* getend = first + end;

        int temp = *getfirst;
        *getfirst = *getend;
        *getend = temp;

        start++;
        end--;
    }
}

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    int arr2[] = {1, 2, 3, 4, 5, 6};
    reverse(arr, 6);
    reverse(arr2, 6);
    printf("arr reversed: ");
    for (int i = 0; i <  6; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    printf("arr2 reversed: ");
    for (int i = 0; i < sizeof arr2 / sizeof arr2[0]; i++) {
        printf("%d ", arr2[i]);
    }
    printf("\n");

    
    return 0;
}