#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include "utils.h"
#define true 1
#define BACKLOG 10


void client_handler(int cfd) {
    char buf[1024];
    int data_size;
    while (true) {
        data_size = read (cfd, &buf, 1024);
        if (data_size <= 0){
            break;
        }

        write (cfd, &buf, data_size);
        
        printf("Recieved sms: %s\n",buf );
    }
    close(cfd);
}


char* param_ip;
char* param_port;
char* param_direction;

int main(int argc, char **argv){
	if(argc!=4){
		printf("Wrong parameters.\nYou should only pass: IP, PORT and Storage Direction.\n");
		return -1;
	}

	param_ip = *(&argv[1]);
	param_port = *(&argv[2]);
	param_direction = *(&argv[3]);

	printf("%s\n",param_ip);
	printf("%s\n",param_port);
	printf("%s\n",param_direction);

	// starting server listening
 	
 	int sfd, cfd;
    struct sockaddr_in addr;
    struct sockaddr_in peer_addr;
    int port = string_to_int(param_port); // 5000 

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    listen(sfd, BACKLOG);
    printf("%s\n", "here 1");
    while (true){
    	printf("%s\n", "here 2");
        int peer_addr_size = sizeof(struct sockaddr_in);
        printf("%s\n", "here 3");
        cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
        printf("%s\n", "here 4");
        switch(fork()) {
            case -1:
                return -1; // error
            case 0:
                close(sfd);
                client_handler(cfd);
                return 0;
            default:
                close(cfd);
        }
    }
    close(sfd);
	return 0;
}