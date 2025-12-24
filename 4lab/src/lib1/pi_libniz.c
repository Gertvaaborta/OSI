#include "../../src/math_utils.h"
#include <stdlib.h>

float Pi(int K) {
    float pi = 0.0;
    int sign = 1;
    
    for (int i = 0; i < K; i++) {
        pi += sign * (4.0 / (2 * i + 1));
        sign = -sign;
    }
    
    return pi;
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
        temp /= 2;
        length++;
    }
    
    // Выделяем память под результат
    char* result = (char*)malloc((length + 1) * sizeof(char));
    
    // Заполняем строку с конца
    int index = length - 1;
    temp = x;
    
    while (temp > 0) {
        result[index] = (temp % 2) + '0';
        temp /= 2;
        index--;
    }
    
    result[length] = '\0';
    return result;
}