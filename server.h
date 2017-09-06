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
#include <iostream>
#include <thread>
#include "cctv.h"

#define TYPE_FILE 1
#define TYPE_BEACON 2
#define TYPE_BEACON_L 3
#define TYPE_BEACON_C 4
#define TYPE_RESULT 5
class mbuf {
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
	int result;
};

class beacon_data{
	public:
	long mtype;
	int state;
	char BeaconId[30];
	char PrimaryKey[100];
};
class thread_data{
		public:
		int *s_socket;
		char *uni_key;
		thread_data(){};
		thread_data(int *s,char *u)
		{
			s_socket=s;
			uni_key=new char[strlen(u)+1];
			strcpy(uni_key,u);
			printf("[thread_data]u:%s\n",u);
			printf("[thread_data]uni_key:%s\n",uni_key);
		};
		~thread_data()
		{
				delete uni_key;

		}
};
