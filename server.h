#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "cctv.h"

class mbuf {
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
};
