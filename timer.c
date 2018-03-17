//
// Created by shmink on 17/03/18.
//

#include <time.h>
#include <stdio.h>
#include <zconf.h>
#include "cSAR.h"

void timer(int bool) {
    // const clock_t begin, end;
    
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
// = tp.tv_nsec
    long begin;
    long end; /* nanoseconds divide by 1000 to get microseconds*/

    // printf("%ld\n", tp.tv_nsec);
    // printf("%.9ld\n", tp.tv_nsec/1000);
    // sleep(1);

    if(bool == 1) {
        //clock_gettime(CLOCK_REALTIME, &tp);
        end = tp.tv_nsec;

        printf("begin = %.9ld\n", begin);
        printf("end = %.9ld\n", end);
        printf("final = %.9ld\n", (end-begin));
        //*(clock_t *)&end = clock();
        // long double time_spent = (long double)(end - begin) / CLOCKS_PER_SEC;
        // printf("=============" "\x1b[32m" "TIME" "\x1b[0m" "============\n");
        // printf("begin = %Lf\n", (long double)begin);
        // printf("end = %Lf\n", (long double)end);
        // printf("final = %Lf\n", (long double)(end - begin));
        // printf("final divided = %Lf\n", time_spent);
    }

    if(bool == 0) {
        // *(clock_t *)&begin = clock();
        //clock_gettime(CLOCK_REALTIME, &tp);
        begin = tp.tv_nsec;
        printf("begin at start = %.9ld\n", begin);
    }
}

int main(void) {
    timer(0);
    sleep(1);
    timer(1);
}
