#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <mysql.h>
#include "cctv.h"
#define PORT 9002
#define BUFSIZE 1024
class mbuf {
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
};
class Pocket {
	public :
	int c_socket;
	char cctv_id[5];
	char ip[20];
	
};
const int type = 1;
