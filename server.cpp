#include "server.h"

#define PORT 9005
#define ARG_MAX 6

using namespace std;
const char *host = (char*)"localhost";
const char *user = (char*)"root";
const char *pw = (char*)"bitiotansehen";
const char *db = (char*)"ansehen";
char    buffer[BUFSIZ];
int max_cctv;
int msgid;

int main(void)
{
        int      s_socket;
        struct sockaddr_in s_addr, c_addr;
        socklen_t       len;
        int     n,query_stat;
        
        
	s_socket = socket(PF_INET, SOCK_STREAM, 0);

        memset(&s_addr, 0, sizeof(s_addr));
        s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        s_addr.sin_family = AF_INET;
        s_addr.sin_port = htons(PORT);
		
	msgid=msgget(1234,IPC_CREAT);

	/* CCTV Information*/
	Node * root = cctv_info_load();

        /* network */
        if(bind(s_socket, (struct sockaddr *) &s_addr, sizeof(s_addr)) == -1) 
	{
                printf("Can not Bind\n");
                return -1;
        }

        if(listen(s_socket, 5) == -1) 
	{
                printf("listen Fail\n");
                return -1;
        }
	

	thread beaconSignaltoCCTV(bcn_sig_to_cctv,&msgid);
	thread resultSignaltoAndroid(result_to_android,&msgid);

	vector<Thread> thr;
	vector<int> c_socket;
	int num=0;

	while(1) 
	{
		len = sizeof(c_addr);
		printf("[main] *%d. before accepting c_socket\n",num+1);
		int temp_c = accept(s_socket, (struct sockaddr *) &c_addr, &len);
		c_socket.push_back(temp_c);
		printf("[main] %d. after accepting c_socket\n",num+1);
		Thread temp;
		temp.c_socket_t=c_socket[num];
		thr.push_back(temp);
		thr[num].run();
		printf("[main]running thr %d\n",num+1);		
		num++;
        }
        close(s_socket);
        return 0;
}
void receive_result_from_android(int *csocket)
{
	int c_socket=*csocket;
	MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
	char *ptr;
	printf("[receive state]\n");

	char s_check[10];
	char unique_key[100];
	int query_stat;

        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
		exit(1);
        }

        read(c_socket, unique_key,sizeof(unique_key));
	strcpy(s_check,"zero");

	//receive unique key from android
	write(c_socket,s_check,strlen(s_check)+1);

	printf("[receive_result_from_android] service finish (%s)\n",unique_key);
	close(c_socket);
        mysql_close(connection);
}

void receive_state_from_android(int *csocket)
{
	int c_socket=*csocket;
	MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
	char *ptr;
	printf("[receive state]\n");
	//cctv 포인트지점을 지났는 지에 대한 결과를 안드로이드로 부터 받아서... 재저장.....

	char s_check[10];
	char unique_key[100];
	char cctv_id[10];
	int query_stat;

        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
		exit(1);
        }

        read(c_socket, unique_key,sizeof(unique_key));
	strcpy(s_check,"zero");
	//receive unique key from android
	write(c_socket,s_check,strlen(s_check)+1);
        
	read(c_socket, cctv_id,sizeof(cctv_id));
	strcpy(s_check,"zero");
	//receive cctv_id from android
	write(c_socket,s_check,strlen(s_check)+1);

	sprintf(query,"update SEND_CCTV_INFO set state = 1 where cctv_id = '%s' and unique_key = '%s'",cctv_id,unique_key);
	printf("query : %s\n",query);
		
	query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
		fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        }
	close(c_socket);
        mysql_close(connection);
}
void receive_data_from_android(int *csocket)
{
	mbuf msg;
	int c_socket=*csocket;
	MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
	char *ptr;
	char c_buff[ARG_MAX][BUFSIZ];
	char s_check[10];
	int query_stat;
	printf("[receive data]\n");

        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
		exit(1);
        }

	printf("[receive_data]after accepting c_socket\n");
        read(c_socket, buffer,sizeof(buffer));
	strcpy(s_check,"zero");
	//receive unique key from android
	write(c_socket,s_check,strlen(s_check)+1);

	buffer[strlen(buffer)+1]='\0';
	printf("[receive_data]got : %s\n",buffer);
	ptr=strtok(buffer," ");

	for(int i=0; i<ARG_MAX ;i++)
	{
		if(ptr==NULL)
			break;
		strcpy(c_buff[i],ptr);
		ptr= strtok(NULL," ");
		printf("[receive_data]c_buff[%d] : %s\n",i,c_buff[i]);
	}

	sprintf(query,"insert into USER_INFO (phone_num,phone_num_input,name,pw,image_add,unique_key,path,end) values ('%s','%s','%s','%s','%s','%s','start', 'end')",c_buff[0],c_buff[1],c_buff[2],c_buff[3],c_buff[4],c_buff[5]);

	query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
		fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        }
	get_location(c_socket);

	printf("[receive_data] after setting location\n");
		
	/* file download from url */
	CURL *curl_handle;
	char *pagefilename =c_buff[4];
	char url[BUFSIZ];
	sprintf(url,"http://13.124.164.203/image/%s",pagefilename); 
	FILE *pagefile;
	curl_global_init(CURL_GLOBAL_ALL);

  	// init the curl session 
  	curl_handle = curl_easy_init();

	// set URL to get here 
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);

	// Switch on full protocol/debug output while testing 
	curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

	// disable progress meter, set to 0L to enable and disable debug output 
	curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

	// send all data to this function  
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

	// open the file 
	pagefile = fopen(pagefilename, "wb");
	if(pagefile) 
	{
		// write the page body to this file handle 
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
		curl_easy_perform(curl_handle);

		// close the header file 
		fclose(pagefile);
	}	

	// cleanup curl stuff 
	curl_easy_cleanup(curl_handle);

	close(c_socket);
	//intercommunication with fileserver	
	msg.mtype=1;
	strcpy(msg.buf,"server sent\n");
	strcpy(msg.unique_key,c_buff[5]);
	strcpy(msg.image_addr,c_buff[4]);
	if(msgsnd(msgid,(void*)&msg,sizeof(msg),0)==-1)
		perror("send fail ");
	printf("\n[receive_data] finished (unique_key %s)\n",msg.unique_key);
        mysql_close(connection);
}

void result_to_android(int *msgid)
{
	mbuf msg;
	while(1)
	{
		printf("[result_to_android]wait for result\n");
		msgrcv(*msgid,(void*)&msg,sizeof(msg),TYPE_RESULT,0);
		printf("[result_to_android] unique key (%s) result %d cctv : %s\n",msg.unique_key,msg.result,msg.buf);

        	MYSQL *connection;
        	MYSQL_RES  *sql_result;
        	MYSQL_ROW sql_row;
        	char query[BUFSIZ];

		/* db */
		connection = mysql_init(NULL);
		if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
		{
			fprintf(stderr,"%s\n",mysql_error(connection));
			exit(1);
		}
		sprintf(query,"update SEND_CCTV_INFO set state = %d where unique_key = '%s' and cctv_id = '%s'",msg.result,msg.unique_key,msg.buf);

		int query_stat = mysql_query(connection,query);
		if(query_stat != 0)
		{
			fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
		}
		mysql_close(connection);
		
		sprintf(query,"curl -d \"uniqueKey=%s&result=%d&CCTVID=%s\" http://13.124.164.203/main_set_result.php",msg.unique_key,msg.result,msg.buf);
		system(query);
		printf("[result_to_android]\n");
	}
}

void get_location(int c_socket)
{
        int query_stat;
        MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
        char *ptr;
        
	char r_uniqueKey[50],r_endPoint[BUFSIZ],r_allPath[BUFSIZ];
	char s_check[10];
	Node *root;
	printf("[get location]\n");
		
	/* db */
        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }

	strcpy(s_check,"zero");
		
	//receive unique key from android
        read(c_socket, r_uniqueKey,sizeof(r_uniqueKey));
	printf("[get_location]got uniqueKey : *%s*\n",r_uniqueKey);
	write(c_socket,s_check,strlen(s_check)+1);
		
	//receive end point from android
	read(c_socket,r_endPoint,sizeof(r_endPoint));
	printf("[get_location]got endPoint : *%s*\n",r_endPoint);
	write(c_socket,s_check,strlen(s_check)+1);

	// receive all path from android
	double latitude;
	double longitude;
	char* temp_p=NULL;
	root = cctv_info_load();
	int seq=0;
	for(;;)
	{
		char test_ch[BUFSIZ];
		Node *cur = root;
		read(c_socket,r_allPath,sizeof(r_allPath));
		if(strcmp(r_allPath,"one")==0)
			break;
		ptr=strtok(r_allPath,",");
		latitude = atof(ptr);
		ptr= strtok(NULL,",");
		longitude = atof(ptr);
		while(cur!=NULL)
		{
			if(cur->data->get_check()==false)
			{
				double latitude_c;
				double longitude_c;
				double temp_lat,temp_lon;
				char *ptr2 = cur->data->get_location();
								
				strcpy(test_ch,ptr2);

				temp_p=test_ch;
				ptr=strtok(temp_p,",");
				latitude_c = atof(ptr);
				ptr= strtok(NULL,",");
				longitude_c = atof(ptr);
				ptr=NULL;
				ptr2=NULL;
				temp_lat=latitude-latitude_c;
				temp_lon=longitude-longitude_c;
				if(temp_lat<0)
				{
					temp_lat=temp_lat*(-1);
				}
				if(temp_lon<0)
				{
					temp_lon=temp_lon*(-1);
				}

				if((temp_lat+temp_lon)<0.00025){
					//허용범위내(25m) CCTV가 위치함
					//경로 설정
					printf("[get CCTV]------------------------\n");
					cur->data->set_check();
					seq++;
					printf("[get CCTV] sequence %d . cctv_id: %s Lat: %lf,Lon: %lf selected\n",seq,cur->data->get_id(),latitude_c,longitude_c);
					// unique key 와 cctv_id를 이용하여 db에 SEND_CCTV_INFO에 저장 
					sprintf(query,"insert into SEND_CCTV_INFO (unique_key, cctv_id,cnt,seq) values ('%s','%s',0,%d)",r_uniqueKey,cur->data->get_id(),seq);

					query_stat = mysql_query(connection,query);
        				if(query_stat != 0)
        				{
                				fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        				}
        
				}
			}
			cur=cur->rear;
		}

		write(c_socket,s_check,strlen(s_check)+1);
	}
	write(c_socket,s_check,strlen(s_check)+1);
		

        /* db */
	sprintf(query,"update USER_INFO set end = '%s' where unique_key = '%s'",r_endPoint,r_uniqueKey);
	query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
                fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        }
			
        mysql_close(connection);
}
void bcn_sig_to_cctv(int* msgid)
{
	beacon_data msg;

	while(1)
	{
		printf("[bcn_sig_to_cctv]wait for beacon signal\n");
		msgrcv(*msgid,(void*)&msg,sizeof(msg),TYPE_BEACON,0);
		printf("[bcn_sig_to_cctv]beacon signal (%s) of unique key (%s)\n",msg.BeaconId, msg.PrimaryKey);

		//check

		MYSQL *connection;
		MYSQL_RES  *sql_result;
		MYSQL_ROW sql_row;
		char query[BUFSIZ];

		/* db */
		connection = mysql_init(NULL);
		if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
		{
			fprintf(stderr,"%s\n",mysql_error(connection));
			exit(1);
		}
		sprintf(query,"select * from SEND_CCTV_INFO join CCTV_INFO on SEND_CCTV_INFO.cctv_id = CCTV_INFO.cctv_id where unique_key = '%s'",msg.PrimaryKey);

		int query_stat = mysql_query(connection,query);
		if(query_stat != 0)
		{
			fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
		}
		sql_result = mysql_use_result(connection);
		bool check=false;
		int state;
		while((sql_row=mysql_fetch_row(sql_result))!=NULL)
		{
				if(strcmp(msg.BeaconId,sql_row[6])==0)
				{		
						check=true;
						int cnt = atoi(sql_row[3]);
						int rs = atoi(sql_row[2]);
						printf("[bcn_sig_to_cctv]true count %d state %d result %d\n",cnt,msg.state,rs);

						if((rs==0)&&msg.state==TYPE_BEACON)//비컨 영역 안에 처음 들어옴
						{
								cnt++;
								sprintf(query,"update SEND_CCTV_INFO set cnt = %d where unique_key = '%s' and cctv_id = '%s'",cnt,msg.PrimaryKey,sql_row[1]);
								state=1;
								printf("[bcn_sig_to_cctv] access bcn %s, unique_key %s\n",msg.BeaconId, msg.PrimaryKey);

						}
						else if(!(rs==1)&&msg.state==TYPE_BEACON_L)//비컨 영역 밖으로 처음 나감
						{
								cnt++;
								sprintf(query,"update SEND_CCTV_INFO set cnt = %d where unique_key = '%s' and cctv_id = '%s'",cnt,msg.PrimaryKey,sql_row[1]);
								state=2;
								printf("[bcn_sig_to_cctv]fist out signal  bcn %s, unique_key %s\n",msg.BeaconId, msg.PrimaryKey);
						}
						else
						{
								check =false;
						}
						break;
				}
		}
        	mysql_free_result(sql_result);
		if(check ==false)
		{
				printf("[bcn_sig_to_cctv]false, bcn %s, unique_key %s\n",msg.BeaconId, msg.PrimaryKey);
				continue;
		}
		if(state==1||state==2)
		{
			query_stat = mysql_query(connection,query);
        	if(query_stat != 0)
        	{
                fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        	}
		}
        	mysql_close(connection);
		msg.mtype=TYPE_BEACON_C;
		if(msgsnd(*msgid,(void*)&msg,sizeof(msg),0)==-1)
             		perror("send fail ");
		printf("becaon signal is sent.(unique key : %s)\n",msg.PrimaryKey);
	}
}

static size_t write_data(void *ptr,size_t size, size_t nmemb, void * stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
	return written;
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
				///////////////////////////////check!!!
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

void Thread::run()
{
		printf("[Thread class]\n");
		char choice[30];
		char s_check[10];
		strcpy(s_check,"zero");
		read(c_socket_t, choice,sizeof(choice));
		printf("[Thread class]got choice info : %s\n",choice);
		write(c_socket_t,s_check,strlen(s_check)+1);
		if(strcmp(choice,"info")==0)
		{
				printf("[Thread class]c_socket is connected and now ready for taking info from a user\n");
				thread thr(receive_data_from_android,&c_socket_t);
				thr.detach();
		}
		else if(strcmp(choice,"state")==0)	
		{
				printf("[Thread class]c_socket is connected and took a state info from a User\n");
				thread thr(receive_state_from_android ,&c_socket_t);
				thr.detach();
		}
		else if(strcmp(choice,"result")==0)
		{
				printf("[Thread class]c_socket is connected and took a result info from a User\n");
				thread thr(receive_result_from_android,&c_socket_t);
				thr.detach();
		}
		else
		{
				printf("[Thread class]c_socket is connected\n");
				close(c_socket_t);
		}
}


