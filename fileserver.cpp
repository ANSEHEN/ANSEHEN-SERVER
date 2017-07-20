#include "fileserver.h"
char	buffer[BUFSIZ] = "hello, world";

int main()
{
	int	c_socket, s_socket;
	struct sockaddr_in s_addr, c_addr;
	socklen_t	len;
	int	n;
	int msgid;
	
	
	msgid=msgget(1234,IPC_CREAT);
	
	 mbuf msg;

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

	while(1) {
		len = sizeof(c_addr);
		msgrcv(msgid,(void*)&msg,sizeof(struct mbuf),type,0);
		printf("rcv mesg!\n");
		printf("msg : %s \n",msg.buf);
		c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);

		n = strlen(buffer);
		write(c_socket, buffer, n);

		close(c_socket);
	}
	close(s_socket);
}
