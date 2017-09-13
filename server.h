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
#include <vector>
#include "cctv.h"

#define TYPE_FILE 1
#define TYPE_BEACON 2
#define TYPE_BEACON_L 3
#define TYPE_BEACON_C 4
#define TYPE_RESULT 5
class mbuf 
{
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
	int result;
};

class beacon_data
{
	public:
	long mtype;
	int state;
	char BeaconId[30];
	char PrimaryKey[100];
};
class Thread
{
		public:
		int c_socket_t;
		void run();
};


static size_t write_data(void *ptr,size_t size, size_t nmemb, void * stream);
Node* cctv_info_load();
void bcn_sig_to_cctv(int *msgid);
void get_location(int c_socket);
void result_to_android(int *msgid);
void receive_data_from_android(int *csocket);
void receive_state_from_android(int *csocket);
