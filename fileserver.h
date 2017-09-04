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
#include <thread>

#include "cctv.h"
#define PORT 9002
#define BUFSIZE 1024
#define TYPE_FILE 1
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
	int state;
	char BeaconId[30];
	char PrimaryKey[100];
};

class Cctv_data {
	public:
	char cctv_id[5];
	char ip[20];
	Cctv_data& operator=(const Cctv_data& ref)
	{

		strcpy(cctv_id,ref.cctv_id);
		strcpy(ip,ref.ip);
		return *this;
	}
};

class Pocket {
	public :
	int c_socket;
	Cctv_data cctv;
};

class Data {
	public :
	char unique_key[100];
	char image_addr[200];
};
void err_quit(const char *msg);
void err_display(const char *msg);
Node* get_send_cctv_info(char * uniqueKey);
Node* cctv_info_load();

class Thr_data{
	public :
	Data user_data;
	Pocket pocket_data;
	Thr_data(){}
	Thr_data(Data data, Pocket pocket) : user_data(data),pocket_data(pocket)
	{}
};

void data_send(Thr_data *thr);
class Thread{
	public:
	Thr_data thr_data;

	public:
	Thread();
	Thread(Thr_data t);
	void run();
	void setThr(Thr_data t);
	friend void data_send(Thr_data *thr);

};

class Beacon_Pocket
{
	public :
	Pocket *pocket_data;
	int msgid;
	int num_pocket;
};
void bcn_sig_to_cctv(Beacon_Pocket *t);

