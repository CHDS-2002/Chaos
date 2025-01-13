#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define NX 100    // Размер сетки по оси x
#define NY 100    // Размер сетки по оси y
#define NT 200    // Количество временных шагов
#define DX 0.05   // Шаг по пространству по оси x
#define DY 0.05   // Шаг по пространству по оси y
#define DT 0.025  // Временной шаг
#define C 1.0     // Скорость света

void apply_boundary_conditions(double ***X, int a) {
    // Применяем граничные условия (фиксированные концы струны)
    for (int i = 0; i < NX; ++i) {
        X[a][i][0] = 0.0;
        X[a][i][NY-1] = 0.0;
    }
    for (int j = 0; j < NY; ++j) {
        X[a][0][j] = 0.0;
        X[a][NX-1][j] = 0.0;
    }
}

void initial_condition(double ***X, int a) {
    // Устанавливаем начальные условия (синусоидальная волна)
    for (int i = 0; i < NX; ++i) {
        for (int j = 0; j < NY; ++j) {
            X[a][i][j] = sin(M_PI*i/(NX-1))*sin(M_PI*j/(NY-1));
        }
    }
    apply_boundary_conditions(X, a);
}

void update_string(double ****X, int a, double rx2, double ry2) {
    for (int i = 1; i < NX-1; ++i) {
        for (int j = 1; j < NY-1; ++j) {
            X[a][i][j][2] = 2*X[a][i][j][1] - X[a][i][j][0] + rx2 * (X[a][i+1][j][1] - 2*X[a][i][j][1] + X[a][i-1][j][1]) + ry2 * (X[a][i][j+1][1] - 2*X[a][i][j][1] + X[a][i][j-1][1]);
        }
    }
    apply_boundary_conditions(X, a);
}

int main() {
	const int num_components = 26; // Предположим, что у нас 26 пространственных измерений
    double ****X = malloc(num_components * sizeof(double***)); // Массивы для каждого измерения
    for (int a = 0; a < num_components; ++a) {
        X[a] = malloc(NT * sizeof(double**));
        for (int n = 0; n < NT; ++n) {
            X[a][n] = malloc(NX * sizeof(double*));
            for (int i = 0; i < NX; ++i) {
                X[a][n][i] = malloc(NY * sizeof(double));
            }
        }
    }
    
    // Инициализация начальных условий
    for (int a = 0; a < num_components; ++a) {
        initial_condition(X[a], a);
    }
    
    // Коэффициенты для численных схем
    double rx2 = pow(C * DT / DX, 2);
    double ry2 = pow(C * DT / DY, 2);
    
    // Основной цикл
    for (int n = 1; n < NT-1; ++n) {
        for (int a = 0; a < num_components; ++a) {
            update_string(X, a, rx2, ry2);
        }
        // Копирование данных для следующего шага
        for (int a = 0; a < num_components; ++a) {
            for (int i = 0; i < NX; ++i) {
                for (int j = 0; j < NY; ++j) {
                    X[a][i][j][0] = X[a][i][j][1];
                    X[a][i][j][1] = X[a][i][j][2];
                }
            }
        }
    }
    
    // Освобождение памяти
    for (int a = 0; a < num_components; ++a) {
        for (int n = 0; n < NT; ++n) {
            for (int i = 0; i < NX; ++i) {
                free(X[a][n][i]);
            }
            free(X[a][n]);
        }
        free(X[a]);
    }
    free(X);
    
    printf("Симуляция завершена.\n");
    
    return 0;
}

