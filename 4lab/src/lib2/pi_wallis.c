#include "../../src/math_utils.h"
#include <stdlib.h>

float Pi(int K) {
    float pi = 1.0;
    
    for (int i = 1; i <= K; i++) {
        float factor = (4.0 * i * i) / (4.0 * i * i - 1);
        pi *= factor;
    }
    
    return pi * 2;
}

char* translation(long x) {
    if (x == 0) {
        char* result = (char*)malloc(2 * sizeof(char));
        result[0] = '0';
        result[1] = '\0';
        return result;
    }
    
    // Определяем размер необходимой памяти
    long temp = x;
    int length = 0;
    
    while (temp > 0) {
        temp /= 3;
        length++;
    }
    
    // Выделяем память под результат
    char* result = (char*)malloc((length + 1) * sizeof(char));
    
    // Заполняем строку с конца
    int index = length - 1;
    temp = x;
    
    while (temp > 0) {
        int digit = temp % 3;
        result[index] = digit + '0';
        temp /= 3;
        index--;
    }
    
    result[length] = '\0';
    return result;
}