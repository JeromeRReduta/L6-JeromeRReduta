#include <netdb.h> 
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

// TODO Since this struct is used by both the server and client, perhaps we
// should move it elsewhere?
struct __attribute__((__packed__)) msg_header {
    uint64_t msg_len;
    uint16_t msg_type;
};

int main(int argc, char *argv[]) {

    if (argc != 3) {
       printf("Usage: %s hostname port\n", argv[0]);
       return 1;
    }

    char *server_hostname = argv[1];
    int port = atoi(argv[2]);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    struct hostent *server = gethostbyname(server_hostname);
    if (server == NULL) {
        fprintf(stderr, "Could not resolve host: %s\n", server_hostname);
        return 1;
    }

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *) server->h_addr);

    if (connect(
                socket_fd,
                (struct sockaddr *) &serv_addr,
                sizeof(struct sockaddr_in)) == -1) {

        perror("connect");
        return 1;
    }

    LOG("Connected to server %s:%d\n", server_hostname, port);

    printf("Welcome. Please type your message below, or press ^D to quit.\n");

    while (true) {
        printf("message> ");
        fflush(stdout);

        char buf[128] = { 0 };
        char *str = fgets(buf, 128, stdin);
        if (str == NULL) {
            LOG("%s", "Reached EOF! Quitting.\n");
            break;
        }

        /* Remove newline characters */
        strtok(buf, "\r\n");

        ssize_t bytes_written = 0;
        size_t bytes_left = sizeof(buf);
        char *write_ptr = buf;
        do {
            // TODO: (STEP 1) once you're ready to see what happens when you
            // don't write the entire message, change 'sizeof(buf)' to 1. Then
            // watch what happens on the server.
            bytes_written = write(socket_fd, write_ptr, sizeof(buf));
            if (bytes_written == -1) {
                perror("write");
                return 1;
            }

            // TODO: (STEP 2) add a sleep for 1 second here to amplify the
            // effect. This will cause only one byte to be sent at a time.
            bytes_left = bytes_left - bytes_written;
            write_ptr += bytes_written;
            LOG("Wrote %zd bytes\n", bytes_written);
        } while (bytes_left > 0);
    }

    return 0;
}
