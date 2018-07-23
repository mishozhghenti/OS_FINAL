#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


int string_to_int(char* s){
	int res=0;
	for(int i=0;i<strlen(s);i++){
		res=res*10 + (s[i]-'0');
	}
	return res;
}

int cach_size_to_int(char* s){
	int res=0;
	for(int i=0;i<strlen(s);i++){
		if(s[i]>='0'&&s[i]<='9'){
			res=res*10 + (s[i]-'0');
		}
	}
	return res;
}

void ip_port_dot_remove(char* s){
	if(s[strlen(s)-1]==','){
		s[strlen(s)-1]='\0';
	}
}

char* get_ip(char* s){
	char* new_s= strdup(s);
	for (int i = 0; i < strlen(s); ++i)
	{
		if(new_s[i]==':'){
			new_s[i]='\0';
			return strdup(&new_s[0]);
		}
	}
	return NULL;
}

char* get_port(char* s){
	for (int i = 0; i < strlen(s); ++i)
	{
		if(s[i]==':'){
			return strdup(&s[i+1]);
		}
	}
	return NULL;
}


// from the documentation copied
char* get_time(){
    time_t current_time;
    char* c_time_string;

    /* Obtain current time. */
    current_time = time(NULL);

    if (current_time == ((time_t)-1))
    {
        (void) fprintf(stderr, "Failure to obtain the current time.\n");
       return NULL;
    }

    /* Convert to local time format. */
    c_time_string = ctime(&current_time);

    if (c_time_string == NULL)
    {
        (void) fprintf(stderr, "Failure to convert the current time.\n");
        return NULL;
    }

    /* Print to stdout. ctime() has already added a terminating newline character. */
    char res[2+strlen(c_time_string)];
    for (int i = 1; i < 1+strlen(c_time_string); ++i)
    {
    	res[i]=c_time_string[i-1];
    }
    res[0]='[';
    res[strlen(res)-2]=']';
    res[strlen(res)-1]='\0';

    return strdup(res);
}

char* get_command_name(char * s){
	char* new_s=strdup(s);
	for (int i = 0; i < strlen(new_s); i++)
	{
		if(new_s[i]==' '){
			new_s[i]='\0';
			break;
		}
	}
	return strdup(new_s);
}

char* get_command_param(char* s){
	int i=0;
	for(i=0;i<strlen(s);i++){
		if(s[i]==' '){
			break;
		}
	}
	char* res = strdup(&s[i+1]);
	return res;
}
