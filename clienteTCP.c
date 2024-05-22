#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define TCP_PORT 8268
#define BUFFER_SIZE 2048

void enviar_mensaje_chat(int sockfd, const char *remitente, const char *mensaje) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "CHAT|%s|%s", remitente, mensaje);
    write(sockfd, buffer, strlen(buffer));
}

void manejar_mensaje(const char *data) {
    char tipo[5];
    sscanf(data, "%4s", tipo);

    if (strcmp(tipo, "CHAT") == 0) {
        char remitente[50];
        char mensaje[BUFFER_SIZE];
        sscanf(data, "CHAT|%49[^|]|%2047[^\n]", remitente, mensaje);
        printf("Mensaje de %s: %s\n", remitente, mensaje);
    }
}

int main(int argc, char * argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    char remitente[50];
    char buffer[BUFFER_SIZE];

    printf("Ingrese su nombre: ");
    fgets(remitente, sizeof(remitente), stdin);
    remitente[strcspn(remitente, "\n")] = 0; // Eliminar el salto de lÃnea

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(TCP_PORT);
    inet_aton(argv[1], &(servaddr.sin_addr));
    

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    int max_sd = sockfd;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sockfd, &readfds);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Error en select");
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            enviar_mensaje_chat(sockfd, remitente, buffer);
        }

        if (FD_ISSET(sockfd, &readfds)) {
            int n = read(sockfd, buffer, BUFFER_SIZE);
            if (n == 0) {
                printf("Servidor desconectado\n");
                break;
            }
            buffer[n] = '\0';
            manejar_mensaje(buffer);
        }
    }

    close(sockfd);
    return 0;
}