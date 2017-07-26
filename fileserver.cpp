#include "fileserver.h"

const char *host = (char*)"localhost";
const char *user = (char*)"root";
const char *pw = (char*)"bitiotansehen";
const char *db = (char*)"ansehen";

char	buffer[BUFSIZ] = "hello, world";
void err_quit(const char *msg);
void err_display(const char *msg);
Node* get_send_cctv_info(char * uniqueKey);

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
		printf("msg : %s \n",msg.unique_key);
		get_send_cctv_info(msg.unique_key);
		printf("msg : %s \n",msg.image_addr);
		c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
                //unique_key
                n = strlen(msg.unique_key);
                write(c_socket, msg.unique_key, n);

		// FILE
		FILE *fp = fopen(msg.image_addr, "rb");
		if(fp == NULL){
			perror("파일 입출력 오류");
			return 1;
		}

		// 파일 이름 보내기
		char filename[256];
		strncpy(filename, msg.image_addr, sizeof(filename));
		int retval = send(c_socket, filename, 256, 0);
		if(retval < 0) err_quit("send()");

		// 전송 시작할 위치(=현재의 파일 크기) 받기
		int currbytes;
		retval = recv(c_socket, (char *)&currbytes, sizeof(currbytes), MSG_WAITALL);
		if(retval < 0) err_quit("recv()");
		printf("### 옵셋 %d 바이트 지점부터 전송을 시작합니다. ###\n", currbytes);

		// 파일 크기 얻기
		fseek(fp, 0, SEEK_END);
		int filesize = ftell(fp);
		int totalbytes = filesize - currbytes;

		// 전송할 데이터 크기 보내기
		retval = send(c_socket, (char *)&totalbytes, sizeof(totalbytes), 0);
		if(retval < 0) err_quit("send()");

		// 파일 데이터 전송에 사용할 변수
		char buf[BUFSIZE];
		int numread;
		int numtotal = 0;

		// 파일 데이터 보내기
		fseek(fp, currbytes, SEEK_SET); // 파일 포인터를 전송 시작 위치로 이동
		while(1){
			numread = fread(buf, 1, BUFSIZE, fp);
			if(numread > 0){
				retval = send(c_socket, buf, numread, 0);
				if(retval < 0){
					err_display("send()");
					break;
				}
				numtotal += numread;
				printf("."); fflush(stdout); // 전송 상황을 표시
				usleep(10000); // 전송 중단 실험을 위해 속도를 느리게 함
			}
			else if(numread == 0 && numtotal == totalbytes){
				printf("\n파일 전송 완료!: %d 바이트 전송됨\n", filesize);
				break;
			}
			else{
				perror("파일 입출력 오류");
				break;
			}
		}
		fclose(fp);
		close(c_socket);
	}
	close(s_socket);
}
Node* get_send_cctv_info(char * uniqueKey)
{

	Node *root = new Node();
        Node *cur =root;

        int query_stat;
        MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
        char *ptr;


        /* db */
        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        //Query _ SEND_CCTV_INFO
	sprintf(query,"select cctv_id from SEND_CCTV_INFO where unique_key = '%s'",uniqueKey);
        if(mysql_query(connection, query))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        sql_result = mysql_use_result(connection);
        int num=0;

        while((sql_row=mysql_fetch_row(sql_result))!=NULL)
        {
		num++;
        }
	printf("num of rows %s : %d\n",uniqueKey,num); 

        mysql_free_result(sql_result);
	
	char  cctv_id[num][5];

	sprintf(query,"select cctv_id from SEND_CCTV_INFO where unique_key = '%s'",uniqueKey);
        if(mysql_query(connection, query))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        sql_result = mysql_use_result(connection);
        int i=0;

        while((sql_row=mysql_fetch_row(sql_result))!=NULL)
        {
		strcpy(cctv_id[i],sql_row[0]);
		printf("%d cctv_id : %s\n",i,cctv_id[i]);
		i++;	
        }

        mysql_free_result(sql_result);
	
        //Query _ CCTV_INFO
	for(i=0;i<num;i++)
	{
		sprintf(query,"select * from CCTV_INFO where cctv_id = '%s'",cctv_id[i]);
        	if(mysql_query(connection,query)) 
	        {
       		         fprintf(stderr,"%s\n",mysql_error(connection));
               		 exit(1);
       		}
       		sql_result = mysql_use_result(connection);
       		sql_row=mysql_fetch_row(sql_result);
                char *id=new char[5];
                strcpy(id,sql_row[0]);
                printf("%s      ",id);
                char *b_id=new char[BUFSIZ];
                strcpy(b_id,sql_row[1]);
                printf("%s      ",b_id);
                char * ip = new char[BUFSIZ];
                strcpy(ip,sql_row[2]);
                printf("%s      ",ip);
                char * lo = new char[BUFSIZ];
                strcpy(lo,sql_row[3]);
                printf("%s\n",lo);
                cur->data=new CCTV(id,b_id,ip,lo);
                if(i!=num-1)
                {
                        cur->rear = new Node();
                        cur->rear->front=cur;
                        cur=cur->rear;
                }

		mysql_free_result(sql_result);
	}

        mysql_close(connection);
	//check if info is right
	cur= root;
	while(cur!=NULL)
	{
		printf("id :%s, ip : %s\n",cur->data->get_id(),cur->data->get_ip());
		cur=cur->rear;
	}
        return root;


}
// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
	perror(msg);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
	perror(msg);
}

