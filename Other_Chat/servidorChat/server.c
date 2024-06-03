#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define TCP_PORT 8162
#define BUFFER_SIZE 4096
#define MAX_NOMBRE 50
#define MAX_MENSAJE (BUFFER_SIZE - MAX_NOMBRE - 8) // BUFFER_SIZE - MAX_NOMBRE - longitud de "CHAT|%s|%s" - terminador nulo
#define MAX_FILE_NAME 255

typedef struct {
    int sockfd;
    char nombre[MAX_NOMBRE];
} Cliente;

Cliente clientes[10]; // Lista de clientes conectados
int num_clientes = 0; // Número de clientes conectados
pthread_mutex_t clientes_mutex = PTHREAD_MUTEX_INITIALIZER;

void manejar_mensaje(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], mensaje[MAX_MENSAJE];
    sscanf(data, "CHAT|%49[^|]|%49[^|]|%4095[^\n]", remitente, destinatario, mensaje);

    printf("Mensaje recibido de %s para %s: %s\n", remitente, destinatario, mensaje);

    pthread_mutex_lock(&clientes_mutex);
    // Buscar al destinatario en la lista de clientes
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            char buffer[BUFFER_SIZE];
            snprintf(buffer, BUFFER_SIZE, "CHAT|%s|%s", remitente, mensaje);
            write(clientes[i].sockfd, buffer, strlen(buffer));
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);
}
/*
void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    pthread_mutex_lock(&clientes_mutex);
    int destinatario_sockfd = -1;
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            destinatario_sockfd = clientes[i].sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);

    if (destinatario_sockfd == -1) {
        printf("Destinatario no encontrado\n");
        return;
    }

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "FILE|%s|%s|%zu|", remitente, nombre_archivo, file_size);

    if (write(destinatario_sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error al enviar el encabezado del archivo");
        return;
    }

    size_t bytes_leidos;
    size_t bytes_totales = 0;
    while (bytes_totales < file_size && (bytes_leidos = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        if (write(destinatario_sockfd, buffer, bytes_leidos) < 0) {
            perror("Error al reenviar los datos del archivo");
            return;
        }
        bytes_totales += bytes_leidos;
    }

    if (bytes_totales == file_size) {
        printf("Archivo %s reenviado correctamente a %s\n", nombre_archivo, destinatario);
    } else {
        printf("Error al reenviar el archivo %s\n", nombre_archivo);
    }
}
*/
/*
void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    pthread_mutex_lock(&clientes_mutex);
    int destinatario_sockfd = -1;
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            destinatario_sockfd = clientes[i].sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);

    if (destinatario_sockfd == -1) {
        printf("Destinatario %s no encontrado\n", destinatario);
        return;
    }

    // Enviar el archivo al destinatario
    if (write(destinatario_sockfd, data, strlen(data)) < 0) {
        perror("Error al reenviar el archivo al destinatario");
        return;
    }

    printf("Archivo reenviado correctamente a %s\n", destinatario);
}
*/
/*
void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    // Generar un nombre único para el archivo
    char nombre_completo[MAX_FILE_NAME + 10]; // 10 caracteres adicionales para un sufijo único
    sprintf(nombre_completo, "%s_%lu", nombre_archivo, time(NULL));

    // Crear y abrir el archivo en modo de escritura binaria
    FILE *file = fopen(nombre_completo, "wb");
    if (file == NULL) {
        perror("Error al abrir el archivo para escritura");
        return;
    }

    // Recibir los datos del archivo y escribirlos en el archivo local
    size_t bytes_recibidos = 0;
    size_t bytes_leidos;
    char buffer[BUFFER_SIZE];
    while (bytes_recibidos < file_size && (bytes_leidos = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        fwrite(buffer, 1, bytes_leidos, file);
        bytes_recibidos += bytes_leidos;
    }

    // Verificar si se recibió el archivo completo
    if (bytes_recibidos != file_size) {
        fprintf(stderr, "Error: No se recibió el archivo completo\n");
        fclose(file);
        remove(nombre_completo); // Eliminar el archivo incompleto
        return;
    }

    printf("Archivo recibido y guardado como %s\n", nombre_completo);

    fclose(file);
}
*/
/*
void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    // Buscar al destinatario en la lista de clientes
    pthread_mutex_lock(&clientes_mutex);
    int destinatario_sockfd = -1;
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            destinatario_sockfd = clientes[i].sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);

    if (destinatario_sockfd == -1) {
        printf("Destinatario %s no encontrado\n", destinatario);
        return;
    }

    // Enviar el encabezado del archivo al destinatario
    if (write(destinatario_sockfd, data, strlen(data)) < 0) {
        perror("Error al reenviar el encabezado del archivo al destinatario");
        return;
    }

    // Reenviar los datos del archivo al destinatario
    size_t bytes_enviados = 0;
    ssize_t bytes_leidos;
    char buffer[BUFFER_SIZE];
    while (bytes_enviados < file_size && (bytes_leidos = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (write(destinatario_sockfd, buffer, bytes_leidos) < 0) {
            perror("Error al reenviar los datos del archivo al destinatario");
            return;
        }
        bytes_enviados += bytes_leidos;
    }

    if (bytes_enviados != file_size) {
        fprintf(stderr, "Error: No se pudo reenviar el archivo completo al destinatario\n");
        return;
    }

    printf("Archivo reenviado correctamente a %s\n", destinatario);
}
*/
/*
void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    // Buscar al destinatario en la lista de clientes
    pthread_mutex_lock(&clientes_mutex);
    int destinatario_sockfd = -1;
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            destinatario_sockfd = clientes[i].sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);

    if (destinatario_sockfd == -1) {
        printf("Destinatario %s no encontrado\n", destinatario);
        return;
    }

    // Enviar el encabezado del archivo al destinatario
    if (write(destinatario_sockfd, data, strlen(data)) < 0) {
        perror("Error al reenviar el encabezado del archivo al destinatario");
        return;
    }

    // Reenviar los datos del archivo al destinatario
    size_t bytes_enviados = 0;
    ssize_t bytes_leidos;
    char buffer[BUFFER_SIZE];
    while (bytes_enviados < file_size && (bytes_leidos = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (write(destinatario_sockfd, buffer, bytes_leidos) < 0) {
            perror("Error al reenviar los datos del archivo al destinatario");
            return;
        }
        bytes_enviados += bytes_leidos;
    }

    if (bytes_enviados != file_size) {
        fprintf(stderr, "Error: No se pudo reenviar el archivo completo al destinatario\n");
        return;
    }

    printf("Archivo reenviado correctamente a %s\n", destinatario);
}
*/

void manejar_archivo(int sockfd, const char *data) {
    char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
    size_t file_size;
    sscanf(data, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

    printf("Archivo recibido de %s para %s: %s (tamaño: %zu bytes)\n", remitente, destinatario, nombre_archivo, file_size);

    // Buscar al destinatario en la lista de clientes
    pthread_mutex_lock(&clientes_mutex);
    int destinatario_sockfd = -1;
    for (int i = 0; i < num_clientes; i++) {
        if (strcmp(clientes[i].nombre, destinatario) == 0) {
            destinatario_sockfd = clientes[i].sockfd;
            break;
        }
    }
    pthread_mutex_unlock(&clientes_mutex);

    if (destinatario_sockfd == -1) {
        printf("Destinatario %s no encontrado\n", destinatario);
        return;
    }

    // Enviar el encabezado del archivo al destinatario
    if (write(destinatario_sockfd, data, strlen(data)) < 0) {
        perror("Error al reenviar el encabezado del archivo al destinatario");
        return;
    }

    // Reenviar los datos del archivo al destinatario
    size_t bytes_enviados = 0;
    ssize_t bytes_leidos;
    char buffer[BUFFER_SIZE];
    while (bytes_enviados < file_size && (bytes_leidos = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (write(destinatario_sockfd, buffer, bytes_leidos) < 0) {
            perror("Error al reenviar los datos del archivo al destinatario");
            return;
        }
        bytes_enviados += bytes_leidos;
    }

    if (bytes_enviados != file_size) {
        fprintf(stderr, "Error: No se pudo reenviar el archivo completo al destinatario\n");
        return;
    }

    printf("Archivo reenviado correctamente a %s\n", destinatario);
}


void *manejar_cliente(void *arg) {
    int sockfd = *((int *)arg);
    char buffer[BUFFER_SIZE];
    char nombre[MAX_NOMBRE];

    // Leer el nombre del cliente
    int n = read(sockfd, nombre, sizeof(nombre) - 1);
    if (n <= 0) {
        perror("Error al leer el nombre");
        close(sockfd);
        pthread_exit(NULL);
    }
    nombre[n] = '\0';

    pthread_mutex_lock(&clientes_mutex);
    // Agregar el cliente a la lista
    printf("Nuevo cliente conectado: %s\n", nombre);
    clientes[num_clientes].sockfd = sockfd;
    strncpy(clientes[num_clientes].nombre, nombre, sizeof(clientes[num_clientes].nombre) - 1);
    num_clientes++;
    pthread_mutex_unlock(&clientes_mutex);

    // Manejar mensajes del cliente
    while (1) {
        n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("Cliente %s desconectado\n", nombre);
            close(sockfd);

            pthread_mutex_lock(&clientes_mutex);
            // Remover el cliente de la lista
            for (int i = 0; i < num_clientes; i++) {
                if (clientes[i].sockfd == sockfd) {
                    clientes[i] = clientes[num_clientes - 1];
                    num_clientes--;
                    break;
                }
            }
            pthread_mutex_unlock(&clientes_mutex);
            break;
        } else {
            buffer[n] = '\0';
            if (strncmp(buffer, "CHAT|", 5) == 0) {
                manejar_mensaje(sockfd, buffer);
            } else if (strncmp(buffer, "FILE|", 5) == 0) {
                manejar_archivo(sockfd, buffer);
            }
        }
    }

    pthread_exit(NULL);
}

void *escuchar_conexiones(void *arg) {
    int server_sock = *((int *)arg);
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while (1) {
        int new_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (new_sock < 0) {
            perror("Error al aceptar conexión");
            continue;
        }

        pthread_t cliente_thread;
        pthread_create(&cliente_thread, NULL, manejar_cliente, (void *)&new_sock);
        pthread_detach(cliente_thread); // Hilo se libera automáticamente al terminar
    }

    pthread_exit(NULL);
}

int main() {
    int server_sock;
    struct sockaddr_in server_addr;

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TCP_PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al hacer bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) < 0) {
        perror("Error al escuchar");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    pthread_t escuchar_thread;
    pthread_create(&escuchar_thread, NULL, escuchar_conexiones, (void *)&server_sock);
    pthread_detach(escuchar_thread);

    // El hilo principal puede hacer otro trabajo, si es necesario.
    // Por ahora, simplemente espera para que el servidor no termine.
    while (1) {
        sleep(1);
    }

    close(server_sock);
    return 0;
}
