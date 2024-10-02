#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT "8080"
#define MAX_MESSAGE_SIZE 1000

#define WARN_FALSE(expected, obtained, f_msg, s_msg) {      \
    if((expected) != (obtained)) {                          \
        printf("[!] %s.\n", f_msg);                         \
    } else {                                                \
        printf("[+] %s.\n", s_msg);                         \
    }                                                       \
}

#define WARN_TRUE(expected, obtained, f_msg, s_msg) {       \
    if((expected) == (obtained)) {                          \
        printf("[!] %s.\n", f_msg);                         \
    } else {                                                \
        printf("[+] %s.\n", s_msg);                         \
    }                                                       \
}

#define EXIT_FALSE(expected, obtained, f_msg, s_msg) {      \
    if((expected) != (obtained)) {                          \
        printf("[E] %s.\n", f_msg);                         \
        exit(EXIT_FAILURE);                                 \
    } else {                                                \
        printf("[+] %s.\n", s_msg);                         \
    }                                                       \
}

#define EXIT_TRUE(expected, obtained, f_msg, s_msg) {       \
    if((expected) == (obtained)) {                          \
        printf("[!] %s.\n", f_msg);                         \
        exit(EXIT_FAILURE);                                 \
    } else {                                                \
        printf("[+] %s.\n", s_msg);                         \
    }                                                       \
}

int main(int argc, char **argv){
    char message[MAX_MESSAGE_SIZE];

    int stat, sock_fd, connect_stat;
    int bytes_recieved, bytes_sent;

    struct addrinfo hints, *result, *temp;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    stat = getaddrinfo("127.0.0.1", PORT, &hints, &result);
    EXIT_FALSE(
        0,
        stat,
        gai_strerror(stat),
        "Address translated"
    );

    for(temp = result; temp != NULL; temp = temp->ai_next) {
        sock_fd = socket(
            temp->ai_family,
            temp->ai_socktype,
            temp->ai_protocol
        );

        WARN_TRUE(
            1,
            sock_fd < 1,
            "Socket creation failed. Retrying..",
            "Socket created successfully"
        );
        if(sock_fd < 0) continue;
        break;
    }

    EXIT_TRUE(
        1,
        temp == NULL,
        "Failed to connect at any port",
        "One step ahead"
    );
    connect_stat = connect(sock_fd, temp->ai_addr, temp->ai_addrlen);

    EXIT_TRUE(
        -1,
        connect_stat,
        "Failed to connect to server",
        "Connection established successfully"
    );

    while(1) {
        memset(message, 0, sizeof message);
        printf("*waiting*");
        fflush(stdout);
        bytes_recieved = recv(sock_fd, message, MAX_MESSAGE_SIZE, 0);
        if(bytes_recieved == 0) {
            break;
        }

        message[bytes_recieved] = '\0';
        if(strcmp(message, "end") == 0) {
            break;
        }

        printf("\r\033[1;33mServer\033[0m: %s\n", message);
        fflush(stdout);
        
        printf("\033[1;34mYou\033[0m: ");
        fgets(message, MAX_MESSAGE_SIZE, stdin);
        message[strlen(message) - 1] = '\0';

        bytes_sent = send(sock_fd, message, strlen(message), 0);
        if(bytes_sent != strlen(message)) {
            printf("Failed to send message completely.");
        }

        if(strcmp("end", message) == 0) {
            break;
        }
    }

    close(sock_fd);
}
