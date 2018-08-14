#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>


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

int get_mode(char* s){
	int res=0;
	for (int i = 0; i < strlen(s); ++i){
		if(s[i]==' '){
			i+=1;
		}
		if(i==2){
			res = s[i+1]-'0';
			break;
		}
	}
	return res;
}

int get_flags(char* s){
	int res=0;
	for (int i = 0; i < strlen(s); ++i){
		if(s[i]==' '){
			i+=1;
		}
		if(i==3){
			res = s[i+1]-'0';
			break;
		}
	}
	return res;
}

int get_write_size(char* s){
	char* new_s= strdup(s);
	int i=0;
	for (i = 0; i < strlen(new_s); ++i)
	{
		if(new_s[i]==' '){
			i++;
			break;
		}
	}

	for(;i<strlen(new_s);i++){
		if(new_s[i]==' '){
			new_s[i]='\0';
			break;
		}
	}
	int size = string_to_int(&new_s[i]);

	return size;
}

char* get_write_buf(char* s,int size){
	char* new_s = strdup(s);
	int space_counter=0;
	char* res;
	for (int i = 0; i < strlen(new_s); i++){
		if(space_counter==2){
			res=strdup(&new_s[i]);
			res[size]='\0';
			break;
		}
		if(new_s[i]==' '){
			space_counter++;
		}
	}
	return res;
}

char* get_write_path(char* s, int size){
	char* new_s = strdup(s);
	int space_counter=0;
	char* res;
	for (int i = 0; i < strlen(new_s); i++){
		if(space_counter==2){
			res=strdup(&new_s[i]);
			break;
		}
		if(new_s[i]==' '){
			space_counter++;
		}
	}
	res =&res[size];
	for (int i = 0; i < strlen(res); ++i)
	{
		if(res[i]==' '){
			res[i]='\0';
			break;
		}
	}
	return res;
}


int get_write_offset(char* s, int size){
	int offset=0;
	char* new_s = strdup(s);
	int space_counter=0;
	char* res;
	for (int i = 0; i < strlen(new_s); i++){
		if(space_counter==2){
			res=strdup(&new_s[i]);
			break;
		}
		if(new_s[i]==' '){
			space_counter++;
		}
	}
	res =&res[size];
	for (int i = 0; i < strlen(res); ++i)
	{
		if(res[i]==' '){
			res[i]='\0';
			offset= string_to_int(&res[i+1]);
			break;
		}
	}
	return offset;
}


char* get_read_buff(char* s){
	char* new_s= strdup(s);
	int spaces=0;
	char* res;
	for (int i = 0; i < strlen(new_s); i++)
	{
		if(new_s[i]==' '){
			spaces++;
		}
		if(spaces==2){
			res=strdup(&s[i+1]);
			break;
		}
	}
	return res;
}
char* get_sub_string(const char* s, int start,int end){
	char * new_s= strdup(s);
	new_s[end+1]='\0';
	return &new_s[start];
}

char* two_strings_xor(char* s1,char* s2){
	char* res= NULL;
	if(strlen(s1)>strlen(s2)){
		res =strdup(s1);
	}else{
		res=strdup(s2);
	}

	for (int i = 0; i < strlen(res);i++){
		if(i<=strlen(s1) && i<strlen(s2)){
			char c1=s1[i];
			char c2 = s2[i];
			res[i]=c1^c2;
		}
	}
	return res;
}


char* XOR(char* arg1, ...){
  va_list ap;
  char* i;
  char* res="";
  va_start(ap, arg1); 
  for (i = arg1; i !=NULL; i = va_arg(ap, char*)){
  	res = two_strings_xor(res,i);
	}
  
  va_end(ap);
  return res;
}





