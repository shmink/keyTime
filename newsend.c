/*
	REMEMBER TO INCLUDE LICENSE IN THE END.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

#include "cSAR.h"

#define CANID_DELIM '#'
#define DATA_SEPERATOR '.'



/*
	[X] function for sending 
	[X] function for recieving
	[ ] main function (args)
	[ ] multi thread send and receive functions, maybe timer?
	[ ] function for timing
*/

void printUsage(char *prg) {
	fprintf(stderr, "Usage: %s [interface] [sending ID#data] [receive ID]\n\n", prg);
	fprintf(stderr, "%s = CAN send and receive\n", prg);
	fprintf(stderr, "Interface: can0 or vcan0 for example\n");
	fprintf(stderr, "Sending: the CAN ID you want to send to followed by '#' followed by data\n");
	fprintf(stderr, "Receive: the CAN ID you want to hear from - no ID is default\n");
	fprintf(stderr, "\nExample:\n");
	fprintf(stderr, "%s can0 18DA40F1#021001 18DAF140\n", prg);
}

unsigned char asc2nibble(char c) {

	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	return 16; /* error */
}

int parseFrame(char *cs, struct canfd_frame *cf) {
	/* documentation see lib.h */

	int i, idx, dlen, len;
	int maxdlen = CAN_MAX_DLEN;
	int ret = CAN_MTU;
	unsigned char tmp;

	len = strlen(cs);
	//printf("'%s' len %d\n", cs, len);

	memset(cf, 0, sizeof(*cf)); /* init CAN FD frame, e.g. LEN = 0 */

	if (len < 4)
		return 0;

	if (cs[3] == CANID_DELIM) { /* 3 digits */

		idx = 4;
		for (i=0; i<3; i++){
			if ((tmp = asc2nibble(cs[i])) > 0x0F)
				return 0;
			cf->can_id |= (tmp << (2-i)*4);
		}

	} else if (cs[8] == CANID_DELIM) { /* 8 digits */

		idx = 9;
		for (i=0; i<8; i++){
			if ((tmp = asc2nibble(cs[i])) > 0x0F)
				return 0;
			cf->can_id |= (tmp << (7-i)*4);
		}
		if (!(cf->can_id & CAN_ERR_FLAG)) /* 8 digits but no errorframe?  */
			cf->can_id |= CAN_EFF_FLAG;   /* then it is an extended frame */

	} else
		return 0;

	if((cs[idx] == 'R') || (cs[idx] == 'r')){ /* RTR frame */
		cf->can_id |= CAN_RTR_FLAG;

		/* check for optional DLC value for CAN 2.0B frames */
		if(cs[++idx] && (tmp = asc2nibble(cs[idx])) <= CAN_MAX_DLC)
			cf->len = tmp;

		return ret;
	}

	if (cs[idx] == CANID_DELIM) { /* CAN FD frame escape char '##' */

		maxdlen = CANFD_MAX_DLEN;
		ret = CANFD_MTU;

		/* CAN FD frame <canid>##<flags><data>* */
		if ((tmp = asc2nibble(cs[idx+1])) > 0x0F)
			return 0;

		cf->flags = tmp;
		idx += 2;
	}

	for (i=0, dlen=0; i < maxdlen; i++){

		if(cs[idx] == DATA_SEPERATOR) /* skip (optional) separator */
			idx++;

		if(idx >= len) /* end of string => end of data */
			break;

		if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
			return 0;
		cf->data[i] = (tmp << 4);
		if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
			return 0;
		cf->data[i] |= tmp;
		dlen++;
	}
	cf->len = dlen;

	return ret;
}

// void recordTime(int startORstop) {
// 	time_t *start = malloc(sizeof(time_t));
// 	time_t end;
	
// 	if(startORstop == 1) {
// 		// gettimeofday(&stop, NULL);
// 		printf("=============" "\x1b[32m" "TIME" "\x1b[0m" "============\n");
// 		// printf("%lu\n", stop.tv_usec - start.tv_usec);
// 		end = clock();
// 		long final = (end-(long)start)/CLOCKS_PER_SEC;
// 		printf("%lu\n", final);
// 		free(start);
// 	}


// 	if(startORstop == 0) {
// 		//gettimeofday(&start, NULL);
// 		*start = clock();
// 	}
// }

/**
	sendMsg function is made to send a message to the CAN network
	@param canID - takes a string in the format of CANID#DATA
	@param s - takes an int which is the socket to send the data to
*/
void *sendMsg(void *ptr) {
	printf("running in sendMsg\n");
	//recordTime(0);
	//char *IDandDATA, int s
	struct sendIDargsStruct *sendStruct = ptr;
	char *sID = sendStruct->sendIDinStruct;
	int s = sendStruct->sock;

	int reqMTU;					// maximum transfer unit
	int nbytes;					// total bytes
	struct canfd_frame frame;		// pack data into this frame

	// Parse the CAN data and check it's the right length
	reqMTU = parseFrame(sID, &frame);
	if(!reqMTU) {
		fprintf(stderr, "Bad CAN format.\n");
		return (void *)1;
	}

	// Write the data to the frame and send
	nbytes = write(s, &frame, reqMTU);

	printf("=============" "\x1b[32m" "SENT" "\x1b[0m" "============\n");
	printf("%X - [%d] - %02X %02X %02X %02X %02X %02X %02X %02X\n", frame.can_id & 0x1fffffffu, frame.len, // 29 bit CAN ID
		frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
		frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
	
	//return 0;
}

/**
	receiveMsg function is made to receive a message from the CAN network
	@param rID - takes an (hexadecimal) int as the ID you want to listen from
	@param s - takes an int which the socket where CAN messages will come from
*/
void *receiveMsg(void *ptr) {
	printf("running in receiveMsg\n");
	//int rID, int s
	struct receiveIDargsStruct *receiveStruct = ptr;
	int rID = receiveStruct->receiveIDinStruct;
	int s = receiveStruct->sock;

	struct can_frame frame;
	int loop = 1;

	while(loop) {
		if(read(s, &frame, sizeof(struct can_frame)) > 0/* && (frame.can_id & 0x1fffffffu) == rID*/) {
			printf("===========" "\x1b[32m" "RECEIVED" "\x1b[0m" "==========\n");
			printf("%X - [%d] - %02X %02X %02X %02X %02X %02X %02X %02X\n", frame.can_id & 0x1fffffffu, frame.can_dlc, // 29 bit CAN ID
				frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			loop = 0;
			//recordTime(1);
		}
	}

	//return 0;
}

int main(int argc, char *argv[]) {

	//cSAR can0 send receive
	//char progname[] = "cSAR";
	if (argc != 4) {
		printUsage("cSAR");
		return 1;
	}

	char *sendID = argv[2];				// Take the 2nd argument as a string as it later gets processed into the parts needed.

	int receiveID;
	sscanf(argv[3], "%x", &receiveID); // Take the 3rd argument and make it a hex value.


	// Boiler plate socket creation
	int soc;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;

	const char *ifname = argv[1];			// Interface name from 1st argument

	if((soc = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(soc, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if(bind(soc, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
		return -2;
	}

	// Passing multiple arguments with pthread is awkward
	sendStruct.sendIDinStruct = sendID;
	sendStruct.sock = soc;

	receiveStruct.receiveIDinStruct = receiveID;
	receiveStruct.sock = soc;


	//Create threads as both functions need to be running at the same time.
	pthread_t threadSEND, threadREC; //Don't forget the -pthread flag when compiling with gcc

	// make threads
	//pthread_create(&threadTIME, NULL, recordTime, 2);
    pthread_create(&threadSEND, NULL, sendMsg, &sendStruct);
    sleep(5);
    pthread_create(&threadREC, NULL, receiveMsg, &receiveStruct);
    

    // wait for them to finish
    pthread_join(threadREC, NULL);
    pthread_join(threadSEND, NULL); 


	//sendMsg(sendID, soc); 				// Sent a message

	//receiveMsg(receiveID, soc); 		// Receive a message

	//close(soc); // Close socket as program is about to end.
	return 0;
}
