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

static size_t write_data(void *ptr,size_t size, size_t nmemb, void * stream);
Node* cctv_info_load();
void set_send_cctv_info(Node *root,char *uniqueKey);
void bcn_sig_to_cctv(int *msgid);
void get_location(thread_data *data);

int main(void)
{
        int     c_socket, s_socket;
        struct sockaddr_in s_addr, c_addr;
        socklen_t       len;
        int     n,query_stat;
        MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
		char *ptr;
		char c_buff[ARG_MAX][BUFSIZ];
        
		s_socket = socket(PF_INET, SOCK_STREAM, 0);

        memset(&s_addr, 0, sizeof(s_addr));
        s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        s_addr.sin_family = AF_INET;
        s_addr.sin_port = htons(PORT);
        ////////////////////////////////////////

		/* CCTV Information*/
		Node * root = cctv_info_load();

        /* db */

        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
				exit(1);
        }

        //send sql query
        if(mysql_query(connection, "show tables"))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
        sql_result = mysql_use_result(connection);

        //output table name
        printf("MySQL Tables in mysql database:\n");
        while((sql_row=mysql_fetch_row(sql_result))!=NULL)
                printf("%s\n",sql_row[0]);

        mysql_free_result(sql_result);



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
	
		int msgid;
		msgid=msgget(1234,IPC_CREAT);

		thread beaconSignaltoCCTV(&bcn_sig_to_cctv,&msgid);
		while(1) 
		{
            len = sizeof(c_addr);
			printf("[main]before accepting c_socket\n");
		 	mbuf msg;

		    c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
			printf("[main]after accepting c_socket\n");
        	n=read(c_socket, buffer,sizeof(buffer));
			buffer[strlen(buffer)+1]='\0';
			printf("[main]got : %s\n",buffer);
			ptr=strtok(buffer," ");

			for(int i=0; i<ARG_MAX ;i++)
			{
				if(ptr==NULL)
					break;
				strcpy(c_buff[i],ptr);
				ptr= strtok(NULL," ");
				printf("[main]c_buff[%d] : %s\n",i,c_buff[i]);
			}

			sprintf(query,"insert into USER_INFO (phone_num,phone_num_input,name,pw,image_add,unique_key,path,end) values ('%s','%s','%s','%s','%s','%s','start', 'end')",c_buff[0],c_buff[1],c_buff[2],c_buff[3],c_buff[4],c_buff[5]);

			query_stat = mysql_query(connection,query);
            if(query_stat != 0)
            {
 		           fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
                        return 1;
            }

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
            
			
			printf("[main] thread data create\n");
			thread_data threadData(&s_socket,c_buff[5]);
			printf("[main] thread create\n");
			printf("[main] s_socket:%d\n",s_socket);
			thread location_thr(get_location,&threadData);
			location_thr.join();
			//location_thr.detach();
			printf("[main] after creating a thread\n");

			close(c_socket);
			//intercommunication with fileserver	
			/*msg.mtype=1;
			strcpy(msg.buf,"server sent\n");
			strcpy(msg.unique_key,c_buff[5]);
			strcpy(msg.image_addr,c_buff[4]);
			if(msgsnd(msgid,(void*)&msg,sizeof(msg),0)==-1)
             		perror("send fail ");
			set_send_cctv_info(root,msg.unique_key);*/
        }
        close(s_socket);
        mysql_close(connection);
        return 0;
}

void get_location(thread_data *data)
{
        int     c_socket;
        int query_stat;
        MYSQL *connection;
        MYSQL_RES  *sql_result;
        MYSQL_ROW sql_row;
        char query[BUFSIZ];
        char *ptr;
        struct sockaddr_in  c_addr;
		char r_uniqueKey[50],r_endPoint[BUFSIZ],r_allPath[BUFSIZ];
		char s_check[10];
		int *s_socket=data->s_socket;
		Node *root;
		socklen_t len = sizeof(c_addr);
		printf("[get location]\n");
		printf("[get location]s_socket : %d\n",*s_socket);
		/* db */
        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }

		/* network */
	    c_socket = accept(*s_socket, (struct sockaddr *) &c_addr,&len);
		printf("[get location] c_socket accepted\n");
		printf("[get_location]uniqueKey : %s wait for location info\n",data->uni_key);

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
		root = cctv_info_load();
		for(;;)
		{
				Node *cur = root;
				read(c_socket,r_allPath,sizeof(r_allPath));
				printf("[get_location]got allPath : *%s*\n",r_allPath);
				if(strcmp(r_allPath,"one")==0)
						break;
				ptr=strtok(r_allPath,",");
				latitude = atof(ptr);
				printf("[get_location] latitude %lf\n",latitude);
				ptr= strtok(NULL,",");
				longitude = atof(ptr);
				printf("[get_location] longitude %lf\n",longitude);
				while(cur!=NULL)
				{
						if(cur->data->get_check()==false)
						{
								double latitude_c;
								double longitude_c;
								ptr = cur->data->get_location();
								ptr=strtok(r_allPath,",");
								latitude_c = atof(ptr);
								ptr= strtok(NULL,",");
								longitude_c = atof(ptr);
								printf("[get_location]cmp latitude %lf longitude %lf \n",latitude_c,longitude_c);
						}
						cur=cur->rear;
				}

				write(c_socket,s_check,strlen(s_check)+1);
		}
		write(c_socket,s_check,strlen(s_check)+1);
		close(c_socket);
		//set_send_cctv_info(root,data->uni_key);

        /* db */
        connection = mysql_init(NULL);
        if(!mysql_real_connect(connection,host,user,pw,db,0,NULL,0))
        {
                fprintf(stderr,"%s\n",mysql_error(connection));
                exit(1);
        }
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
//TYPE_BEACON 2
//type_beacon_c 4
	while(1)
	{
		printf("wait for beacon signal\n");
		msgrcv(*msgid,(void*)&msg,sizeof(msg),TYPE_BEACON,0);
		printf("beacon signal (%s) of unique key (%s)\n",msg.BeaconId, msg.PrimaryKey);
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

void set_send_cctv_info(Node *root,char *uniqueKey)
{
 	
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
		/*	sprintf(query,"insert into SEND_CCTV_INFO values ('%s','%s')",uniqueKey,"2");

		query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
                fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
		}*/

		sprintf(query,"insert into SEND_CCTV_INFO values ('%s','%s')",uniqueKey,"3");

		query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
                fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        }
        
		/*sprintf(query,"insert into SEND_CCTV_INFO values ('%s','%s')",uniqueKey,"1");

		query_stat = mysql_query(connection,query);
        if(query_stat != 0)
        {
                fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
        }*/
        mysql_close(connection);
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



