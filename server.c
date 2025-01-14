#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_CONNECTIONS 256
#define BUF_SIZE 4096
#define BACKLOG 16

// Описание структуры для хранения данных о соединении
typedef struct Connection {
    int fd;
    struct pollfd poll_fd;
    char buf[BUF_SIZE];
    size_t buf_len;
    size_t pos;
    pthread_mutex_t lock;
} Connection;

Connection connections[MAX_CONNECTIONS];
int num_connections = 0;

// Обработчик сигналов для завершения работы программы
void signal_handler(int signum) {
    switch(signum) {
        case SIGTERM:
        case SIGINT:
            fprintf(stderr, "\nCaught signal %d, shutting down...\n", signum);
            for (int i = 0; i < num_connections; ++i) {
                shutdown(connections[i].fd, SHUT_RDWR);
                close(connections[i].fd);
            }
            exit(EXIT_SUCCESS);
    }
}

// Инициализация соединения
void init_connection(Connection* conn, int fd) {
    conn->fd = fd;
    conn->buf_len = 0;
    conn->pos = 0;
    conn->poll_fd.fd = fd;
    conn->poll_fd.events = POLLIN | POLLOUT;
    pthread_mutex_init(&conn->lock, NULL);
    memset(conn->buf, 0, BUF_SIZE);  // Обнуляем буфер
}

// Освобождение ресурсов соединения
void destroy_connection(Connection* conn) {
    shutdown(conn->fd, SHUT_RDWR);
    close(conn->fd);
    pthread_mutex_destroy(&conn->lock);
}

// Чтение данных из соединения
ssize_t read_from_connection(Connection* conn) {
    ssize_t nread = 0;
    pthread_mutex_lock(&conn->lock);
    nread = recv(conn->fd, conn->buf + conn->buf_len, BUF_SIZE - conn->buf_len, 0);
    if (nread > 0) {
        conn->buf_len += nread;
    }
    pthread_mutex_unlock(&conn->lock);
    return nread;
}

// Запись данных в соединение
ssize_t write_to_connection(Connection* conn) {
    ssize_t nwritten = 0;
    pthread_mutex_lock(&conn->lock);
    nwritten = send(conn->fd, conn->buf + conn->pos, conn->buf_len - conn->pos, 0);
    if (nwritten > 0) {
        conn->pos += nwritten;
        if (conn->pos == conn->buf_len) {
            conn->buf_len = 0;
            conn->pos = 0;
        }
    }
    pthread_mutex_unlock(&conn->lock);
    return nwritten;
}

// Потоковая функция для обработки соединений
void* process_connection(void* arg) {
    Connection* conn = (Connection*)arg;
    while (1) {
        int ret = poll(&conn->poll_fd, 1, -1);
        if (ret == -1 && errno != EINTR) {
            perror("Poll error");
            break;
        } else if (ret > 0) {
            if (conn->poll_fd.revents & POLLIN) {
                ssize_t nread = read_from_connection(conn);
                if (nread <= 0) {
                    if (nread == 0 || (nread == -1 && errno != EAGAIN)) {
                        fprintf(stderr, "Client disconnected\n");
                        destroy_connection(conn);
                        break;
                    }
                }
            }
            if (conn->poll_fd.revents & POLLOUT) {
                ssize_t nwritten = write_to_connection(conn);
                if (nwritten <= 0) {
                    if (nwritten == 0 || (nwritten == -1 && errno != EAGAIN)) {
                        fprintf(stderr, "Write error\n");
                        destroy_connection(conn);
                        break;
                    }
                }
            }
        }
    }
    return NULL;
}

// Основная функция сервера
int main(int argc, char** argv) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t sin_size;
    pthread_t tid;

    // Установка обработчиков сигналов
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Настройки сокета
    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Привязка сокета к порту
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Переводим сокет в режим прослушивания
    if (listen(server_fd, BACKLOG) == -1) {
        perror("Listen failed");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Основной цикл сервера
    while (1) {
        sin_size = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &sin_size);
        if (client_fd == -1) {
            perror("Accept failed");
            continue;
        }

        // Проверка лимита соединений
        if (num_connections >= MAX_CONNECTIONS) {
            fprintf(stderr, "Max connections reached, rejecting new connection\n");
            close(client_fd);
            continue;
        }

        // Инициализируем новое соединение
        init_connection(&connections[num_connections], client_fd);
        num_connections++;

        // Запуск потока для обработки соединения
        if (pthread_create(&tid, NULL, process_connection, &connections[num_connections - 1]) != 0) {
            perror("Thread creation failed");
            destroy_connection(&connections[num_connections - 1]);
            num_connections--;
            continue;
        }

        // Отсоединяем поток
        pthread_detach(tid);
    }

    close(server_fd);
    return EXIT_SUCCESS;
}