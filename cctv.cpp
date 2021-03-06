#include "cctv.h"
#include <stdio.h>
CCTV::CCTV()
{
	id = NULL;
	beacon_id=NULL;
	ip_addr=NULL;
	location_addr=NULL;
	check=false;
}

CCTV::CCTV(char* id_p,char *b_id_p,char *ip_p,char* lo_p)
{
	id=id_p;
	beacon_id=b_id_p;
	ip_addr=ip_p;
	location_addr=lo_p;
	check=false;
}
CCTV::~CCTV()
{
        printf("delete CCTV id : %s\n",id);
		if(id!=NULL)
				delete id;
        if(beacon_id!=NULL)
                delete beacon_id;
        if(ip_addr!=NULL)
                delete ip_addr;
        if(location_addr!=NULL)
                delete location_addr;
        printf("delete CCTV\n");
}

char* CCTV::get_id() const
{
	return id;
}
char* CCTV::get_beacon_id() const
{
	return beacon_id;
}

char* CCTV::get_ip() const
{
	return ip_addr;
}
char* CCTV::get_location() const
{
	return location_addr;
}
void CCTV::set_check()
{
		check=true;
}
bool CCTV::get_check()
{
		return check;
}
Node::Node()
{
	front=NULL;
	rear=NULL;
	data=NULL;
}
Node::~Node()
{
        if(data!=NULL)
                delete data;
        if(rear!=NULL)
                delete rear;
        printf("delete Node\n");
}      
	
