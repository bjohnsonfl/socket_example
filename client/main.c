#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// input: *sfd: pointer to socket fild descriptor 
// output: error code, 0 pass, -1 fail
int generate_socket(int* sfd);

// input: sfd: socket file descriptor, addr: IP address, port: IP port
// output: error code, 0 pass, -1 fail
int bind_socket(int sfd, char* addr, uint16_t port);

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


int bind_socket(int sfd, char* addr, uint16_t port){
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

    rc = bind(sfd, (struct sockaddr *) &sock_addr, sizeof (sock_addr));
    if (rc == -1){
        printf("bind() failed with rc: %d\n", rc);
        return -1;
    }
    else{
        printf("bind() succeeded\n");
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

    printf("\nBinding Socket...\n");
    if((rc = bind_socket(sfd, addr, port))) return rc;

    return 0;
}