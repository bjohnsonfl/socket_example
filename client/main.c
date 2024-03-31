#include <arpa/inet.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ONE_SECOND 1000
#define TIMEOUT 10 * ONE_SECOND
#define TO_STR(STR) #STR

// input: *sfd: pointer to socket fild descriptor
// output: error code, 0 pass, -1 fail
int generate_socket(int *sfd);

// input: sfd: socket file descriptor, addr: IP address, port: IP port
// output: error code, 0 pass, -1 fail
int connect_socket(int sfd, char *addr, uint16_t port);

// input: sfd: socket file descriptor
// output: error code, 0 pass, -1 fail
int comms(int sfd);

// input: sfd: socket file descriptor, buff: msg to send, len: length of msg
// output: error code, 0 pass, -1 fail
int send_socket(int sfd, char *buff, size_t len);

// input: sfd: socket file descriptor
// output: error code, 0 pass, -1 fail
int recv_socket(int sfd);

int generate_socket(int *sfd) {
    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sfd == -1) {
        printf("socket() failed with rc: %d\n", *sfd);
        return -1;
    } else {
        printf("Socket created\n"
               "Socket file descriptor: %d\n",
               *sfd);
    }
    return 0;
}

int connect_socket(int sfd, char *addr, uint16_t port) {
    struct sockaddr_in sock_addr;
    int rc = 0;

    // convert host values to network values
    uint16_t ip_port = htons(port);
    in_addr_t ip_addr = inet_addr(addr);
    printf("inet_addr(%s) returned %d\n", addr, ip_addr);
    if (ip_addr == -1)
        return -1;

    // configure address for socket
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = ip_port;
    sock_addr.sin_addr.s_addr = ip_addr;

    rc = connect(sfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
    if (rc == -1) {
        printf("connect() failed with rc: %d\n", rc);
        close(sfd);
        return -1;
    } else {
        printf("connect() succeeded\n");
    }
    return 0;
}

int send_socket(int sfd, char *buff, size_t len) {
    int rc = 0;
    if (buff == NULL)
        return -1;
    printf(" Msg: %s\n", buff);
    printf(" Number of Bytes: %lu\n", len);
    printf(" Sending...\n");
    rc = send(sfd, buff, len, MSG_NOSIGNAL);
    if (rc == -1) {
        printf(" send() failed with rc: %d\n", rc);
        return -1;
    } else {
        printf(" send() sent %d bytes\n", rc);
    }
    return 0;
}

int recv_socket(int sfd) {
    char buff[100];
    int rc = 0;

    rc = recv(sfd, buff, 100, 0);
    if (rc == -1) {
        printf(" recv() failed with rc: %d\n", rc);
        return -1;
    } else {
        printf(" \nMessage recieved...\n");
        printf(" Msg len: %d\n", rc);
        printf(" Msg: %s", buff);
    }
    return 0;
}

int comms(int sfd) {
    char buff_loc[100];
    int matches = 0, len = 0;
    int ready = 0;
    int numOfFiles = 2;
    int revent = 0;
    int count = 0;
    int rc = 0;

    printf("\n Send: \"\\quit\" to break connection\n");

    struct pollfd *pfds;
    pfds = calloc(numOfFiles, sizeof(struct pollfd));

    // Events for stdin stream. All input from user
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    // Events for socket sfd. All input from socket
    pfds[1].fd = sfd;
    pfds[1].events = POLLIN;

    while (true) {
        printf("\n client [%08d] > ", count++);
        fflush(stdout);
        ready = poll(pfds, numOfFiles, TIMEOUT);

        if (ready < 0) {
            printf(" poll() returned with rc: %d", ready);
            return -1;
        } else if (ready == 0)
            printf(" poll() timeout\n");
        else {
            for (int i = 0; i < numOfFiles; i++) {
                if (pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    printf("\n Revent Error, FD:%d\n", pfds[i].fd);
#define REVENTS_ERR(ERROR)                                                     \
    if (pfds[i].revents & ERROR)                                               \
    printf(" Error: %s\n", TO_STR(#ERROR))
                    REVENTS_ERR(POLLERR);
                    REVENTS_ERR(POLLHUP);
                    REVENTS_ERR(POLLNVAL);
                    return -1;
                }

                if (pfds[i].fd == STDIN_FILENO) {
                    if (pfds[i].revents && POLLIN) {
                        pfds[i].revents ^= POLLIN;
                        char *buff = fgets(buff_loc, 99, stdin);
                        if (buff != NULL) {
                            if (strcmp(buff_loc, "\\quit\n") == 0)
                                return 0;
                            if (send_socket(sfd, buff, strlen(buff)))
                                return -1;
                        }
                    }
                } else if (pfds[i].fd == sfd) {
                    if (pfds[i].revents && POLLIN) {
                        pfds[i].revents ^= POLLIN;
                        if (recv_socket(sfd))
                            return -1;
                    }
                } else {
                    printf("\n File Descriptor: %d, revent: %x \n", pfds[i].fd,
                           pfds[i].revents);
                }
            }
        }
    }
    return 0;
}

int main() {
    printf("Client Main\n");
    int rc = 0;
    int sfd = 0;
    int port = 8080;
    char *addr = "127.0.0.1";

    printf("\nCreating Socket...\n");
    if ((rc = generate_socket(&sfd)))
        return rc;

    printf("\nConnect Socket...\n");
    if ((rc = connect_socket(sfd, addr, port)))
        return rc;

    printf("\nStarting Comms...\n");
    if ((rc = comms(sfd)))
        return rc;

    printf("\nClosing Socket...\n");
    close(sfd);

    return 0;
}