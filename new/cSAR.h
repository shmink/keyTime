struct canInfoStruct {
		char *sendIDinStruct;
		int receiveIDinStruct;
		int sock;
		long *beginTime;
		long *endTime;
		int cdata[8];
		int loop;
	};

struct canInfoStruct sendStruct;
// struct receiveIDargsStruct receiveStruct;

void timer(long *timer_val);

void makeCSV(char *filename, int data[], double time);

int createSocket(char *interface);
