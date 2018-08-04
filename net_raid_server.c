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
#include <sys/stat.h>
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
        printf("Server got a new msg\n");

        if (data_size <= 0){ break; }
        buf[data_size]='\0'; // endl symbol

        printf("Recieved sms size: %d\n",data_size);
        printf("Recieved sms: %s\n",buf);

        char* current_command = get_command_name(buf);
        printf("current command: %s\n",current_command);
        
        if(strcmp(current_command,"readdir")==0){
            printf("%s\n", "Server [readdir] command");
             char* current_params =get_command_param(buf);
            //printf("current param: %s\n",current_params);

            char current_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_path, "%s%s", param_direction, current_params);

            //printf("full current command: %s\n",current_path);

            DIR *d;
            d = opendir(current_path);
            int response_code;

            if(d==NULL){ // error
                response_code=-1;
                write (cfd, &response_code, sizeof(response_code));
            }else{ // OK
                response_code=0;
                write (cfd, &response_code, sizeof(response_code));

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
                   // printf("llllll>>>>>>>>>>> %s\n", d_name);
                }
                res_size-=1;
                res[res_size]='\0';

                write (cfd, &res, res_size);
            }
        }else if(strcmp(current_command,"open")==0){
            printf("%s\n", "Server [open] command");
            char* current_params =get_command_param(buf);
            //printf("current param: %s\n",current_params);
            char current [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current, "%s%s", param_direction, current_params);
            int res = open(current,0);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"getattr")==0){
            printf("%s\n", "Server [getattr] command");

            char* current_params =get_command_param(buf);
            char current_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_path, "%s%s", param_direction, current_params);

            //printf("current path%s\n", current_path);

            struct stat fileStat;
           
            int method_code = lstat(current_path,&fileStat);

            //printf("Server [getattr] response method_code: %d\n",method_code);

            write(cfd,&method_code,sizeof(method_code));

            if(method_code!=-1){ 
                //printf("Server [getattr] response OK\n");
                write (cfd, &fileStat, sizeof(fileStat));
            }
        }else if(strcmp(current_command,"rename")==0){
            printf("%s\n", "Server [rename] command");
            char* current_params =get_command_param(buf);

            char current_from_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_from_path, "%s%s", param_direction, current_params);

            char to_data[1024];
            int to_size=read(cfd,&to_data,1024);
            to_data[to_size]='\0';

            char current_to_path [strlen(param_direction)+strlen(to_data)+1];
            sprintf(current_to_path, "%s%s", param_direction, to_data);

            //printf("From:|%s|  To:|%s|\n",current_from_path,current_to_path );

            int res= rename(current_from_path,current_to_path);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"unlink")==0){
            char* path = get_command_param(buf);
            
            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);

            int res = unlink(current_path);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"rmdir")==0){
            char* path = get_command_param(buf);
            
            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);
            printf("rmdir: |%s|\n",current_path);
            
            int res = rmdir(current_path);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"mkdir")==0){
            char* path = get_command_param(buf);

            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);
            int res = mkdir(current_path,0);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"create")==0){
            char* path = get_command_param(buf);
            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);
            int mode = get_mode(buf);

            int res = creat(current_path,mode);
            write (cfd, &res, sizeof(int));
        }else if(strcmp(current_command,"opendir")==0){
            char* path = get_command_param(buf);
            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);
            printf("opendir %s\n", "here1");
            DIR *dp = opendir(current_path);
            printf("opendir %s\n", "here2");

            write (cfd, dp, sizeof(DIR*));
            printf("opendir %s\n", "here3");
        }else if(strcmp(current_command,"write")==0){
            printf("%s\n", "Server [write] command");
           // sprintf(request, "%s %zu %s %s %d", "write", size,buf,path,(int)offset);


            char* current_params =get_command_param(buf);
            char current_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_path, "%s%s", param_direction, current_params);

            int fd = open(current_path, O_WRONLY);
            write (cfd, &fd, sizeof(int));  

            if(fd!=-1){
                char write_data[1024];
                read(cfd,&write_data,1024);
                printf("Server Write Data: |%s|\n",write_data);

                size_t size;
                off_t offset;

                read(cfd,&size,sizeof(size));
                read(cfd,&offset,sizeof(offset));

                int res = pwrite(fd, (const void*)write_data, size, offset);
                write(cfd,&res,sizeof(int));
            }
 /*           int size = get_write_size(buf);
            char* buf= get_write_buf(buf,size);
            char* path= get_write_path(buf,size);
            int offset= get_write_offset(buf,size);

            char current_path [strlen(param_direction)+strlen(path)+1];
            sprintf(current_path, "%s%s", param_direction, path);
            int fd;
            int res;
            fd = open(current_path, O_WRONLY);
            if (fd == -1){
                write (cfd, &fd, sizeof(int));
            }else{
                res = pwrite(fd, buf, size, offset);
                if (res == -1){
                    write (cfd, &res, sizeof(int));
                }
                close(fd);
                int ok=0;
                write (cfd, &ok, sizeof(int));
            }*/
        }else if(strcmp(current_command,"read")==0){
            printf("%s\n", "Server [read] command");

            char* current_params =get_command_param(buf);
            char current_path [strlen(param_direction)+strlen(current_params)+1];
            sprintf(current_path, "%s%s", param_direction, current_params);

            int fd = open(current_path, O_RDONLY);
            write (cfd, &fd, sizeof(int));  

            if(fd!=-1){
                size_t size;
                off_t offset;

                read(cfd,&size,sizeof(size));
                read(cfd,&offset,sizeof(offset));
                char read_buf[1024];
                int res = pread(fd, read_buf, size, offset);

                write(cfd,&res,sizeof(int));
                if(res!=-1){
                    write(cfd,read_buf,sizeof(read_buf));
                }
            }
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
    printf("\n%s\n","Server Details");
    printf("%s\n","--------------" );
	printf("IP: %s\n",param_ip);
	printf("PORT: %s\n",param_port);
	printf("Directory: %s\n",param_direction);

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
    printf("\nStarting Server (%s:%s) Listening...\n\n",param_ip,param_port);
    while (true){
        int peer_addr_size = sizeof(struct sockaddr_in);
        cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
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
