#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // для sleep

// Максимальное количество итераций
#define MAX_ITERATIONS 100000

// Структура для передачи параметров потокам
typedef struct {
    double r;
    double x0;
    int start_iteration;
    int end_iteration;
} LogisticMapParams;

// Функция потока для вычисления логистического отображения
void* compute_logistic_map(void *params_ptr) {
    LogisticMapParams* params = (LogisticMapParams*)params_ptr;
    double r = params->r;
    double x0 = params->x0;
    int start_iter = params->start_iteration;
    int end_iter = params->end_iteration;
    
    // Вычисление значений
    double xn = x0;
    for (int i = start_iter; i <= end_iter; ++i) {
        xn = r * xn * (1 - xn);
        // Симуляция задержки для демонстрации многопоточности
        usleep(500);
    }
    
    // Освобождаем память перед выходом из потока
    free(params);
    pthread_exit(NULL);
}

// Основная Функция
int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <r> <x0> <num_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    double r = atof(argv[1]);       // Параметр r
    double x0 = atof(argv[2]);      // Параметр x0
    int num_threads = atoi(argv[3]); // Количество потоков
    
    // Проверяем корректность ввода
    if (r < 0 || r > 4 || x0 < 0 || x0 > 1 || num_threads <= 0) {
        fprintf(stderr, "Invalid input parameters.\n");
        exit(EXIT_FAILURE);
    }
    
    // Массив идентификаторов потоков
    pthread_t threads[num_threads];
    
    // Распределение итераций между потоками
    int iterations_per_thread = MAX_ITERATIONS / num_threads;
    int remainder_iterations = MAX_ITERATIONS % num_threads;
    
    // Запускаем потоки
    for (int i = 0; i < num_threads; ++i) {
        LogisticMapParams *params = (LogisticMapParams*)malloc(sizeof(LogisticMapParams));
        if (!params) {
            perror("Failed to allocate memory for thread parameters");
            exit(EXIT_FAILURE);
        }
        
        params->r = r;
        params->x0 = x0;
        params->start_iteration = i * iterations_per_thread + 1;
        params->end_iteration = (i == num_threads - 1) ? params->start_iteration + iterations_per_thread + remainder_iterations - 1 : params-> start_iteration + iterations_per_thread - 1;
        
        int rc = pthread_create(&threads[i], NULL, compute_logistic_map, (void*)params);
        if (rc) {
            fprintf(stderr, "Error creating thread: %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }
    
    // Ждем завершения всех потоков
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    printf("All threads completed successfully.\n");
    
    return EXIT_SUCCESS;
}