#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


char* file_name=NULL;
FILE *f=NULL;

int logger_init(char* s){
	file_name= strdup(s);
	f=fopen(file_name,"w");
	if(f==NULL){
		printf("%s\n", "Can not open the file");
		return -1;
	}
	return 0;
}


void log_message(char* msg){
	char* current_time_str = get_time();

	char res[strlen(msg)+strlen(current_time_str)+1];

	for (int i = 0; i < strlen(current_time_str); ++i)
	{
		res[i]=current_time_str[i];
	}

	res[strlen(current_time_str)]=' ';

	for (int i = 0; i < strlen(msg); ++i)
	{
		res[i+strlen(current_time_str)+1]=msg[i];
	}

	res[strlen(msg)+strlen(current_time_str)+1]='\0';
	//printf("%s\n", res);
	fprintf(f, "%s\n",strdup(res));
}

void logger_deinit(){
	fclose(f);
	free(file_name);
}
