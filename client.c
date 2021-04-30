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

ssize_t write_len(int fd, void *buf, size_t length)
{

    size_t total_written = 0;

    while (total_written < length) {
        ssize_t bytes_written = write(fd, buf + total_written, length - total_written);

        if(bytes_written == -1) {
            perror("read");
            return -1;

        }   
        else if (bytes_written == 0) {
            return total_written;
        }

        total_written += bytes_written;
    }
    return total_written;

}


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

    char *buf = NULL;
    size_t buf_sz;

    while (true) {
        printf("message> ");
        fflush(stdout); // Essential to make the things we want to print print out exactly

        ssize_t bytes_read = getline(&buf, &buf_sz, stdin); // note: we don't free here - we free in server - otherwise will have dbl free
      
        if (bytes_read == -1) { // Case: getline error
            LOG("%s", "Reached EOF! Quitting.\n");
            break;
        }

        /* Remove newline characters */
        strtok(buf, "\r\n");

        struct msg_header header = {0};
        header.msg_len = strlen(buf) + 1;


        write_len(socket_fd, &header, sizeof(struct msg_header)); // send header to server
        write_len(socket_fd, buf, header.msg_len); // send message to server

    }
    return 0;
}