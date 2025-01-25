#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 10000
#define MAX 18

char *Type[] = {
    "QUARK UP", "QUARK CHARGE", "QUARK TRUE",
    "QUARK DOWN", "QUARK STRANGE", "QUARK BOTTOM",
    "ELECTRON", "MUON", "TAON",
    "NEUTRINO ELECTRON", "NEUTRINO MUON", "NEUTRINO TAON",
    "GLUON", "PHOTON", "Z-BOSON", "W+-BOSON", "W--BOSON",
    "HIGGS", "GRAVITON"
};

double Mass[] = {
    24, 1275, 173070,
    48, 95, 4180,
    0.511, 175.7, 1777,
    0.022, 1.7, 15.5,
    0, 0, 91.2, 80.4, 80.4,
    125, 0
};

double Charge[] = {
    2 / 3., 2 / 3., 2 / 3.,
    -1 / 3., -1 / 3., -1 / 3.,
    -1, -1, -1,
    0, 0, 0,
    0, 0, 0, 1, -1,
    0, 0
};

double Spin[] = {
    1 / 2., 1 / 2., 1 / 2.,
    1 / 2., 1 / 2., 1 / 2.,
    1 / 2., 1 / 2., 1 / 2.,
    1 / 2., 1 / 2., 1 / 2.,
    1, 1, 1,
    1, 1, 1, 1, 1,
    0, 2
};

typedef struct {
    char *name;
    char *type;
    double mass;
    double charge;
    double spin;
} Particle;

int main() {
    srand((unsigned) time(NULL));  // Инициализируем генератор случайных чисел

    Particle *particles = malloc(SIZE * sizeof(Particle));  // Выделяем память для массива частиц

    if (!particles) {  // Проверяем успешность выделения памяти
        fprintf(stderr, "Ошибка выделения памяти!\n");
        return 1;
    }

    for (int i = 0; i < SIZE; ++i) {
        int index = rand() % MAX;  // Генерируем случайный индекс

        particles[i].name = Type[index];   // Заполняем структуру данными
        particles[i].type = Type[index];
        particles[i].mass = Mass[index];
        particles[i].charge = Charge[index];
        particles[i].spin = Spin[index];
    }

    printf("\tPARTICLES\n");
    printf("Name\tType\tMass\tCharge\tSpin\n");

    for (int i = 0; i < SIZE; ++i) {
        printf("%s\t%s\t%g\t%g\t%g\n", particles[i].name, particles[i].type, particles[i].mass, particles[i].charge, particles[i].spin);
    }

    free(particles);  // Освобождаем выделенную память

    return 0;
}