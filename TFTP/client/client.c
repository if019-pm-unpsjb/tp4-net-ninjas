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
void receive_file(const char *server_ip, const char *remote_file_name, const char *local_file_path) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    FILE *file;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(PORT);

    file = fopen(local_file_path, "wb");
    if (!file) {
        error("fopen");
    }

    *(uint16_t *)buffer = htons(1); 
    strcpy(buffer + 2, remote_file_name);
    strcpy(buffer + 2 + strlen(remote_file_name) + 1, "octet");

    if (sendto(sockfd, buffer, 2 + strlen(remote_file_name) + 1 + 5, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("sendto");
    }

    uint16_t block_number = 1;
    socklen_t client_len = sizeof(client_addr);
    ssize_t n;
    while (1) {
       
        n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            error("recvfrom");
        } 
        
        uint16_t data_opcode = ntohs(*(uint16_t *)buffer);
        uint16_t data_block_number = ntohs(*(uint16_t *)(buffer + 2));

        if (data_opcode != 3 || data_block_number != block_number) {
            fprintf(stderr, "Invalid data packet received\n");
            continue;
        }

        fwrite(buffer + 4, 1, n - 4, file);

        
        *(uint16_t *)buffer = htons(4); 
        *(uint16_t *)(buffer + 2) = htons(block_number);

        if (sendto(sockfd, buffer, 4, 0, (struct sockaddr *)&server_addr, client_len) < 0) {
            error("sendto");
        }

        if (n < DATA_SIZE + 4) {
            break;
        }

        block_number++;
    }
    fclose(file);
    close(sockfd);
}



//PUT
void send_file(const char *server_ip, const char *local_file_path, const char *remote_file_name) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    FILE *file;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        error("socket");
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr.sin_port = htons(PORT);

    file = fopen(local_file_path, "rb");
    if (!file) {
        error("fopen");
    }

    *(uint16_t *)buffer = htons(2); 
    strcpy(buffer + 2, remote_file_name);
    strcpy(buffer + 2 + strlen(remote_file_name) + 1, "octet");

    if (sendto(sockfd, buffer, 2 + strlen(remote_file_name) + 1 + 5, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        error("sendto");
    }

    uint16_t block_number = 0;
    size_t n;
    socklen_t server_len = sizeof(server_addr);

    while ((n = fread(buffer + 4, 1, DATA_SIZE, file)) > 0) {
        block_number++;
        *(uint16_t *)buffer = htons(3); // Data opcode
        *(uint16_t *)(buffer + 2) = htons(block_number);

        if (sendto(sockfd, buffer, n + 4, 0, (struct sockaddr *)&server_addr, server_len) < 0) {
            error("sendto");
        }

        // Receive ACK
        ssize_t ack_size = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_len);
        if (ack_size < 0) {
            error("recvfrom");
        }

        uint16_t ack_opcode = ntohs(*(uint16_t *)buffer);
        uint16_t ack_block_number = ntohs(*(uint16_t *)(buffer + 2));

        if (ack_opcode != 4 || ack_block_number != block_number) {
            fprintf(stderr, "Invalid ACK packet received\n");
            break;
        }
    }

    if (ferror(file)) {
        error("fread");
    }

    // Send final packet with 0 data size to indicate end of file
    *(uint16_t *)buffer = htons(3);
    *(uint16_t *)(buffer + 2) = htons(++block_number); 
    if (sendto(sockfd, buffer, 4, 0, (struct sockaddr *)&server_addr, server_len) < 0) {
        error("sendto");
    }

    fclose(file);
    close(sockfd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <server_ip> <operation> <local_file_path> <remote_file_name>\n", argv[0]);
        fprintf(stderr, "operation: 'get' to download file, 'put' to upload file\n");
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    const char *operation = argv[2];
    const char *local_file_path = argv[3];
    const char *remote_file_name = argv[4];

    if (strcmp(operation, "put") == 0) {
      
        send_file(server_ip, local_file_path, remote_file_name);
    } else if (strcmp(operation, "get") == 0) {
       
        receive_file(server_ip, remote_file_name, local_file_path);
    }  else {
        fprintf(stderr, "Invalid operation. Use 'get' or 'put'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

