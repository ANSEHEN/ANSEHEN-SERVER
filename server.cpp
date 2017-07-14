#include "server.h"

#define PORT 9001
const char *host = (char*)"localhost";
const char *user = (char*)"root";
const char *pw = (char*)"bitiotansehen";
const char *db = (char*)"ansehen";
char    buffer[BUFSIZ];
USER_INFO userTemp;


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
                c_socket = accept(s_socket, (struct sockaddr *) &c_addr, &len);
                //n=read(c_socket, buffer,sizeof(buffer));
		read(c_socket,&userTemp,sizeof(userTemp));
                //buffer[n]='\0';
                //printf("received Data : %s\n",buffer);
                //memset(query,'\0',strlen(query));
                //sprintf(query,"insert into USER_INFO (unique_key,name,phone_num,phone_num_input,image_add,start,end) values ('11','la','%s','123','123','111','111')",buffer);
                sprintf(query,"insert into USER_INFO (unique_key,name,phone_num,phone_num_input,pw,image_add,start,end) values ('%s','%s','%s','%s','%s','%s','start', 'end')",userTemp.uniqueKey,userTemp.name,userTemp.phoneNum,userTemp.phoneNumInput,userTemp.pw,userTemp.imageAdd);
		query_stat = mysql_query(connection,query);
                if(query_stat != 0)
                {
                        fprintf(stderr,"Mysql query error : %s\n",mysql_error(connection));
                        return 1;
                }
                close(c_socket);
        }
        close(s_socket);
        mysql_close(connection);
        return 0;
}

