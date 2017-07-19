class CCTV
{
        private :
	static int total_cctv;
        char * id;
        char *beacon_id;
        char * ip_addr;
        char * location_addr;

        public :
	CCTV();
        CCTV(char* id_p,char *b_id_p,char * ip_p, char *lo_p); 
        char* get_id() const;
        char* get_beacon_id() const;
        char* get_ip() const;
        char* get_location() const;
	~CCTV();
};

class Node
{
        public :
        Node * front;
        CCTV * data;
        Node * rear;
	Node();
	~Node();
};

