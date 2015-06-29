#include "CurrentTime.h"

#define OS_LINUX

#ifdef OS_LINUX
#include <sys/time.h>
#else 
#endif // OS_LINUX

namespace MTF{

#ifdef OS_LINUX
double currentTime(){
	struct timeval tv;
	gettimeofday(&tv, 0);
	return static_cast<double>(tv.tv_sec)+static_cast<double>(tv.tv_usec)/1000000.0;
}

double monotonicallyIncreasingTime(){
	//static_cast<double>(g_get_monotonic_time()/1000000.0);
	return currentTime();
}

double currentCPUTime(){
	double firstTime = currentTime();
	return ( currentTime()-firstTime );
}
#else

#endif // OS_LINUX

double currentTimeMS(){
	return currentTime()*1000.0;
}

double currentCPUTimeMS(){
	return currentCPUTime()*1000.0;
}

} // namespace WTF
