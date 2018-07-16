#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include  <sys/types.h>
#include <unistd.h>
#include "client.h"
#include "utils.h"
#include "logger.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#define true 1

char* diskname;
char* mount_point;
int raid;
char* hotswap_ip;	
char* hotswap_port;

int num_servers=0;
int servers_sfd [10];
struct Server servers[10];
struct Client client;


int main(int argc, char **argv){

	/*printf("%d\n", socket(AF_INET, SOCK_STREAM, 0));
	printf("%d\n", socket(AF_INET, SOCK_STREAM, 0));
	return 0;*/


	/*logger_init("log.txt");
	log_message("1");
	log_message("message 2");
	log_message("random message for now :)))");
	logger_deinit();
	return 0;*/

 /*	int sfd;
    struct sockaddr_in addr;
    int ip;
    char buf[1024];
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    inet_pton(AF_INET, "127.0.0.1", &ip);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(10001);
    addr.sin_addr.s_addr = ip;

    connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    printf("%s\n", "woke up");
    write(sfd, "qwe", 3);
    read(sfd, &buf, 3);
    printf("This is client: response%s\n", buf);
    sleep(600);
    close(sfd);

    return 0;*/

	if(argc!=2){
		printf("Wrong parameters.\nYou should only pass: Configuration File Direction\n");
		exit(0);
	}

	char* file_name = *(&argv[1]);

	FILE * fp = fopen(file_name, "r");
	if (fp == NULL){
		printf("Can not open the %s file.\n",file_name);
		return 0;
	}

	char buff[255];
	while(true){

		int is_end=fscanf(fp, "%s", buff);
		if(is_end==-1){
			break;
		}

		if(strcmp(buff, "errorlog")==0){
			fscanf(fp, "%s", buff); // for this symbol '='
			fscanf(fp, "%s", buff); // reads the value
			client.error_log=strdup(buff);
			logger_init(client.error_log);
			//printf("our errorlog is:%s\n",buff);
		}else if(strcmp(buff, "cache_size")==0){
			fscanf(fp, "%s", buff);
			fscanf(fp, "%s", buff);
			client.cache_size=cach_size_to_int(buff);
			//printf("our cache_size is:%s\n",buff );
		}else if(strcmp(buff, "cache_replacment")==0){
			fscanf(fp, "%s", buff);
			fscanf(fp, "%s", buff);
			client.cache_replacment=strdup(buff);
			//printf("our cache_replacment is:%s\n",buff );
		}else if(strcmp(buff, "timeout")==0){
			fscanf(fp, "%s", buff);
			fscanf(fp, "%s", buff);
			client.timeout=string_to_int(buff);
			//printf("our timeout is:%s\n",buff );
		}else if(strcmp(buff,"diskname")==0){
			fscanf(fp, "%s", buff); // '='
			fscanf(fp, "%s", buff); // value
			diskname=strdup(buff);
		}else if(strcmp(buff,"mountpoint")==0){
			fscanf(fp, "%s", buff); 
			fscanf(fp, "%s", buff);
			mount_point=strdup(buff);
		}else if(strcmp(buff,"raid")==0){
			fscanf(fp, "%s", buff); 
			fscanf(fp, "%s", buff);
			raid=string_to_int(buff);
		}else if (strcmp(buff,"servers")==0){
			fscanf(fp, "%s", buff); 
			fscanf(fp, "%s", buff);
			ip_port_dot_remove(buff);   // the first server config is here and other are in [else]
			servers[num_servers].port = get_port(buff);
			servers[num_servers].ip = get_ip(buff);
			num_servers++;
		}else if(strcmp(buff,"hotswap")==0){
			fscanf(fp, "%s", buff); 
			fscanf(fp, "%s", buff);  
			servers[num_servers].port = get_port(buff);// the last element is hotswap in [servers]
			servers[num_servers].ip = get_ip(buff);
			num_servers++;

			if(0==fork()){
				//child
				printf("%s start %d\n", "___________________",getpid());
				// FOR loop tries connecting to server
				for (int i = 0; i < num_servers; i++){
					/*char msg [strlen(diskname)+strlen(servers[i].port)+strlen(servers[i].ip)+3];
					sprintf(msg, "%s %s %s", diskname, servers[i].ip, servers[i].port);

					log_message(msg);*/

					/*log_message(diskname);
					log_message(servers[i].ip);
					log_message(servers[i].port);*/
					//printf("%s\n", msg);

					
				    struct sockaddr_in addr;
				    int ip;
				   // char buf[1024];
				    int sfd = socket(AF_INET, SOCK_STREAM, 0);
				    inet_pton(AF_INET, servers[i].ip, &ip);

				    addr.sin_family = AF_INET;
				    addr.sin_port = htons(string_to_int(servers[i].port));
				    addr.sin_addr.s_addr = ip;

				    servers_sfd[i]=sfd; // save in array

				    int is_connected = connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));

				    if(is_connected==0){ // successful connection
				    	if(i==num_servers-1){// hotswap server
				    		char msg [strlen(diskname)+strlen(servers[i].port)+strlen(servers[i].ip)+28];// include endln symbol
							sprintf(msg, "%s %s:%s hotswap server connected", diskname, servers[i].ip, servers[i].port);
							log_message(msg);
				    	}else{
				    		char msg [strlen(diskname)+strlen(servers[i].port)+strlen(servers[i].ip)+19];
							sprintf(msg, "%s %s:%s open connection", diskname, servers[i].ip, servers[i].port); 
							log_message(msg);
				    	}
				    }else{ // connection failed
				    	if(i==num_servers-1){
							char msg [strlen(diskname)+strlen(servers[i].port)+strlen(servers[i].ip)+36];
							sprintf(msg, "%s %s:%s hotswap server connection failed", diskname, servers[i].ip, servers[i].port);
							log_message(msg);
				    	}else{
				    		char msg [strlen(diskname)+strlen(servers[i].port)+strlen(servers[i].ip)+21];
							sprintf(msg, "%s %s:%s connection failed", diskname, servers[i].ip, servers[i].port);
							log_message(msg);
				    	}
				    }
				}


				// TODO mountpoints


				printf("%s end %d\n", "___________________",getpid());
				break;	
			}else{
				//parent
				printf("%s parent %d\n", "___________________",getpid());
				num_servers=0;
			}
		}else{
			ip_port_dot_remove(buff);
			servers[num_servers].port = get_port(buff);
			servers[num_servers].ip = get_ip(buff);
			num_servers++;
		}
	}

	//fclose(fp);

	printf("%s %d\n", "\n--------finished PARSING-----------",getpid());
	/*printf("%s\n", client.error_log);
	printf("%d\n", client.cache_size);
	printf("%s\n", client.cache_replacment);
	printf("%d\n", client.timeout);*/

	return 0;
}