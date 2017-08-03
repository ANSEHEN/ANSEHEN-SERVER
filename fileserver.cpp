#include "fileserver.h"
using namespace std;
const char *host = (char*)"localhost";
const char *user = (char*)"root";
const char *pw = (char*)"bitiotansehen";
const char *db = (char*)"ansehen";

char	buffer[BUFSIZ] = "hello, world";
int max_cctv;
int snd_num_cctv;
/*
void err_quit(const char *msg);
void err_display(const char *msg);
Node* get_send_cctv_info(char * uniqueKey);
Node* cctv_info_load();
void data_send(Thr_data thr);
*/
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

	Node * root_all_cctv = cctv_info_load();
	Node *root_snd_cctv = NULL;
	Pocket pocket[max_cctv];
//		msgrcv(msgid,(void*)&msg,sizeof(msg),type,0);
	for(int i=0;i<max_cctv;i++)
	{
		char ip[20];
		char id[5];

		pocket[i].c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
		Cctv_data cctv_data; 
		recv(pocket[i].c_socket,&cctv_data, sizeof(cctv_data),0);
		pocket[i].cctv=cctv_data;
		printf("well connected c_socket : %d, cctv_id : %s, ip : %s\n",pocket[i].c_socket,pocket[i].cctv.cctv_id,pocket[i].cctv.ip);
//		break; // test
	} 


	while(1) {
		len = sizeof(c_addr);
		msgrcv(msgid,(void*)&msg,sizeof(msg),type,0);
		printf("rcv mesg!\n");
		printf("msg : %s \n",msg.buf);
		printf("msg : %s \n",msg.unique_key);
		
		Data data;
		strcpy(data.unique_key,msg.unique_key);
		strcpy(data.image_addr,msg.image_addr);
		printf("unique : %s\n",data.unique_key);
		printf("image : %s\n",data.image_addr);
		if(root_snd_cctv!=NULL)
			delete root_snd_cctv;
		else
			printf("initial setting of root snd cctv\n");	
		snd_num_cctv=0;
		root_snd_cctv = get_send_cctv_info(msg.unique_key);
		printf("msg : %s \n",msg.image_addr);
//thread create  : thread 가져야하는 정보들 : cctvid, i imageaddr uniquekey 
		//thread snd
		Thread t[snd_num_cctv];
		Thr_data td[snd_num_cctv];
		Node *cur=root_snd_cctv;
		int i=0;
		while(cur!=NULL)
		{
			td[i].user_data=data;
			for(int j=0;j<max_cctv;j++)
			{	
				if(strcmp(cur->data->get_id(),pocket[j].cctv.cctv_id)==0)
				{
					td[i].pocket_data=pocket[j];
					break;	
				}
			}
			cur=cur->rear;
			i++;
		}
		for(i=0;i<snd_num_cctv;i++)
		{
			t[i].setThr(td[i]);
			t[i].run();
		}
	}

	for(int i=0;i<max_cctv;i++)
	{
		close(pocket[i].c_socket);
	}
	//close(pocket[0].c_socket);
	close(s_socket);
}

void data_send(Thr_data *thr)
{
	int retval;
	printf("c_socket : %d, cctv_id : %s, cctv_ip : %s\n",thr->pocket_data.c_socket,thr->pocket_data.cctv.cctv_id,thr->pocket_data.cctv.ip);
	printf("uniqueKey : %s, image_addr : %s\n",thr->user_data.unique_key,thr->user_data.image_addr);
	FILE *fp = fopen(thr->user_data.image_addr, "rb");
	if(fp == NULL){
		perror("파일 입출력 오류");
	}
	else
	{
		printf("lala\n");
	}
		

	// 파일 이름,유니크키  보내기 
		

	send(thr->pocket_data.c_socket, &thr->user_data,sizeof(thr->user_data),  0);


	// 전송 시작할 위치(=현재의 파일 크기) 받기
	int currbytes;
	retval = recv(thr->pocket_data.c_socket, (char *)&currbytes, sizeof(currbytes), MSG_WAITALL);
	if(retval < 0) err_quit("recv()");
	printf("### 옵셋 %d 바이트 지점부터 전송을 시작합니다. ###\n", currbytes);

	// 파일 크기 얻기
	fseek(fp, 0, SEEK_END);
	int filesize = ftell(fp);
	int totalbytes = filesize - currbytes;

	retval = send(thr->pocket_data.c_socket, (char *)&totalbytes, sizeof(totalbytes), 0);
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
			retval = send(thr->pocket_data.c_socket, buf, numread, 0);
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
}


Thread::Thread(){} 
Thread::Thread(Thr_data t):thr_data(t){} 

void Thread::run()
{
	thread snd(&data_send,&thr_data);
	snd.join();
} 
void Thread::setThr(Thr_data t)
{
 	thr_data=t;
}
Node* cctv_info_load()
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
        //Query _ number of CCTV
        if(mysql_query(connection, "select TABLE_ROWS from information_schema.tables where table_name = 'CCTV_INFO'"))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        sql_result = mysql_use_result(connection);
        sql_row=mysql_fetch_row(sql_result);
        max_cctv =atoi(sql_row[0]);
        printf("max_cctv : %d\n",max_cctv);

        mysql_free_result(sql_result);
        //Query _ CCTV_INFO
        if(mysql_query(connection, "select * from CCTV_INFO"))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        sql_result = mysql_use_result(connection);
        int i=0;
        while((sql_row=mysql_fetch_row(sql_result))!=NULL)
        {
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
                if(i!=max_cctv-1)
                {
                        cur->rear = new Node();
                        cur->rear->front=cur;
                        cur=cur->rear;
                }
                i++;

        }


        mysql_free_result(sql_result);

        mysql_close(connection);
        return root;
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
	snd_num_cctv=num;
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

