#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    bool state;
} Particle;

Particle* create_particle(bool initial_state) {
    Particle* particle = (Particle*)malloc(sizeof(Particle));
    if (!particle) {
        perror("Failed to allocate memory for particle.");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_init(&particle->mutex, NULL);
    particle->state = initial_state;
    return particle;
}

void destroy_particle(Particle* particle) {
    pthread_mutex_destroy(&particle->mutex);
    free(particle);
}

void* thread_function(void* arg) {
    Particle* particle = (Particle*)arg;
    unsigned long long iterations = 0;
    while (iterations++ < 1000000) { // Ограничиваем количество итераций
        pthread_mutex_lock(&particle->mutex);
        particle->state = !particle->state;
        pthread_mutex_unlock(&particle->mutex);
        usleep(rand() % 500000); // Simulate random delay
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    
    // Create two particles with opposite initial states
    Particle* particle_a = create_particle(true);
    Particle* particle_b = create_particle(false);
    
    // Start threads to simulate entanglement
    pthread_t thread_a, thread_b;
    pthread_create(&thread_a, NULL, thread_function, (void*)particle_a);
    pthread_create(&thread_b, NULL, thread_function, (void*)particle_b);
    
    // Wait for threads to finish
    pthread_join(thread_a, NULL);
    pthread_join(thread_b, NULL);
    
    // Main loop to check the states of the particles
    for (unsigned long long i = 0; i < 1000000; i++) { // Проверка состояния ограниченное количество раз
        pthread_mutex_lock(&particle_a->mutex);
        pthread_mutex_lock(&particle_b->mutex);
        if (particle_a->state != !particle_b->state) {
            printf("Particles are not entangled!\n");
        } else {
            printf("Particles are entangled!\n");
        }
        pthread_mutex_lock(&particle_b->mutex);
        pthread_mutex_lock(&particle_a->mutex);
        sleep(1); // Check every second
    }
    
    // Clean up resources
    destroy_particle(particle_a);
    destroy_particle(particle_b);
    
    return 0;
}