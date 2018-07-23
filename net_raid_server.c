#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include "utils.h"
#include <fcntl.h>
#include <dirent.h>
#define true 1
#define BACKLOG 10

char* param_ip;
char* param_port;
char* param_direction;

struct dirent *readdir(DIR *dirp);

void client_handler(int cfd) {
    char buf[1024];
    int data_size;
    while (true) {
        data_size = read (cfd, &buf, 1024);
        if (data_size <= 0){
            break;
        }
        buf[data_size]='\0'; // endl symbol

        printf("Recieved sms size: %d\n",data_size);
        printf("Recieved sms: %s\n",buf);

        char* current_command = get_command_name(buf);
        printf("current command: %s\n",current_command);
       

        if(strcmp(current_command,"readdir")==0){
            printf("%s\n", "Server Readdir command");
             char* current_params =get_command_param(buf);
            //printf("current param: %s\n",current_params);

            char current_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_path, "%s%s", param_direction, current_params);

            printf("full current command: %s\n",current_path );
            // TODO readdir impl.

            DIR *d;
            d = opendir(current_path);
            char res[1024];
            int res_size=0;

            while(true){
                struct dirent * entry;
                const char * d_name;
                entry =readdir(d);
                if(! entry){ break; }

                d_name =entry->d_name;
                strcpy(res+res_size,d_name);
                res_size+=strlen(d_name);
                res[res_size]=' ';
                res_size+=1;
                printf("llllll>>>>>>>>>>> %s\n", d_name);
            }
            res_size-=1;
            res[res_size]='\0';
            printf("finaaaaaaaaaal whole stirng: %s\n",res);
            printf("finaaaaaaaaaal |%s| %d\n",res,res_size);

            printf("Server sent response: %s\n", res);
            write (cfd, &res, res_size);
        }else if(strcmp(current_command,"open")==0){
            printf("%s\n", "Server OPEN command");
            char* current_params =get_command_param(buf);
            //printf("current param: %s\n",current_params);
            char current [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current, "%s%s", param_direction, current_params);
            int res = open(current,0);
            write (cfd, &res, sizeof(int));
        }
    }
    close(cfd);
}




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