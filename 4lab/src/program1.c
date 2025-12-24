#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/math_utils.h"

int main() {
    printf("Программа с линковкой на этапе компиляции\n");
    printf("Используется реализация 1 (Лейбниц, двоичная система)\n");
    printf("Доступные команды:\n");
    printf("  0 - информация о реализации\n");
    printf("  1 K - вычисление Pi с длиной ряда K\n");
    printf("  2 N - перевод числа N в двоичную систему\n");
    printf("  exit - выход из программы\n\n");

    char command[256];
    
    while (1) {
        printf("> ");
        
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        // Удаляем символ новой строки
        command[strcspn(command, "\n")] = '\0';
        
        if (strcmp(command, "exit") == 0) {
            break;
        }
        
        if (strcmp(command, "0") == 0) {
            printf("Используется реализация 1:\n");
            printf("  - Pi вычисляется по ряду Лейбница\n");
            printf("  - Перевод в двоичную систему счисления\n");
            continue;
        }
        
        if (command[0] == '1' && command[1] == ' ') {
            int K;
            if (sscanf(command + 2, "%d", &K) == 1) {
                if (K > 0) {
                    float result = Pi(K);
                    printf("Pi(%d) = %.10f\n", K, result);
                } else {
                    printf("Ошибка: K должно быть положительным числом\n");
                }
            } else {
                printf("Ошибка: неверный формат команды\n");
            }
            continue;
        }
        
        if (command[0] == '2' && command[1] == ' ') {
            long N;
            if (sscanf(command + 2, "%ld", &N) == 1) {
                char* result = translation(N);
                printf("%ld (10) = %s (2)\n", N, result);
                free(result);
            } else {
                printf("Ошибка: неверный формат команды\n");
            }
            continue;
        }
        
        printf("Неизвестная команда. Введите '0', '1 K', '2 N' или 'exit'\n");
    }
    
    printf("Программа завершена.\n");
    return 0;
}