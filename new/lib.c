#include <stdio.h>
#include <string.h>

#include <time.h>

#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

#include "cSAR.h"

void timer(long *timer_val) {
	struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    *timer_val = tp.tv_sec * 1000000000 + tp.tv_nsec;
}

void makeCSV(char *filename, int data[], double time) {
	FILE *fp;

	// struct canInfoStruct *receiveStruct = ptr;
	// int data3 = receiveStruct->cdata[3];
	// int data4 = receiveStruct->cdata[4];
	// int data5 = receiveStruct->cdata[5];
	// int data6 = receiveStruct->cdata[6];
	// long *begin = receiveStruct->beginTime;
	// long *end = receiveStruct->endTime;

	filename = strcat(filename, ".csv");
	fp = fopen(filename, "w+");

	// ###########TIME################
	// Work out the time difference from when the timer ended to when it started
	// long duration = *end - *begin;
	// Convert that difference into seconds by dividing by 10^9
	// double durSec = ((double)time)/1e9;

	fprintf(fp, "Seed, Time\n");

	fprintf(fp, "%02X %02X %02X %02X, %.10f seconds\n", data[0], data[1], data[2], data[3], time);

	fclose(fp);
	printf("\n\n%s saved\n", filename);
	// free(receiveStruct.endTime);
	// free(receiveStruct.beginTime);
}

int createSocket(char *interface) {
	// ##########SOCKET############
	// Boiler plate socket creation
	int s;
	// int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;

	// Interface name from 1st argument
	const char *ifname = interface;

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

	return s;
}
