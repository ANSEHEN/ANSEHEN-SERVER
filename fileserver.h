#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#define PORT 9002
class mbuf {
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
};
const int type = 1;
