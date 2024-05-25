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

void send_data(int sockfd, struct sockaddr_in *client_addr, FILE *file) {
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(*client_addr);
    int block = 1;
    size_t n;

    while ((n = fread(buffer + 4, 1, BUF_SIZE - 4, file)) > 0) {
        buffer[0] = 0;
        buffer[1] = DATA;
        buffer[2] = (block >> 8) & 0xFF;
        buffer[3] = block & 0xFF;
        sendto(sockfd, buffer, n + 4, 0, (struct sockaddr *)client_addr, addr_len);

        recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)client_addr, &addr_len);
        if (buffer[1] != ACK || ((buffer[2] << 8) | buffer[3]) != block) {
            fprintf(stderr, "Error: ACK not received or incorrect block number\n");
            break;
        }

        block++;
    }
}

void receive_data(int sockfd, struct sockaddr_in *client_addr, FILE *file) {
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(*client_addr);
    int block = 0;
    int received;

    while (1) {
        received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)client_addr, &addr_len);
        if (buffer[1] == DATA) {
            int received_block = (buffer[2] << 8) | buffer[3];
            if (received_block == block + 1) {
                fwrite(buffer + 4, 1, received - 4, file);
                block++;

                buffer[0] = 0;
                buffer[1] = ACK;
                buffer[2] = (block >> 8) & 0xFF;
                buffer[3] = block & 0xFF;
                sendto(sockfd, buffer, 4, 0, (struct sockaddr *)client_addr, addr_len);

                if (received < BUF_SIZE) {
                    break;  // Last packet
                }
            }
        } else if (buffer[1] == ERROR) {
            fprintf(stderr, "Error: %s\n", buffer + 4);
            break;
        }
    }
}

void handle_rrq(int sockfd, struct sockaddr_in *client_addr, char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        return;
    }

    send_data(sockfd, client_addr, file);
    fclose(file);
}

void handle_wrq(int sockfd, struct sockaddr_in *client_addr, char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("fopen");
        return;
    }

    receive_data(sockfd, client_addr, file);
    fclose(file);
}

void handle_request(int sockfd, struct sockaddr_in *client_addr, char *buffer) {
    char filename[256];
    char mode[12];
    sscanf(buffer + 2, "%s %s", filename, mode);

    if (buffer[1] == RRQ) {
        handle_rrq(sockfd, client_addr, filename);
    } else if (buffer[1] == WRQ) {
        handle_wrq(sockfd, client_addr, filename);
    } else {
        fprintf(stderr, "Unsupported request\n");
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TFTP_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    while (1) {
        int received = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (received < 0) {
            perror("recvfrom");
            continue;
        }

        handle_request(sockfd, &client_addr, buffer);
    }

    close(sockfd);
    return 0;
}
