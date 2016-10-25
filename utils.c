#include "utils.h"
#include "time.h"


unsigned long long getCurrentSeconds(){
	return getCurrentMillis()/1000LL;
}

unsigned long long getCurrentMillis(){
	struct timespec curTime;
	clock_gettime(CLOCK_REALTIME, &curTime);
	return ((unsigned long long)curTime.tv_sec) * 1000LL + ((unsigned long long)curTime.tv_nsec)/ 1000000LL;
}