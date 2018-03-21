#include <time.h>
#include "cSAR.h"

void timer(long *timer_val) {
	struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    *timer_val = tp.tv_sec * 1000000000 + tp.tv_nsec;
}