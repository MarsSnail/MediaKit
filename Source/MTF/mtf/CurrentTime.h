#ifndef CurrentTime_h
#define CurrentTime_h

#include <time.h>

namespace MTF{

/*
  return the current UTC time in seconds, counted from the January 1, 1970
  The precision of varies depending on platform but is usually good or better than 
  a million second
*/
double currentTime();
/* same funciton other than returned value in million second*/
double currentTimeMS();

/*
   provides an  monotonically increaing time in second since an arbitrary ['ɑːbɪt(rə)rɪ] point in the past
   on unsupport platform ,this function only guarantees the result will be no-decreaing.
*/
double monotonicallyIncreasingTime();

/* return the current CPU time of the current thread in seconds.
   The precision of varies depending on platform but is usually good or better than a million second
*/
double currentCPUTime();
//return the current CPU time of the current thread in million seconds
double currentCPUTimeMS();

} // namespace MTF

using MTF::currentTime;
using MTF::currentTimeMS;
using MTF::monotonicallyIncreasingTime;
using MTF::currentCPUTime;
using MTF::currentCPUTimeMS;

#endif // CurrentTime_h
