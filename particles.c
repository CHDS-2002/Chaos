#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>

// Количество частиц
#define NUM_PARTICLES 10000

// Максимальная задержка при обработке частицы
#define MAX_DELAY 5000

// Структура хранения информации о частице
typedef struct {
    float mass;
    float charge;
    float spin;
} Particle;

// Массив частицы
Particle particles[NUM_PARTICLES];

// Функция для обработки частицы
void process_particle(size_t index) {
    // Симуляция обработки частицы
    usleep((rand() % (MAX_DELAY + 1)) * 1000); // Имитация задержки
    
    // Изменяем массу частицы случайным образом
    particles[index].mass = (rand() / (float)RAND_MAX) - 0.5; // Чтобы изменение было в пределах [-0.5; 0.5]
    
    printf("Processed particle at index %zu with new mass %.2f\n", index, particles[index].mass);
}

int main() {
    srand(time(NULL)); // Инициализация генератора случайных чисел
    
    // Заполнение массива частиц начальными значениями
    for (size_t i = 0; i < NUM_PARTICLES; i++) {
        particles[i].mass = rand() / (float)RAND_MAX;
        particles[i].charge = rand() / (float)RAND_MAX;
        particles[i].spin = rand() / (float)RAND_MAX;
    }
    
    // Использование liddispatch для создания пула потоков
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    
    // Отправляем задачи в пул потоков
    for (size_t i = 0; i < NUM_PARTICLES; i++) {
        dispatch_async(queue, ^{ process_particle(i); });
    }
    
    // Ожидаем завершения всех задач
    dispatch_sync(queue, ^{
        // Печать итоговых значений масс частиц
        for (size_t i = 0; i < NUM_PARTICLES; i++) {
            printf("Final particle at index %zu has mass %.2f\n", i, particles[i].mass);
        }
    });
    
    return 0;
}