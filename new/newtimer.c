#include <stdio.h>
#include <string.h>
#include <time.h>
#include "cSAR.h"

void timer(long *timer_val) {
	struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    *timer_val = tp.tv_sec * 1000000000 + tp.tv_nsec;
}

void makeCSV(char *filename, void *ptr) {
	FILE *fp;

	struct canInfoStruct *receiveStruct = ptr;
	int data3 = receiveStruct->cdata[3];
	int data4 = receiveStruct->cdata[4];
	int data5 = receiveStruct->cdata[5];
	int data6 = receiveStruct->cdata[6];
	long *begin = receiveStruct->beginTime;
	long *end = receiveStruct->endTime;

	filename = strcat(filename, ".csv");
	fp = fopen(filename, "w+");

	// ###########TIME################
	// Work out the time difference from when the timer ended to when it started
	long duration = *end - *begin;
	// Convert that difference into seconds by dividing by 10^9
	double durSec = ((double)duration)/1e9;

	fprintf(fp, "Seed, Time");

	fprintf(fp, "%02X %02X %02X %02X, %.10f seconds\n", data3, data4, data5, data6, durSec);

	fclose(fp);
	printf("\n\n%s.csv saved\n", filename);
	// free(receiveStruct.endTime);
	// free(receiveStruct.beginTime);
}
