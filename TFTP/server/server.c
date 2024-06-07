#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 69
#define BUFFER_SIZE 516
#define DATA_SIZE 512

void error(const char *msg) {
    perror(msg);
    exit(1);
}

//GET
void handle_rrq(int sockfd, struct sockaddr_in *client_addr, socklen_t client_len, char *file_name) {
    FILE *file = fopen(file_name, "rb");
    if (!file) {
        error("fopen");
    }

    char buffer[BUFFER_SIZE];
    uint16_t block_number = 0;
    size_t n;
    while (1) {
        n = fread(buffer + 4, 1, DATA_SIZE, file);
        block_number++;
        *(uint16_t *)buffer = htons(3); // DATA opcode
        *(uint16_t *)(buffer + 2) = htons(block_number);

        if (sendto(sockfd, buffer, n + 4, 0, (struct sockaddr *)client_addr, client_len) < 0) {
            error("sendto");
        }

        // Receive ACK
        ssize_t ack_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client_addr, &client_len);
        if (ack_size < 0) {
            error("recvfrom");
        }

        uint16_t ack_opcode = ntohs(*(uint16_t *)buffer);
        uint16_t ack_block_number = ntohs(*(uint16_t *)(buffer + 2));

        if (ack_opcode != 4 || ack_block_number != block_number) {
            fprintf(stderr, "Invalid ACK packet received\n");
            break;
        }

        if (n < DATA_SIZE) {
            break; // Last packet sent
        }
    }

    // Send final packet with 0 data size to indicate end of file
    *(uint16_t *)buffer = htons(3); 
    *(uint16_t *)(buffer + 2) = htons(block_number);
    if (sendto(sockfd, buffer, 4, 0, (struct sockaddr *)client_addr, client_len) < 0) {
        error("sendto");
    }
    fclose(file);
}



//PUT
void handle_wrq(int sockfd, struct sockaddr_in *client_addr, socklen_t client_len, char *file_name) {
    FILE *file = fopen(file_name, "wb");
    if (!file) {
        error("fopen");
    }

    char buffer[BUFFER_SIZE];
    uint16_t block_number = 0;
    ssize_t n;

    while (1) {
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)client_addr, &client_len);
        if (n < 0) {
            error("recvfrom");
        }

        uint16_t received_opcode = ntohs(*(uint16_t *)buffer);
        uint16_t received_block_number = ntohs(*(uint16_t *)(buffer + 2));

        if (received_opcode == 3 && n == 4) { // Check for final packet with 0 data size
            break;
        }

        if (received_opcode != 3 || received_block_number != block_number + 1) {
            fprintf(stderr, "Invalid data packet received\n");
            continue;
        }

        fwrite(buffer + 4, 1, n - 4, file);

        block_number++;

        // Send ACK
        *(uint16_t *)buffer = htons(4); 
        *(uint16_t *)(buffer + 2) = htons(received_block_number);
        if (sendto(sockfd, buffer, 4, 0, (struct sockaddr *)client_addr, client_len) < 0) {
            error("sendto");
        }

        if (n < DATA_SIZE + 4) {
            break; // Last packet received
        }
    }

    fclose(file);
}


void handle_request(int sockfd, struct sockaddr_in *client_addr, socklen_t client_len, char *buffer) {
    char file_name[BUFFER_SIZE];
    int opcode = ntohs(*(uint16_t *)buffer);

    if (opcode != 1 && opcode != 2) { // Handle RRQ (Read Request) and WRQ (Write Request)
        fprintf(stderr, "Unsupported request\n");
        return;
    }

    strcpy(file_name, buffer + 2); // Extract file name from the request

    if (opcode == 1) {
        handle_rrq(sockfd, client_addr, client_len, file_name);
    } else
     if (opcode == 2) {
        handle_wrq(sockfd, client_addr, client_len, file_name);
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("bind");
    }

    printf("TFTP server is running on port %d...\n", PORT);

    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            error("recvfrom");
        }
        handle_request(sockfd, &client_addr, client_len, buffer);
    }

    close(sockfd);
    return 0;
}

