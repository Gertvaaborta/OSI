#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "../src/math_utils.h"

typedef float (*PiFunc)(int);
typedef char* (*TransFunc)(long);

int main() {
    void* library_handle = NULL;
    PiFunc pi_func = NULL;
    TransFunc trans_func = NULL;
    
    int current_lib = 1;  // 1 - первая реализация, 2 - вторая
    
    printf("Программа с загрузкой библиотек во время выполнения\n");
    printf("Доступные команды:\n");
    printf("  0 - переключить реализацию (сейчас: %d)\n", current_lib);
    printf("  1 K - вычисление Pi с длиной ряда K\n");
    printf("  2 N - перевод числа N в другую систему\n");
    printf("  exit - выход из программы\n\n");
    
    // Загружаем первую библиотеку
    library_handle = dlopen("./libmath1.so", RTLD_LAZY);
    if (!library_handle) {
        fprintf(stderr, "Ошибка загрузки библиотеки: %s\n", dlerror());
        return 1;
    }
    
    // Получаем указатели на функции
    pi_func = (PiFunc)dlsym(library_handle, "Pi");
    trans_func = (TransFunc)dlsym(library_handle, "translation");
    
    if (!pi_func || !trans_func) {
        fprintf(stderr, "Ошибка получения функций: %s\n", dlerror());
        dlclose(library_handle);
        return 1;
    }
    
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
            // Переключаем библиотеку
            dlclose(library_handle);
            
            if (current_lib == 1) {
                current_lib = 2;
                library_handle = dlopen("./libmath2.so", RTLD_LAZY);
                if (!library_handle) {
                    fprintf(stderr, "Ошибка загрузки библиотеки 2: %s\n", dlerror());
                    return 1;
                }
                printf("Переключено на реализацию 2 (Валлис, троичная система)\n");
            } else {
                current_lib = 1;
                library_handle = dlopen("./libmath1.so", RTLD_LAZY);
                if (!library_handle) {
                    fprintf(stderr, "Ошибка загрузки библиотеки 1: %s\n", dlerror());
                    return 1;
                }
                printf("Переключено на реализацию 1 (Лейбниц, двоичная система)\n");
            }
            
            // Получаем указатели на функции из новой библиотеки
            pi_func = (PiFunc)dlsym(library_handle, "Pi");
            trans_func = (TransFunc)dlsym(library_handle, "translation");
            
            if (!pi_func || !trans_func) {
                fprintf(stderr, "Ошибка получения функций: %s\n", dlerror());
                dlclose(library_handle);
                return 1;
            }
            
            continue;
        }
        
        if (command[0] == '1' && command[1] == ' ') {
            int K;
            if (sscanf(command + 2, "%d", &K) == 1) {
                if (K > 0) {
                    float result = pi_func(K);
                    printf("Pi(%d) = %.10f (реализация %d)\n", K, result, current_lib);
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
                char* result = trans_func(N);
                if (current_lib == 1) {
                    printf("%ld (10) = %s (2)\n", N, result);
                } else {
                    printf("%ld (10) = %s (3)\n", N, result);
                }
                free(result);
            } else {
                printf("Ошибка: неверный формат команды\n");
            }
            continue;
        }
        
        printf("Неизвестная команда. Введите '0', '1 K', '2 N' или 'exit'\n");
    }
    
    // Закрываем библиотеку
    if (library_handle) {
        dlclose(library_handle);
    }
    
    printf("Программа завершена.\n");
    return 0;
}