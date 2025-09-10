#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        int bytes_read = read(sock, buffer, 1024);
        if (bytes_read <= 0) {
            break;
        }
        buffer[bytes_read] = '\0';
        printf("%s", buffer);

        if (strstr(buffer, "Enter") != NULL || 
            strstr(buffer, "choice") != NULL ||
            strstr(buffer, "Enter Your Choice") != NULL) {
            memset(buffer, 0, 1024);
            fgets(buffer, 1024, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            send(sock, buffer, strlen(buffer), 0);
        }

        if (strstr(buffer, "EXIT") != NULL) {
            break;
        }
        
        memset(buffer, 0, 1024);
    }
    
    close(sock);
    return 0;
}