#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TFTP_PORT 6969
#define BUF_SIZE 516
#define MODE "octet"
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

// Declaraciones de funciones
void send_rrq(int sockfd, struct sockaddr_in *server_addr, const char *filename);
void send_wrq(int sockfd, struct sockaddr_in *server_addr, const char *filename);
void handle_data(int sockfd, struct sockaddr_in *server_addr, FILE *file);
void handle_ack(int sockfd, struct sockaddr_in *server_addr, FILE *file);

// Implementación de funciones
void send_rrq(int sockfd, struct sockaddr_in *server_addr, const char *filename) {
    char buffer[BUF_SIZE];
    int pos = 0;

    // Opcode for RRQ
    buffer[pos++] = 0;
    buffer[pos++] = RRQ;

    // Filename
    int filename_len = strlen(filename);
    memcpy(buffer + pos, filename, filename_len);
    pos += filename_len;
    buffer[pos++] = 0;

    // Mode
    int mode_len = strlen(MODE);
    memcpy(buffer + pos, MODE, mode_len);
    pos += mode_len;
    buffer[pos++] = 0;

    sendto(sockfd, buffer, pos, 0, (struct sockaddr *)server_addr, sizeof(*server_addr));
}

void send_wrq(int sockfd, struct sockaddr_in *server_addr, const char *filename) {
    char buffer[BUF_SIZE];
    int pos = 0;

    // Opcode for WRQ
    buffer[pos++] = 0;
    buffer[pos++] = WRQ;

    // Filename
    int filename_len = strlen(filename);
    memcpy(buffer + pos, filename, filename_len);
    pos += filename_len;
    buffer[pos++] = 0;

    // Mode
    int mode_len = strlen(MODE);
    memcpy(buffer + pos, MODE, mode_len);
    pos += mode_len;
    buffer[pos++] = 0;

    sendto(sockfd, buffer, pos, 0, (struct sockaddr *)server_addr, sizeof(*server_addr));
}

void handle_data(int sockfd, struct sockaddr_in *server_addr, FILE *file) {
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(*server_addr);
    int block = 1;
    int received;

    do {
        received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)server_addr, &addr_len);
        if (buffer[1] == DATA) {
            fwrite(buffer + 4, 1, received - 4, file);
            buffer[1] = ACK;
            buffer[2] = (block >> 8) & 0xFF;
            buffer[3] = block & 0xFF;
            sendto(sockfd, buffer, 4, 0, (struct sockaddr *)server_addr, addr_len);
            block++;
        } else if (buffer[1] == ERROR) {
            fprintf(stderr, "Error: %s\n", buffer + 4);
            break;
        }
    } while (received == BUF_SIZE);
}

void handle_ack(int sockfd, struct sockaddr_in *server_addr, FILE *file) {
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(*server_addr);
    int block = 0;
    int bytes_read;

    while ((bytes_read = fread(buffer + 4, 1, BUF_SIZE - 4, file)) > 0) {
        block++;
        buffer[0] = 0;
        buffer[1] = DATA;
        buffer[2] = (block >> 8) & 0xFF;
        buffer[3] = block & 0xFF;

        sendto(sockfd, buffer, bytes_read + 4, 0, (struct sockaddr *)server_addr, addr_len);

        // Wait for ACK
        int received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)server_addr, &addr_len);
        if (buffer[1] == ACK) {
            int ack_block = (buffer[2] << 8) | buffer[3];
            if (ack_block != block) {
                fprintf(stderr, "Error: ACK block number mismatch\n");
                break;
            }
        } else if (buffer[1] == ERROR) {
            fprintf(stderr, "Error: %s\n", buffer + 4);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <filename> <mode>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    const char *filename = argv[2];
    const char *mode = argv[3];

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TFTP_PORT);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    FILE *file;
    if (strcmp(mode, "read") == 0) {
        file = fopen(filename, "wb");
        if (!file) {
            perror("fopen");
            close(sockfd);
            exit(1);
        }
        send_rrq(sockfd, &server_addr, filename);
        handle_data(sockfd, &server_addr, file);
    } else if (strcmp(mode, "write") == 0) {
        file = fopen(filename, "rb");
        if (!file) {
            perror("fopen");
            close(sockfd);
            exit(1);
        }
        send_wrq(sockfd, &server_addr, filename);
        handle_ack(sockfd, &server_addr, file);
    } else {
        fprintf(stderr, "Invalid mode. Use 'read' or 'write'.\n");
        close(sockfd);
        exit(1);
    }

    fclose(file);
    close(sockfd);
    return 0;
}

