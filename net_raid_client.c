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
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <dirent.h>
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

//-----------------------------------FUSE---------------------------------------------
// TODO
// write() return (-1) if connection is not 

static int my_getattr(const char *path, struct stat *stbuf){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "getattr",path);
	memset(stbuf, 0, sizeof(struct stat));

	char request [strlen("getattr")+strlen(path)+2];
	sprintf(request, "%s %s", "getattr", path);

	int request_status_code= write(servers_sfd[0], request, strlen(request));

	if(request_status_code!=-1){

		int response_code;
		read(servers_sfd[0],&response_code,sizeof(response_code));

		if(response_code==-1){ // Error
			return -ENOENT;
		}else{ // OK
			read(servers_sfd[0],stbuf,sizeof(struct stat));
			//printf("%s\n", "Client getattr OK response");
			return 0;
		}
	}else{
		printf("getattr could not sent data to server \n");
	}
	return -ENOENT;

/*	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, hello_path) == 0) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = strlen(hello_str);
	} else{
		res = -ENOENT;
	}

	return res;*/
}

static int my_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n", getpid(),diskname, "readdir",path);

	(void) offset;
	(void) fi;

	char request [strlen("readdir")+strlen(path)+2];
	sprintf(request, "%s %s", "readdir", path);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if(request_status_code!=-1){

		int response_code;
		read(servers_sfd[0],&response_code,sizeof(response_code));

		if(response_code==-1){// error
			return -ENOENT;
		}else{ // OK
			char response[1024];
			int data_size = read(servers_sfd[0],&response,1024);
			response[data_size]='\0';
			const char s[2] =" ";
		   	char *token;
			token = strtok(response, s);
			while( token != NULL ) {
				//printf( "Client readdir tokens: %s\n", token );
				filler(buf,token,NULL,0);
				token = strtok(NULL, s);
			}
			return 0;
		}
	}else{
		printf("%s\n", "readdir cant send data to server");
	}
	return -ENOENT;

/*	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0){
		printf("retunred readdir %s\n","here" );
		return -ENOENT;
	}

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	filler(buf, hello_path + 1, NULL, 0);

	return 0;*/
}

static int my_open(const char *path, struct fuse_file_info *fi){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n", getpid(),diskname, "open",path);
	(void) fi;

	char request [strlen("open")+strlen(path)+2];
	sprintf(request, "%s %s", "open", path);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if(request_status_code!=-1){
		int response_code;
		read(servers_sfd[0],&response_code,sizeof(response_code));

		if(response_code==-1){
			return -ENOENT;
		}else{
			return 0;
		}
	}else{
		printf("%s\n", "open cant send data to server");
	}
	return -ENOENT;


/*	for (int i = 0; i < num_servers-1; i++){
		int request_status_code =write(servers_sfd[i], request, strlen(request));
		if(request_status_code==0){
			int res;
			read(servers_sfd[i],&res,sizeof(int));
			if (res == -1){
				return -errno;
			}
			return res;
		}
	}
	
	return -errno;*/


/*	if (strcmp(path, hello_path) != 0){
		return -ENOENT;
	}

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;*/
}


static int my_rename(const char* from, const char* to){
	printf("Process ID:%d Diskname:%s Method:%s From:%s To:%s\n",getpid(), diskname, "rename",from,to);

	char request [strlen("rename")+strlen(from)+2];
	sprintf(request, "%s %s", "rename", from);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if(request_status_code!=-1){
		write(servers_sfd[0], to, strlen(to));

		int response_code;
		read(servers_sfd[0],&response_code,sizeof(response_code));

		if(response_code==-1){
			return -ENOENT;
		}else{
			return 0;
		}
	}else{
		printf("%s\n", "rename cant send data to server");
	}
	return -ENOENT;


/*	char request [strlen("rename")+strlen(from)+strlen(to)+3];
	sprintf(request, "%s %s %s", "rename", from, to);

	for (int i = 0; i < num_servers-1; i++){
		int request_status_code= write(servers_sfd[i], request, strlen(request));
		if(request_status_code==0){
			int res;
			read(servers_sfd[i],&res,sizeof(int));
			if (res == -1){
				return -errno;
			}
		}else{
			return -errno;
		}
	}
	return 0;*/
}

static int my_unlink(const char* path){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "unlink",path);
	return 0;
	char request [strlen("unlink")+strlen(path)+2];
	sprintf(request, "%s %s", "unlink", path);
	for (int i = 0; i < num_servers-1; i++){
		int request_status_code=write(servers_sfd[i], request, strlen(request));
		if(request_status_code==0){
			int res;
			read(servers_sfd[i],&res,sizeof(int));
			if (res == -1){
				return -errno;
			}
			return res;
		}
	}
	return -errno;
}

static int my_release(const char* path, struct fuse_file_info *fi){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname,"release",path);
	(void) path;
	(void) fi;
	return 0;
}

static int my_rmdir(const char* path){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "rmdir",path);
	
	char request [strlen("rmdir")+strlen(path)+2];
	sprintf(request, "%s %s", "rmdir", path);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if(request_status_code!=-1){
		int response_code;
		read(servers_sfd[0],&response_code,sizeof(response_code));
		if(response_code==-1){
			return response_code;
		}else{
			return 0;
		}
	}else{
		printf("%s\n", "rmdir cant send data to server");
	}
	return -ENOENT;
}

static int my_mkdir(const char* path, mode_t mode){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "mkdir",path);
	return 0;
	char request [strlen("mkdir")+strlen(path)+2];
	sprintf(request, "%s %s", "mkdir", path);

	for (int i = 0; i < num_servers-1; i++){
		int request_status_code=write(servers_sfd[i], request, strlen(request));
		if(request_status_code==0){
			int res;
			read(servers_sfd[i],&res,sizeof(int));
			if (res == -1){
				return -errno;
			}
		}else{
			return -errno;
		}

	}
	return 0;
}

static int my_releasedir(const char* path, struct fuse_file_info *fi){
	printf("Process ID:%d diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "releasedir",path);
	(void) path;
	(void) fi;
	return 0;
}

static int my_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "create",path);
	(void) fi;
	return 0;
	char request [strlen("create")+strlen(path)+sizeof(mode_t)+3];
	sprintf(request, "%s %s %d", "create", path,mode);

	for (int i = 0; i < num_servers-1; i++){
		int request_status_code=write(servers_sfd[i], request, strlen(request));
		if(request_status_code==0){
			int res;
			read(servers_sfd[i],&res,sizeof(int));
			if (res == -1){
				return -errno;
			}
		}else{
			return -errno;
		}
	}	
	return 0;
}

static int my_opendir(const char* path, struct fuse_file_info* fi){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s\n",getpid(), diskname, "opendir",path);
	(void) fi;
	return 0;
	char request [strlen("opendir")+strlen(path)+2];
	sprintf(request, "%s %s", "opendir", path);
	write(servers_sfd[0], request, strlen(request));

	DIR* dp=NULL;
	read(servers_sfd[0],dp,sizeof(DIR*));
	if(dp!=NULL){
		return 0;
	}
	return -errno;
}

static int  my_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s Buf:%s\n",getpid(), diskname, "write",path,buf);
	(void) fi;

	char request [strlen("write")+strlen(path)+2];
	sprintf(request, "%s %s", "write", path);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if (request_status_code!=-1){
		int open_code;
		read(servers_sfd[0],&open_code,sizeof(open_code));

		if(open_code!=-1){ // OPEN File OK
			write(servers_sfd[0], buf, strlen(buf)); // buf
			write(servers_sfd[0], &size, sizeof(size_t)); // size
			write(servers_sfd[0], &offset, sizeof(off_t)); // offset
			int write_code;
			read(servers_sfd[0],&write_code,sizeof(write_code));
			printf("%d\n", write_code);
			if(write_code==-1){
				return  -ENOENT;
			}else{
				return write_code;
			}
		}else{
			return -ENOENT;
		}
	}else{
		printf("%s\n", "read cant send data to server");
	}

	return -ENOENT;


/*	int raid_1_used_counter=0;
	if(raid==1){
		printf("write %s\n", "raind 1");

		char request [strlen("write")+sizeof(size_t)+strlen(buf)+strlen(path)+sizeof(int)+5];
		sprintf(request, "%s %zu %s %s %d", "write", size,buf,path,(int)offset);

		for (int i = 0; i < num_servers-1; ++i){
			int request_status_code=write(servers_sfd[i], request, strlen(request));

			if (request_status_code==0){  // sent
				int res;
				read(servers_sfd[i],&res,sizeof(int));
				if(res==-1){
					return -errno;
				}
			}else{
				raid_1_used_counter++;
				if(raid_1_used_counter==2){
					return -errno;
				}
				// hotswap
				int hotswap_request_status_code=write(servers_sfd[num_servers-1], request, strlen(request));

				if (hotswap_request_status_code==0){  // sent
					char hotswap_msg [strlen(diskname)+33];
					sprintf(hotswap_msg, "%s hotswap server has been reached", diskname);
					log_message(hotswap_msg);
					
					int res;
					read(servers_sfd[num_servers-1],&res,sizeof(int));
					if(res==-1){
						return -errno;
					}
				}else {
					// hotswap is not available
					char hotswap_msg [strlen(diskname)+33];
					sprintf(hotswap_msg, "%s hotswap server is not reachable", diskname);
					log_message(hotswap_msg);
				}
			}
		}
		return 0;
	}else if(raid==5){
		printf("write %s\n", "raid 5");
	}
	return 0;*/
}

static int my_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
	printf("Process ID:%d Diskname:%s Method:%s PATH:%s Buf:%s\n",getpid(), diskname, "read",path,buf);
	(void) fi;

	char request [strlen("read")+strlen(path)+2];
	sprintf(request, "%s %s", "read", path);

	int request_status_code =write(servers_sfd[0], request, strlen(request));

	if (request_status_code!=-1){
			int open_code;
			read(servers_sfd[0],&open_code,sizeof(open_code));

			if(open_code!=-1){ // OPEN OK

				write(servers_sfd[0], &size, sizeof(size_t)); // size
				write(servers_sfd[0], &offset, sizeof(off_t)); // offset

				int read_code;
				read(servers_sfd[0],&read_code,sizeof(read_code));

				if(read_code==-1){
					return  -ENOENT;
				}else{
					char result[1024];
					int data_size = read (servers_sfd[0], &result, 1024);
					result[data_size]='\0';
					memcpy(buf,result,strlen(result));
					return strlen(result);
				}
			}else{
				return -ENOENT;
			}
	}else{
		printf("%s\n", "read cant send data to server");
	}
	return -ENOENT;

	/*return 0;
	size_t len;
	if(raid==1){
		printf("read %s\n", "raind 1");
		
	}else if(raid==5){
		printf("read %s\n", "raid 5");

	}
	return size;*/

/*	size_t len;

	if(strcmp(path, hello_path) != 0){
		return -ENOENT;
	}

	len = strlen(hello_str);
	if (offset < len) {
		if (offset + size > len){
			size = len - offset;
		}
		memcpy(buf, hello_str + offset, size);
	} else
		size = 0;

	return size;*/
}

static struct fuse_operations all_methods = {
	.getattr	= my_getattr,
	.readdir	= my_readdir,
	.open		= my_open,
	.read		= my_read,
	.write      = my_write,
	.rename     = my_rename,
	.release    = my_release,
	.releasedir = my_releasedir,
	.rmdir      = my_rmdir,
	/*.unlink     = my_unlink,
	.mkdir      = my_mkdir,
	.opendir    = my_opendir,
	.create     = my_create,*/
};
//-----------------------------------------------------------------------------------------

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

	if(argc<2){
		printf("Wrong parameters.\nYou should only pass: Configuration File Direction\n");
		return 0;
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
				
				char* new_argv[argc];

				/*for (int i = 0; i < argc; ++i)
				{
					new_argv[i]=strdup(argv[i]);
				}*/
 				new_argv[0]=argv[0];
 				new_argv[1]=strdup(mount_point);// set to argv mountpoint directory
 				if(argc==3){
 					new_argv[2]=strdup(argv[2]);//
 				}
 				if(argc==4){
 					new_argv[3]=strdup(argv[3]);//
 				}

				char msg [strlen(diskname)+strlen(mount_point)+20];
				sprintf(msg, "%s mountpointing to: %s", diskname, mount_point);
				log_message(msg);
				
				int fuse_code = fuse_main(argc, new_argv, &all_methods, NULL);   // TODO return -1 if can not mountain

				if(fuse_code!=0){
					char fuse_msg [strlen(diskname)+strlen(mount_point)+27];
					sprintf(fuse_msg, "%s mountpointing failed to: %s", diskname, mount_point);
					log_message(fuse_msg);
				}
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

	printf("%s pID:%d\n", "\n--------FINISHED-----------",getpid());
	/*printf("%s\n", client.error_log);
	printf("%d\n", client.cache_size);
	printf("%s\n", client.cache_replacment);
	printf("%d\n", client.timeout);*/

	return 0;
}