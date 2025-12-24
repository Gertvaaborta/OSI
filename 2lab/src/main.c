#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>

typedef struct {
    double** matrix;
    double* solution;
    int start_row;
    int end_row;
    int current_col;
    int total_size;
    int is_forward;
} Task;

int max_threads_allowed = 1;

void* forward_elimination(void* arg) {
    Task* t = (Task*)arg;
    double** matrix = t->matrix;
    int n = t->total_size;
    int current_col = t->current_col;
    
    for (int i = t->start_row; i <= t->end_row; i++) {
        if (i <= current_col) continue;
        
        double factor = matrix[i][current_col] / matrix[current_col][current_col];
        for (int j = current_col; j <= n; j++) {
            matrix[i][j] -= factor * matrix[current_col][j];
        }
    }
    
    free(t);
    return NULL;
}

void* backward_substitution(void* arg) {
    Task* t = (Task*)arg;
    double** matrix = t->matrix;
    double* solution = t->solution;
    int n = t->total_size;
    
    for (int i = t->start_row; i >= t->end_row; i--) {
        solution[i] = matrix[i][n];
        for (int j = i + 1; j < n; j++) {
            solution[i] -= matrix[i][j] * solution[j];
        }
        solution[i] /= matrix[i][i];
    }
    
    free(t);
    return NULL;
}

int find_pivot(double** matrix, int col, int n) {
    int pivot_row = col;
    double max_val = fabs(matrix[col][col]);
    
    for (int i = col + 1; i < n; i++) {
        if (fabs(matrix[i][col]) > max_val) {
            max_val = fabs(matrix[i][col]);
            pivot_row = i;
        }
    }
    return pivot_row;
}

void swap_rows(double** matrix, int row1, int row2, int n) {
    for (int j = 0; j <= n; j++) {
        double temp = matrix[row1][j];
        matrix[row1][j] = matrix[row2][j];
        matrix[row2][j] = temp;
    }
}

void parallel_gauss_elimination(double** matrix, double* solution, int n, int threads) {
    if (n == 0) return;
    
    max_threads_allowed = threads;
    
    // Прямой ход метода Гаусса
    for (int i = 0; i < n; i++) {
        // Поиск ведущего элемента
        int pivot = find_pivot(matrix, i, n);
        if (pivot != i) {
            swap_rows(matrix, i, pivot, n);
        }
        
        // Проверка на вырожденность
        if (fabs(matrix[i][i]) < 1e-10) {
            fprintf(stderr, "Матрица вырождена или почти вырождена (из-за погрешности double)\n");
            return;
        }
        
        // Распределение строк между потоками для исключения
        int rows_per_thread = (n - i - 1) / threads;
        if (rows_per_thread == 0) rows_per_thread = 1;
        
        pthread_t* thread_pool = (pthread_t*)malloc(threads * sizeof(pthread_t));
        int thread_count = 0;
        
        for (int t = 0; t < threads; t++) {
            int start_row = i + 1 + t * rows_per_thread;
            int end_row = (t == threads - 1) ? n - 1 : start_row + rows_per_thread - 1;
            
            if (start_row >= n) break;
            
            Task* task = (Task*)malloc(sizeof(Task));
            task->matrix = matrix;
            task->solution = solution;
            task->start_row = start_row;
            task->end_row = end_row;
            task->current_col = i;
            task->total_size = n;
            task->is_forward = 1;
            
            pthread_create(&thread_pool[thread_count], NULL, forward_elimination, task);
            thread_count++;
        }
        
        // Ожидание завершения всех потоков
        for (int t = 0; t < thread_count; t++) {
            pthread_join(thread_pool[t], NULL);
        }
        
        free(thread_pool);
    }
    
    // Обратная подстановка
    int rows_per_thread = n / threads;
    if (rows_per_thread == 0) rows_per_thread = 1;
    
    pthread_t* thread_pool = (pthread_t*)malloc(threads * sizeof(pthread_t));
    int thread_count = 0;
    
    for (int t = 0; t < threads; t++) {
        int start_row = n - 1 - t * rows_per_thread;
        int end_row = (t == threads - 1) ? 0 : start_row - rows_per_thread + 1;
        
        if (start_row < 0) break;
        
        Task* task = (Task*)malloc(sizeof(Task));
        task->matrix = matrix;
        task->solution = solution;
        task->start_row = start_row;
        task->end_row = end_row;
        task->current_col = 0;
        task->total_size = n;
        task->is_forward = 0;
        
        pthread_create(&thread_pool[thread_count], NULL, backward_substitution, task);
        thread_count++;
    }
    
    // Ожидание завершения всех потоков
    for (int t = 0; t < thread_count; t++) {
        pthread_join(thread_pool[t], NULL);
    }
    
    free(thread_pool);
}

double** allocate_matrix(int n, int m) {
    double** matrix = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        matrix[i] = (double*)malloc(m * sizeof(double));
    }
    return matrix;
}

void free_matrix(double** matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main() {
    int n, threads;
    
    printf("Введите размер системы и количество потоков: ");
    scanf("%d %d", &n, &threads);
    
    // Выделение памяти под матрицу и решение
    double** matrix = allocate_matrix(n, n + 1);
    double* solution = (double*)malloc(n * sizeof(double));
    
    printf("Введите матрицу коэффициентов и правые части:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= n; j++) {
            scanf("%lf", &matrix[i][j]);
        }
    }
    
    // Вызов параллельного метода Гаусса
    parallel_gauss_elimination(matrix, solution, n, threads);
    
    // Вывод решения
    printf("Решение системы:\n");
    for (int i = 0; i < n; i++) {
        printf("x[%d] = %.6f\n", i, solution[i]);
    }
    
    // Освобождение памяти
    free_matrix(matrix, n);
    free(solution);
    
    return 0;
}