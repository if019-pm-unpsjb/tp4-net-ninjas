#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define TCP_PORT 8268
#define BUFFER_SIZE 2048

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

void enviar_mensaje_chat(int sockfd, const char *remitente, const char *mensaje) {
    char buffer[BUFFER_SIZE];
    snprintf(buffer, BUFFER_SIZE, "CHAT|%s|%s", remitente, mensaje);
    write(sockfd, buffer, strlen(buffer));
}

int main() {
    int server_sock, new_sock;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);

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

    new_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
    if (new_sock < 0) {
        perror("Error al aceptar conexión");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    fd_set readfds;
    int max_sd = new_sock;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(new_sock, &readfds);

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) {
            perror("Error en select");
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            enviar_mensaje_chat(new_sock, "servidor", buffer);
        }

        if (FD_ISSET(new_sock, &readfds)) {
            int n = read(new_sock, buffer, BUFFER_SIZE);
            if (n == 0) {
                printf("Cliente desconectado\n");
                close(new_sock);
                new_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
                if (new_sock < 0) {
                    perror("Error al aceptar conexión");
                    close(server_sock);
                    exit(EXIT_FAILURE);
                }
                continue;
            }
            buffer[n] = '\0';
            manejar_mensaje(buffer);
        }
    }

    close(server_sock);
    return 0;
}
