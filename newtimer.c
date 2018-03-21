#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#include "cSAR.h"

void timer(long *timer_val) {
	struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    *timer_val = tp.tv_sec * 1000000000 + tp.tv_nsec;
}

// int randHex(void) {
// 	int length;
//     char output[2];
//     char str[] = "0123456789ABCDEF";

//     /* Seed number for rand() */
//     srand((unsigned int) time(0) + getpid());
//     length = rand() % 15 + 8;
 
//     for (int i = 0; i < 2; ++i) {
//         output[i] = str[rand() % 16];
//         srand(rand());
//     }

//     int final = (int)strtol(output, NULL, 16);

//     printf("%X\n", final);

//     return final;
// }