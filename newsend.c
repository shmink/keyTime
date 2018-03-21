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
	[X] main function (args)
	[X] multi thread send and receive functions, maybe timer?
	[X] function for timing
*/

/*
 *	printUsage function prints how to use the program if there aren't enough arguments passed
 *	@param prg - string which will be the name of the program shoud it ever change.
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

/*
 * From can-utils, lib.h file.
 * Returns the decimal value of a given ASCII hex character.
 *
 * While 0..9, a..f, A..F are valid ASCII hex characters.
 * On invalid characters the value 16 is returned for error handling.
 */
unsigned char asc2nibble(char c) {

	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	return 16; /* error */
}

/*
 * From can-utils, lib.h file.
 *
 * Transfers a valid ASCII string decribing a CAN frame into struct canfd_frame.
 *
 * CAN 2.0 frames
 * - string layout <can_id>#{R{len}|data}
 * - {data} has 0 to 8 hex-values that can (optionally) be separated by '.'
 * - {len} can take values from 0 to 8 and can be omitted if zero
 * - return value on successful parsing: CAN_MTU
 *
 * CAN FD frames
 * - string layout <can_id>##<flags>{data}
 * - <flags> a single ASCII Hex value (0 .. F) which defines canfd_frame.flags
 * - {data} has 0 to 64 hex-values that can (optionally) be separated by '.'
 * - return value on successful parsing: CANFD_MTU
 *
 * Return value on detected problems: 0
 *
 * <can_id> can have 3 (standard frame format) or 8 (extended frame format)
 * hexadecimal chars
 *
 *
 * Examples:
 *
 * 123# -> standard CAN-Id = 0x123, len = 0
 * 12345678# -> extended CAN-Id = 0x12345678, len = 0
 * 123#R -> standard CAN-Id = 0x123, len = 0, RTR-frame
 * 123#R0 -> standard CAN-Id = 0x123, len = 0, RTR-frame
 * 123#R7 -> standard CAN-Id = 0x123, len = 7, RTR-frame
 * 7A1#r -> standard CAN-Id = 0x7A1, len = 0, RTR-frame
 *
 * 123#00 -> standard CAN-Id = 0x123, len = 1, data[0] = 0x00
 * 123#1122334455667788 -> standard CAN-Id = 0x123, len = 8
 * 123#11.22.33.44.55.66.77.88 -> standard CAN-Id = 0x123, len = 8
 * 123#11.2233.44556677.88 -> standard CAN-Id = 0x123, len = 8
 * 32345678#112233 -> error frame with CAN_ERR_FLAG (0x2000000) set
 *
 * 123##0112233 -> CAN FD frame standard CAN-Id = 0x123, flags = 0, len = 3
 * 123##1112233 -> CAN FD frame, flags = CANFD_BRS, len = 3
 * 123##2112233 -> CAN FD frame, flags = CANFD_ESI, len = 3
 * 123##3 -> CAN FD frame, flags = (CANFD_ESI | CANFD_BRS), len = 0
 *     ^^
 *     CAN FD extension to handle the canfd_frame.flags content
 *
 * Simple facts on this compact ASCII CAN frame representation:
 *
 * - 3 digits: standard frame format
 * - 8 digits: extendend frame format OR error frame
 * - 8 digits with CAN_ERR_FLAG (0x2000000) set: error frame
 * - an error frame is never a RTR frame
 * - CAN FD frames do not have a RTR bit
 */
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

int randHex(void) {
	int length;
    char output[2];
    char str[] = "0123456789ABCDEF";

    /* Seed number for rand() */
    srand((unsigned int) time(0) + getpid());
    length = rand() % 15 + 8;
 
    for (int i = 0; i < 2; ++i) {
        output[i] = str[rand() % 16];
        srand(rand());
    }

    int final = (int)strtol(output, NULL, 16);

    // printf("%X\n", final);

    return final;
}

/*
 *	sendMsg function is made to send a message to the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *sendMsg(void *ptr) {

	struct sendIDargsStruct *sendStruct = ptr;
	char *sID = sendStruct->sendIDinStruct;
	int s = sendStruct->sock;

	int reqMTU;						// Maximum transfer unit
	int nbytes;						// Total bytes
	struct canfd_frame frame;		// Pack data into this frame

	// Parse the CAN data and check it's the right length
	reqMTU = parseFrame(sID, &frame);
	if(!reqMTU) {
		fprintf(stderr, "Bad CAN format.\n");
		return (void *)1;
	}

	// Fill the final 4 bytes with random data
	frame.data[4] = randHex();
	frame.data[5] = randHex();
	frame.data[6] = randHex();
	frame.data[7] = randHex();

	// Write the data to the frame and send
	nbytes = write(s, &frame, reqMTU);

	// Once the data has been sent start the timer
	timer(sendStruct->beginTime);

	// Print out the information
	printf("=============" "\x1b[32m" "SENT" "\x1b[0m" "============\n");
	printf("%X - [%d] - %02X %02X %02X %02X %02X %02X %02X %02X\n", frame.can_id & 0x1fffffffu, frame.len, // 29 bit CAN ID
		frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
		frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
	
	
}

/*
 *	receiveMsg function is made to receive a message from the CAN network
 *	@param ptr - a void pointer used to accept structs of data
 */
void *receiveMsg(void *ptr) {

	struct receiveIDargsStruct *receiveStruct = ptr;
	int rID = receiveStruct->receiveIDinStruct;
	int s = receiveStruct->sock;

	struct can_frame frame;

	// Control how long the following while loop takes
	int loop = 1;

	// While looping read in CAN frames, if it matches a certain ID is an option aswell
	while(loop) {
		if(read(s, &frame, sizeof(struct can_frame)) > 0/* && (frame.can_id & 0x1fffffffu) == rID*/) {
			printf("===========" "\x1b[32m" "RECEIVED" "\x1b[0m" "==========\n");
			printf("%X - [%d] - %02X %02X %02X %02X %02X %02X %02X %02X\n", frame.can_id & 0x1fffffffu, frame.can_dlc, // 29 bit CAN ID
				frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);

			loop = 0;

			// Stop the timer
			timer(receiveStruct->endTime);
		}
	}
}

int main(int argc, char *argv[]) {

	// Check the amount of arguments
	if (argc != 4) {
		printUsage("cSAR");
		return 1;
	}

	// Take the 2nd argument as a string as it later gets processed into the parts needed.
	char *sendID = argv[2];	

	// Take the 3rd argument and make it a hex value.
	int receiveID;
	sscanf(argv[3], "%x", &receiveID);


	// Boiler plate socket creation
	int soc;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;

	// Interface name from 1st argument
	const char *ifname = argv[1];

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

	// Fill the structure with the relevant information
	sendStruct.sendIDinStruct = sendID;
	sendStruct.sock = soc;
	sendStruct.beginTime = malloc(sizeof(long)); 

	receiveStruct.receiveIDinStruct = receiveID;
	receiveStruct.sock = soc;
	receiveStruct.endTime = malloc(sizeof(long)); 


	//Create threads as both functions need to be running at the same time.
	pthread_t threadSEND, threadREC; //Don't forget the -pthread flag when compiling with gcc


	// Make threads
	// Running the receive function as it'll be ready and waiting after the frame from send has been sent
    pthread_create(&threadREC, NULL, receiveMsg, &receiveStruct);
    pthread_create(&threadSEND, NULL, sendMsg, &sendStruct);

    // Wait for threads to finish
    pthread_join(threadREC, NULL);
    pthread_join(threadSEND, NULL);

    // Work out the time difference from when the timer ended to when it started
	long duration = *(receiveStruct.endTime) - *(sendStruct.beginTime);
	// Convert that difference into seconds by dividing by 10^9
	double durSec = ((double)duration)/1e9;

	// Print information (also so useful for debugging left commented out - begin and end will be from EPOCH)
	printf("=============" "\x1b[32m" "TIME" "\x1b[0m" "============\n");
	// printf("begin = %.9ld\n", *(sendStruct.beginTime));
	// printf("end = %.9ld\n", *(receiveStruct.endTime));
	// printf("duration = %.9ld\n", duration);
	printf("%.10f seconds\n",durSec);

	// Free previously malloc'd items
	free(receiveStruct.endTime);
	free(sendStruct.beginTime);

	return 0;
}
