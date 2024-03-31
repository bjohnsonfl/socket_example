#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <stdlib.h>

#define ONE_SECOND 1000
#define TIMEOUT 10 * ONE_SECOND
#define TO_STR(STR) #STR

// input: *sfd: pointer to socket fild descriptor 
// output: error code, 0 pass, -1 fail
int generate_socket(int* sfd);

// input: sfd: socket file descriptor, addr: IP address, port: IP port
// output: error code, 0 pass, -1 fail
int connect_socket(int sfd, char* addr, uint16_t port);

// input: sfd: socket file descriptor
// output: return code
int comms(int sfd);

int generate_socket(int* sfd){
    *sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sfd == -1){
        printf("socket() failed with rc: %d\n", *sfd);
        return -1;
    }
    else {
        printf("Socket created\n"
               "Socket file descriptor: %d\n"
               , *sfd);
    }
    return 0;
}


int connect_socket(int sfd, char* addr, uint16_t port){
    struct sockaddr_in sock_addr;
    int rc = 0;

    // convert host values to network values
    uint16_t ip_port = htons(port);
    in_addr_t ip_addr = inet_addr(addr);
    printf("inet_addr(%s) returned %d\n", addr, ip_addr);
    if (ip_addr == -1) return -1;

    // configure address for socket
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = ip_port;
    sock_addr.sin_addr.s_addr = ip_addr;

    rc = connect(sfd, (struct sockaddr *) &sock_addr, sizeof (sock_addr));
    if (rc == -1){
        printf("connect() failed with rc: %d\n", rc);
        close(sfd);
        return -1;
    }
    else{
        printf("connect() succeeded\n");
    }
    return 0;
}

int comms(int sfd){
    char buff_loc [100];
    char buff_net [100];
    int matches = 0, len = 0;
    bool quit = false;
    int ready = 0;
    int numOfFiles = 2;
    int revent = 0;
    bool err = false;
    int count = 0;

    printf(" Send: \"\\quit\" to break connection\n");

    //int poll(struct pollfd fds[], nfds_t nfds, int timeout);
    struct pollfd* pfds;
    pfds = calloc(numOfFiles, sizeof(struct pollfd));

    // Events for stdin stream. All input from user
    pfds[0].fd = STDIN_FILENO;
    pfds[0].events = POLLIN;
    // Events for socket sfd. All input from socket
    pfds[1].fd = sfd;
    pfds[1].events = POLLIN;
    
    while(true){
        printf("\n client [%08d] > ", count++);
        fflush(stdout);
        ready = poll(pfds, numOfFiles, TIMEOUT);

        if(ready < 0){
            printf(" poll() returned with rc: %d", ready);
            return -1;
        }
        else if(ready == 0) printf(" poll() timeout\n");
        else{
            for(int i=0; i<numOfFiles; i++){
                if(pfds[i].revents & (POLLERR | POLLHUP | POLLNVAL)){
                    printf("\n Revent Error, FD:%d\n", pfds[i].fd);
                    #define REVENTS_ERR(ERROR) \
                        if (pfds[i].revents & ERROR) printf(" Error: %s\n", TO_STR(#ERROR))
                    REVENTS_ERR(POLLERR);
                    REVENTS_ERR(POLLHUP);
                    REVENTS_ERR(POLLNVAL);
                    return -1;
                }
                
                if(pfds[i].fd == STDIN_FILENO){
                    if(pfds[i].revents && POLLIN){
                        pfds[i].revents ^= POLLIN;
                        char* buff = fgets(buff_loc, 99, stdin);
                        if (buff != NULL){
                            if (strcmp(buff_loc,"\\quit\n") == 0) return 0;

                             printf(" Msg: %s\n", buff_loc);
                             printf(" Number of Bytes: %lu\n", strlen(buff_loc));
                             printf(" Sending...\n");
                             len = send(sfd, buff_loc, strlen(buff_loc), MSG_NOSIGNAL);
                             if (len == -1){
                                printf(" send() failed with rc: %d\n", len);
                                return -1;
                             }
                             else{
                                printf(" send() sent %d bytes\n", len);
                             }
                        }
                    }
                }
                else if(pfds[i].fd == sfd){
                    if(pfds[i].revents && POLLIN){
                        pfds[i].revents ^= POLLIN;
                        len = recv(sfd, buff_net, 100, 0);
                        printf(" \nMessage recieved...\n");
                        printf(" Msg len: %d\n", len);
                        printf(" Msg: %s", buff_net);
                        bzero(buff_net, len);
                    }
                }
                else{
                    printf("\n File Descriptor: %d, revent: %x \n", pfds[i].fd, pfds[i].revents);
                }
            }
        }

    }
    return 0;
}


int main(){
    printf("Client Main\n");
    int rc = 0;
    int sfd = 0;
    int port = 8080;
    char* addr = "127.0.0.1";

    printf("\nCreating Socket...\n");
    if((rc = generate_socket(&sfd))) return rc;

    printf("\nConnect Socket...\n");
    if((rc = connect_socket(sfd, addr, port))) return rc;

    printf("\nStarting Comms...\n");
    if((rc = comms(sfd))) return rc;

    printf("\nClosing Socket...\n");
    close(sfd);

    return 0;
}