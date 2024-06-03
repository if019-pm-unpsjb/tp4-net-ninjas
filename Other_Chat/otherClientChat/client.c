#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096
#define MAX_NOMBRE 50
#define MAX_MENSAJE (BUFFER_SIZE - MAX_NOMBRE - 8)
#define MAX_FILE_NAME 255

int sockfd;
char nombre[MAX_NOMBRE];
/*
void *escuchar_servidor(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("Desconectado del servidor\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';
        printf("%s\n", buffer);

        if (strncmp(buffer, "FILE|", 5) == 0) {
            char remitente[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
            size_t file_size;
            sscanf(buffer, "FILE|%49[^|]|%255[^|]|%zu|", remitente, nombre_archivo, &file_size);

            FILE *file = fopen(nombre_archivo, "wb");
            if (file == NULL) {
                perror("Error al abrir el archivo para escritura");
                continue;
            }

            size_t bytes_totales = 0;
            size_t bytes_leidos;
            while (bytes_totales < file_size && (bytes_leidos = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
                fwrite(buffer, 1, bytes_leidos, file);
                bytes_totales += bytes_leidos;
            }

            if (bytes_totales == file_size) {
                printf("Archivo %s recibido de %s\n", nombre_archivo, remitente);
            } else {
                printf("Error al recibir el archivo %s\n", nombre_archivo);
            }

            fclose(file);
        }
    }
    pthread_exit(NULL);
}
*/
/*
void enviar_archivo(const char *destinatario) {
    char nombre_archivo[MAX_FILE_NAME];
    printf("Ingresa el nombre del archivo: ");
    fgets(nombre_archivo, MAX_FILE_NAME, stdin);
    nombre_archivo[strcspn(nombre_archivo, "\n")] = '\0';

    FILE *file = fopen(nombre_archivo, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    // Obtener el tamaño del archivo
    struct stat st;
    if (stat(nombre_archivo, &st) != 0) {
        perror("Error al obtener el tamaño del archivo");
        fclose(file);
        return;
    }
    size_t file_size = st.st_size;

    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "FILE|%s|%s|%s|%zu|", nombre, destinatario, nombre_archivo, file_size);

    // Enviar encabezado del archivo
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error al enviar el encabezado del archivo");
        fclose(file);
        return;
    }

    // Enviar datos del archivo
    size_t nread;
    while ((nread = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (write(sockfd, buffer, nread) < 0) {
            perror("Error al enviar los datos del archivo");
            fclose(file);
            return;
        }
    }

    printf("Archivo %s enviado a %s\n", nombre_archivo, destinatario);
    fclose(file);
}
*/
/*
void *escuchar_servidor(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("Desconectado del servidor\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';

        if (strncmp(buffer, "FILE|", 5) == 0) {
            char remitente[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
            size_t file_size;
            sscanf(buffer, "FILE|%49[^|]|%255[^|]|%zu|", remitente, nombre_archivo, &file_size);

            FILE *file = fopen(nombre_archivo, "wb");
            if (file == NULL) {
                perror("Error al abrir el archivo para escritura");
                continue;
            }

            size_t bytes_totales = 0;
            size_t bytes_leidos;
            while (bytes_totales < file_size && (bytes_leidos = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
                fwrite(buffer, 1, bytes_leidos, file);
                bytes_totales += bytes_leidos;
            }

            if (bytes_totales == file_size) {
                printf("Archivo %s recibido de %s\n", nombre_archivo, remitente);
            } else {
                printf("Error al recibir el archivo %s\n", nombre_archivo);
            }

            fclose(file);
        } else {
            printf("%s\n", buffer);  // Mostrar otros mensajes en la consola
        }
    }
    pthread_exit(NULL);
}
*/

void *escuchar_servidor(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int n = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (n <= 0) {
            printf("Desconectado del servidor\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        buffer[n] = '\0';

        if (strncmp(buffer, "FILE|", 5) == 0) {
            char remitente[MAX_NOMBRE], destinatario[MAX_NOMBRE], nombre_archivo[MAX_FILE_NAME];
            size_t file_size;
            sscanf(buffer, "FILE|%49[^|]|%49[^|]|%255[^|]|%zu|", remitente, destinatario, nombre_archivo, &file_size);

            printf("Recibiendo archivo %s de %s para %s (tamaño: %zu bytes)\n", nombre_archivo, remitente, destinatario, file_size);

            FILE *file = fopen(nombre_archivo, "wb");
            if (file == NULL) {
                perror("Error al abrir el archivo para escritura");
                continue;
            }

            size_t bytes_totales = 0;
            size_t bytes_leidos;
            while (bytes_totales < file_size && (bytes_leidos = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
                fwrite(buffer, 1, bytes_leidos, file);
                bytes_totales += bytes_leidos;
            }

            if (bytes_totales == file_size) {
                printf("Archivo %s recibido de %s\n", nombre_archivo, remitente);
            } else {
                printf("Error al recibir el archivo %s\n", nombre_archivo);
            }

            fclose(file);
        } else {
            printf("Esta saliendo del IFFFFFF");  // Mostrar otros mensajes en la consola
        }
    }
    pthread_exit(NULL);
}


void enviar_archivo(const char *destinatario) {
    char nombre_archivo[MAX_FILE_NAME];
    printf("Ingresa el nombre del archivo: ");
    fgets(nombre_archivo, MAX_FILE_NAME, stdin);
    nombre_archivo[strcspn(nombre_archivo, "\n")] = '\0';

    FILE *file = fopen(nombre_archivo, "rb");
    if (file == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    // Obtener el tamaño del archivo
    struct stat st;
    if (stat(nombre_archivo, &st) != 0) {
        perror("Error al obtener el tamaño del archivo");
        fclose(file);
        return;
    }
    size_t file_size = st.st_size;

    char header_buffer[BUFFER_SIZE];
    snprintf(header_buffer, BUFFER_SIZE, "FILE|%s|%s|%s|%zu|", nombre, destinatario, nombre_archivo, file_size);

    // Enviar encabezado del archivo
    if (write(sockfd, header_buffer, strlen(header_buffer)) < 0) {
        perror("Error al enviar el encabezado del archivo");
        fclose(file);
        return;
    }

    // Enviar datos del archivo
    char data_buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(data_buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (write(sockfd, data_buffer, bytes_read) < 0) {
            perror("Error al enviar los datos del archivo");
            fclose(file);
            return;
        }
    }

    printf("Archivo %s enviado a %s\n", nombre_archivo, destinatario);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <direccion_servidor> <puerto_servidor>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *direccion_servidor = argv[1];
    int puerto_servidor = atoi(argv[2]);

    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(puerto_servidor);

    if (inet_pton(AF_INET, direccion_servidor, &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar con el servidor");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Conectado al servidor %s:%d\n", direccion_servidor, puerto_servidor);

    printf("Ingresa tu nombre: ");
    fgets(nombre, MAX_NOMBRE, stdin);
    nombre[strcspn(nombre, "\n")] = '\0';

    if (write(sockfd, nombre, strlen(nombre)) < 0) {
        perror("Error al enviar el nombre");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    pthread_t escuchar_thread;
    pthread_create(&escuchar_thread, NULL, escuchar_servidor, NULL);
    pthread_detach(escuchar_thread);

    char destinatario[MAX_NOMBRE];
    char mensaje[MAX_MENSAJE];
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("Ingresa el destinatario: ");
        fgets(destinatario, MAX_NOMBRE, stdin);
        destinatario[strcspn(destinatario, "\n")] = '\0';

        printf("Elige una opción:\n1. Enviar mensaje\n2. Enviar archivo\nOpción: ");
        int opcion;
        scanf("%d", &opcion);
        getchar(); // Limpiar el salto de línea del buffer

        if (opcion == 1) {
            printf("Ingresa el mensaje: ");
            fgets(mensaje, MAX_MENSAJE, stdin);
            mensaje[strcspn(mensaje, "\n")] = '\0';

            // Asegurar que el buffer no se desborde
        if (strlen(nombre) + strlen(destinatario) + strlen(mensaje) + 8 < BUFFER_SIZE) {
            snprintf(buffer, BUFFER_SIZE, "CHAT|%s|%s|", nombre, destinatario);
            strncat(buffer, mensaje, BUFFER_SIZE - strlen(buffer) - 1);
            if (write(sockfd, buffer, strlen(buffer)) < 0) {
                perror("Error al enviar el mensaje");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        } else {
            fprintf(stderr, "Error: El mensaje es demasiado largo.\n");
        }
        } else if (opcion == 2) {
            enviar_archivo(destinatario);
        } else {
            printf("Opción no válida\n");
        }
    }

    close(sockfd);
    return 0;
}
