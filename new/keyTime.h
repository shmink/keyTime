/*
 * struct canInfoStruct stores all the data you would need in for use in the program.
 * This was set up so that passing data to a function when using threads would work.
 *
 * sendIDinStruct - Stores the CAN ID you want to send to. In the form of a string as that's how parseFrame accepts them.
 * receiveIDinStruct - Stores the CAN ID you want to receive from.
 * sock - Stores the socket you created at an earlier time to then be able to send and receive data from the CAN bus network.
 * beginTime - Stores the time when passed to the timer function.
 * endTime - Stores the time when passed to the timer function.
 * cdata - Stores the values of each byte from a CAN message received from the ECU.
 * loop - Stores the amount of messages that you want to receive from the ECU. An optional counter.
 */
struct canInfoStruct {
		char *sendIDinStruct;
		int receiveIDinStruct;
		int sock;
		long *beginTime;
		long *endTime;
		int cdata[8];
		int loop;
	};

struct canInfoStruct myStruct;

/*
 * timer function is responsible for getting a time values
 * @param timer_val - take a long variable and sets the time equal to it
 */
void timer(long *timer_val);

/*
 * createSocket function creates a socket on the CAN bus network
 * so we're able to send and receive messages from it.
 * @param interface - A string value naming the interface where you want to create the socket.
 */
int createSocket(char *interface);

/*
 *	receiveMsg function is made to receive a message from the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *receiveMsg(void *ptr);

/*
 *	sendMsg function is made to send a message to the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *sendMsg(void *ptr);
