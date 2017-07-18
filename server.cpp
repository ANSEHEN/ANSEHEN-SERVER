#include "server.h"

#define PORT 9001
#define ARG_MAX 6
const char *host = (char*)"localhost";
const char *user = (char*)"root";
const char *pw = (char*)"bitiotansehen";
const char *db = (char*)"ansehen";
char    buffer[BUFSIZ];

static size_t write_data(void *ptr,size_t size, size_t nmemb, void * stream);

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
		printf("before\n");
                c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
		printf("after\n");
                n=read(c_socket, buffer,sizeof(buffer));
		buffer[strlen(buffer)+1]='\0';
		printf("got : %s",buffer);
		ptr=strtok(buffer," ");
		for(int i=0; i<ARG_MAX ;i++)
		{
			if(ptr==NULL)
				break;
			strcpy(c_buff[i],ptr);
			ptr= strtok(NULL," ");
			printf("c_buff[%d] : %s\n",i,c_buff[i]);
		}
		sprintf(query,"insert into USER_INFO (phone_num,phone_num_input,name,pw,image_add,unique_key,start,end) values ('%s','%s','%s','%s','%s','%s','start', 'end')",c_buff[0],c_buff[1],c_buff[2],c_buff[3],c_buff[4],c_buff[5]);
		query_stat = mysql_query(connection,query);
                if(query_stat != 0)
                {
                        fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
                        return 1;
                }
		CURL *curl_handle;
		char *pagefilename =c_buff[4];
		char url[BUFSIZ];
		sprintf(url,"http://13.124.164.203/image/%s",pagefilename); 
		FILE *pagefile;
		curl_global_init(CURL_GLOBAL_ALL);

  		/* init the curl session */
  		curl_handle = curl_easy_init();

	 	/* set URL to get here */
		curl_easy_setopt(curl_handle, CURLOPT_URL, url);

		/* Switch on full protocol/debug output while testing */
		curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

		/* disable progress meter, set to 0L to enable and disable debug output */
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

		/* send all data to this function  */
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

		/* open the file */
		pagefile = fopen(pagefilename, "wb");
		if(pagefile) 
		{
			/* write the page body to this file handle */
			curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

			/* get it! */
			curl_easy_perform(curl_handle);

			/* close the header file */
			fclose(pagefile);
		}

		/* cleanup curl stuff */
		curl_easy_cleanup(curl_handle);
                close(c_socket);
        }
        close(s_socket);
        mysql_close(connection);
        return 0;
}

static size_t write_data(void *ptr,size_t size, size_t nmemb, void * stream)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
	return written;
}
