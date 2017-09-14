MYSQL_LIBS = -L/usr/lib/arm-linux-gnueabihf -lmysqlclient -lpthread -lz -lm -ldl
MYSQL_CFLAGS = -I/usr/include/mysql -DBIG_JOINS=1  -fno-strict-aliasing    -g -DNDEBUG
CURL_LIBS = -L/usr/lib/arm-linux-gnueabihf -lcurl

all :  
	g++ -o server $(MYSQL_LIBS) server.cpp cctv.cpp $(MYSQL_CFLAGS) $(CURL_LIBS) -pthread -std=c++11
	g++ -o fileserver $(MYSQL_LIBS) fileserver.cpp cctv.cpp $(MYSQL_CFLAGS) -pthread -std=c++11 
	g++ beacon_disconnected.cpp -o beacondisconn $(MYSQL_CFLAGS) $(MYSQL_LIBS) -std=c++11
	g++ beaconserver.cpp -o beaconconn $(MYSQL_CFLAGS) $(MYSQL_LIBS) -std=c++11

clean :
	rm server
	rm fileserver
	rm beacondisconn
	rm beaconconn
