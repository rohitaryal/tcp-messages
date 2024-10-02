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

int main(int argc, char **argv) {
    char message[MAX_MESSAGE_SIZE];

    int stat, bind_stat, sock_fd;
    int listen_stat, new_fd, bytes_sent;
    int bytes_recieved;

    socklen_t client_size;
    struct sockaddr_storage client_addr;
    client_size = sizeof client_addr;

    struct addrinfo hints, *result, *temp;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    // Address translation
    stat = getaddrinfo(NULL, PORT, &hints, &result);

    EXIT_FALSE(
        0, 
        stat, 
        gai_strerror(stat), 
        "Address translated"
    );

    // Try connecting on all possible port
    for(temp = result; temp != NULL; temp = temp->ai_next) {
        // Create socket
        sock_fd = 
            socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol);

        // If socket creation failed
        WARN_TRUE(
            1,
            sock_fd < 0,
            "Socket creation failed. Retrying..",
            "Socket created succesfully"
        );
        if(sock_fd < 0) continue;

        // Bind to available socket
        bind_stat = bind(sock_fd, temp->ai_addr, temp->ai_addrlen);

        // If binding failed
        WARN_FALSE(
            0,
            bind_stat,
            "Failed to bind to socket. Retrying..",
            "Binded to socket succesfully"
        );
        if(bind_stat != 0) continue;

        // Connected to one port so no need to 
        // continue anymore
        break;
    }

    // No binding possible at any port
    EXIT_TRUE(
        1,
        temp == NULL,
        "Unable to bind on any port",
        "Connection setup done"
    );

    // Start the listener
    listen_stat = listen(sock_fd, 10);

    EXIT_FALSE(
        0,
        listen_stat,
        "Failed to start listener at port: " PORT,
        "Listener started at port: " PORT
    );

    while(1) {
        // Accept connection from client
        new_fd = 
            accept(sock_fd, (struct sockaddr *)&client_addr, &client_size);

        WARN_TRUE(
            1,
            new_fd < 0,
            "Failed to accept connection",
            "New connection established"
        );

        if(!fork()) {
            // I am a child node so continue connection
            // on new_fd for child
            while(1) {
                printf("\033[1;33mYou\033[0m: ");
                fflush(stdout);

                // Start message from server
                memset(message, 0, sizeof message);
                fgets(message, MAX_MESSAGE_SIZE, stdin);
                message[strlen(message) - 1] = '\0';

                bytes_sent = send(new_fd, message, strlen(message), 0);

                if(bytes_sent != strlen(message)) {
                    printf("Failed to send complete mesage.\n");
                }
                fflush(stdout);

                if(strcmp(message, "end") == 0) {
                    break;
                }

                memset(message, 0, sizeof message);
                printf("*waiting*");
                fflush(stdout);

                bytes_recieved = recv(new_fd, message, MAX_MESSAGE_SIZE, 0);

                // Client closed connection
                if(bytes_recieved == 0) {
                    break;
                }
                message[bytes_recieved] = '\0';

                if(strcmp(message, "end") == 0) {
                    break;
                }

                printf("\r\033[1;34mClient\033[0m: %s\n", message);
                fflush(stdout);
            }

            WARN_TRUE(1, 1, "Closing connection", "Closing Connection");
            close(sock_fd);
            exit(EXIT_SUCCESS);
        } else {
            // I am parent node so close new_fd for parent
            close(new_fd);
        }
    }
    close(sock_fd);
    freeaddrinfo(result);
}
