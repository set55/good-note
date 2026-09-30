#include <stdio.h>

size_t my_strlen(char *s){
    char *start = s;
    while (*s != '\0') {
        s++;
    }
    return s - start;
}

int main() {
    char li[] = "Linux";
    char empty[] = ""; 
    char word[] = "字";
    char t[5] = "Linux";
    char u[4] = "Linux";
    printf("Length of '%s' is %zu\n", li, my_strlen(li));
    printf("Length of '%s' is %zu\n", empty, my_strlen(empty));
    printf("Length of '%s' is %zu\n", word, my_strlen(word));
    printf("Length of '%s' is %zu\n", t, my_strlen(t));
    printf("Length of '%s' is %zu\n", u, my_strlen(u));

    char name[16] = "Linux";
    printf("Length of '%s' is %zu\n", name, sizeof name);
    printf("Length of '%s' is %zu\n", name, my_strlen(name));
    
    return 0;
}