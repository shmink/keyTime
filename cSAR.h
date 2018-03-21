struct sendIDargsStruct {
		char *sendIDinStruct;
		int sock;
		long *beginTime;
	};

struct receiveIDargsStruct {
	int receiveIDinStruct;
	int sock;
	long *endTime;
};

struct sendIDargsStruct sendStruct;
struct receiveIDargsStruct receiveStruct;


void timer(long *timer_val);

// int randHex(void);