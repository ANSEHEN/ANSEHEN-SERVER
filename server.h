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
class mbuf {
	public :
	long mtype;
	char buf[100];
	char unique_key[100];
	char image_addr[200];
};

class beacon_data{
	public:
	long mtype;
	char BeaconId[30];
	char PrimaryKey[100];
};
