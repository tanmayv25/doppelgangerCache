#include <stdio.h>

int main() {
    int i, temp;
    int a[100000];
    for(i = 0; i < 100000; i++) {
        a[i] = 100;
    }
    for(i = 0; i < 100000; i++) {
        temp = a[i];
    }
}
    
