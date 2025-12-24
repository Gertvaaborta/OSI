#ifndef MATH_UTILS_H
#define MATH_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Расчет числа Пи при заданной длине ряда
 * @param K Длина ряда (количество итераций)
 * @return Приближенное значение числа Пи
 */
float Pi(int K);
/**
 * @brief Перевод числа из десятичной системы в другую
 * @param x Число в десятичной системе
 * @return Указатель на строку с числом в новой системе счисления
 *         (необходимо освободить память после использования)
 */
char* translation(long x);
#ifdef __cplusplus
}
#endif

#endif 