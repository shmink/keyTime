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
	int nbytes;
	struct sockaddr_can addr;
	struct canfd_frame frame;
	struct ifreq ifr;

	const char *ifname = "vcan0";

	if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
		perror("Error while opening socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(s, SIOCGIFINDEX, &ifr);

	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	//printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

	if(bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error in socket bind");
		return -2;
	}

//	frame.can_id  = 0x123;  //CAN ID
//	frame.can_dlc = 3;	//CAN amount of data?
//	frame.data[0] = 0x11;	//CAN 0th bit of data
//	frame.data[1] = 0x22;	//CAN i'th bit of data...
//	frame.data[2] = 0x33;
//	frame.data[3] = 0x69;
//	frame.data[4] = 0x69;
//	frame.data[5] = 0x69;
//	frame.data[6] = 0x69;
//	frame.data[7] = 0x69;
		   // 18DAF140
//	int idHex = 0x18DAF140; //not sure why have to prepend value with an 8 at this time
//	int idHex2 = 0xC20A000;
	//printf("Looking for CAN ID %x\n", idHex);

	while(1) {
		if(read(s, &frame, sizeof(struct can_frame)) > 0 && 
			frame.can_id == 0x18DAF140) {
			printf("%X - [%X] - %X %X %X %X %X %X %X %X\n", frame.can_id, frame.len,
				frame.data[0], frame.data[1], frame.data[2], frame.data[3], 
				frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
		}
	}
	/*
		Need to start a timer around here.
		Need to check for response on the same ID?
		Stop timer when the response comes in.
	*/

	printf("Wrote %d bytes\n", nbytes);
	printf("Sent %d %d %d to CAN ID %d\n", frame.data[0], frame.data[1], frame.data[2], frame.can_id);

	return 0;
}
