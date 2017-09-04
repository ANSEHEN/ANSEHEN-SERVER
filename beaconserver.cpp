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

#define PORT 9003
#define ARG_MAX 2
#define BEACON_CONNECT 2

class beacon_data{
	public:
	long mtype;
	int state;
	char BeaconId[30];
	char PrimaryKey[100];
};
char buffer[BUFSIZ];
const int type = 2;
int main(){
	int	c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	socklen_t	len;
	int	n;
	int msgid;
	
	char *ptr;
	char c_buff[ARG_MAX][BUFSIZ];
	
	msgid=msgget(1234,IPC_CREAT);
	
	beacon_data bd;
	//mbuf msg;

	s_socket = socket(PF_INET, SOCK_STREAM, 0);

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(PORT);

	if(bind(s_socket, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) {
		printf("Can not Bind\n");
		return -1;
	}

	if(listen(s_socket, 5) == -1) {
		printf("listen Fail\n");
		return -1;
	}
	c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
	printf("socket connected\n");
		printf("point 1.-------\n");
		
//test
//////
	while(1){
		printf("Before: socket connecte\n");
		c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
		printf("socket connected\n");
		n=read(c_socket, buffer,sizeof(buffer));
		ptr=strtok(buffer," ");
		for(int i=0; i<=ARG_MAX ;i++)
		{
			if(ptr==NULL)
				break;
			strcpy(c_buff[i],ptr);
			ptr= strtok(NULL," ");
			printf("c_buff[%d] : %s\n",i,c_buff[i]);
		}
		strcpy(bd.BeaconId,c_buff[0]);
		strcpy(bd.PrimaryKey,c_buff[1]);
		bd.mtype=type;
		bd.state=BEACON_CONNECT;
		if(msgsnd(msgid,(void*)&bd,sizeof(class beacon_data),0)==-1){
			perror("send fail");
		}
		close(c_socket);
	}
}	
