#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Client {
   char* error_log;
   int cache_size;
   char* cache_replacment;
   int timeout;
};

struct Server{
	char* ip;
	char* port;
};
