#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// input: *sfd: pointer to socket fild descriptor 
// output: error code, 0 pass, -1 fail
int generate_socket(int* sfd);

//input: sfd: socket file descriptor
int bind_socket(int sfd);

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


int bind_socket(int sfd, in_addr_t addr, uint16_t port){

}



int main(){
    printf("Client Main\n");
    int rc = 0;
    int sfd = 0;

    printf("\nCreating Socket...\n");
    if((rc = generate_socket(&sfd))) return rc;

    printf("\nBinding Socket...\n");
    if((rc = bind_socket(*sfd, "127.0.0.1", 8080))) return rc;

    return 0;
}