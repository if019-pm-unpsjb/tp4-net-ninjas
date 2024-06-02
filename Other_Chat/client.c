#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 4096
#define MAX_NOMBRE 50
#define MAX_MENSAJE (BUFFER_SIZE - MAX_NOMBRE - 8)

int sockfd;
char nombre[MAX_NOMBRE];

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
    }
    pthread_exit(NULL);
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
    }

    close(sockfd);
    return 0;
}