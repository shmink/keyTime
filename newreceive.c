#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int
main(void) {

	int s;
	int nbytes = 0;
	struct sockaddr_can addr;
	struct can_frame frame;
	struct ifreq ifr;
	int loop = 1;				// loop till ID has been found.
//	struct can_filter *rfilter; // how do work?
// not sure about stuff
/*	struct can_filter rfilter[2];

    rfilter[0].can_id   = 0x123;
    rfilter[0].can_mask = CAN_SFF_MASK;
    rfilter[1].can_id   = 0x200;
    rfilter[1].can_mask = 0x700;

    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
*/
//not sure about stuff end

	const char *ifname = "vcan0";

	if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(s, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
		return -2;
	}

	//nbytes = read(s, &frame, sizeof(struct can_frame));

	// if(nbytes < 0) {
	// 	perror("Error reading from socket.\n");
	// 	return 1;
	// }

	while(1) {
		if(read(s, &frame, sizeof(struct can_frame)) > 0/* && (frame.can_id & 0x1fffffffu) == 0x18DA40F1*/) {
			printf("===========" "\x1b[32m" "RECEIVED" "\x1b[0m" "==========\n");
			printf("%X - [%d] - %02X %02X %02X %02X %02X %02X %02X %02X\n", frame.can_id & 0x1fffffffu, frame.can_dlc, // 29 bit CAN ID
				frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
			loop = 0;
		}
	}

	close(s);

	return 0;
}
