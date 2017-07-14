#include <stdio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>

class USER_INFO
{

	public :
	char phoneNum[14];
	char phoneNumInput[14];
	char name[20];
	char pw[20];
	char imageAdd[200];
	char uniqueKey[100];
};

