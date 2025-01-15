#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 8
#define NUM_ITERATIONS 10000000

typedef struct {
    double* values;
    int start_index;
    int end_index;
} thread_data_t;

void* calculate_mean(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    double sum = 0.0;
    for (int i = data->start_index; i < data->end_index; ++i) {
        sum += data->values[i];
    }
    double* result = (double*)malloc(sizeof(double));
    *result = sum / (data->end_index - data->start_index);
    return (void*)result;
}

double generate_random_double() {
    return ((double)rand()) / RAND_MAX;
}

void free_memory(double* array, pthread_t* threads, thread_data_t* data) {
    free(array);
    free(threads);
    free(data);
}

int main() {
	// Инициализация генератора случайных чисел
	srand(time(NULL));
	
	// Выделение памяти под массив случайных чисел
	double* random_values = (double*)malloc(NUM_ITERATIONS * sizeof(double));
	if (!random_values) {
	    printf("Ошибка выделения памяти\n");
	    return 1;
	}
	
	// Заполнение массива случайными числами
	for (int i = 0; i < NUM_ITERATIONS; ++i) {
	    random_values[i] = generate_random_double();
	}
	
	// Вычисление среднего значения
	double mean = 0.0;
	for (int i = 0; i < NUM_ITERATIONS; ++i) {
	    mean += random_values[i];
	}
	mean /= NUM_ITERATIONS;
	
	// Выделение памяти под потоки и данные потоков
	pthread_t* threads = (pthread_t*)malloc(NUM_THREADS * sizeof(pthread_t));
	thread_data_t* data = (thread_data_t*)malloc(NUM_THREADS * sizeof(thread_data_t));
	if (!threads || !data) {
	    printf("Ошибка выделения памяти\n");
	    free(random_values);
	    return 1;
	}
	
	// Распределение данных между потоками
	int chunk_size = NUM_ITERATIONS / NUM_THREADS;
	for (int i = 0; i < NUM_THREADS; ++i) {
	    data[i].values = random_values;
	    data[i].start_index = i * chunk_size;
	    data[i].end_index = (i == NUM_THREADS - 1) ? NUM_ITERATIONS : (i + 1) * chunk_size;
	}
	
	// Запуск потоков
	for (int i = 0; i < NUM_THREADS; ++i) {
	    pthread_create(&threads[i], NULL, calculate_mean, &data[i]);
	}
	
	// Ожидание завершения потоков
	double total_sum = 0.0;
	void* result;
	for (int i = 0; i < NUM_THREADS; ++i) {
	    pthread_join(threads[i], &result);
	    total_sum += *(double*)result;
	}
	
	// Освобождение памяти
	free_memory(random_values, threads, data);
	
	// Проверка выполнения законов
	double chebyshev_mean = total_sum / NUM_THREADS;
	double kolmogorov_mean = mean;
	
	printf("Среднее значение по Чебышеву: %f\n", chebyshev_mean);
	printf("Среднее значение по Колмогорову: %f\n", kolmogorov_mean);
	
    return 0;
}

