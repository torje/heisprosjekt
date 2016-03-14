#include "timer.h"
bool timespec_cmp( struct timespec *start, struct timespec *stop, double compare_to){
  int sec = stop->tv_sec - start->tv_sec;
  int nsec = stop->tv_nsec - start->tv_nsec;
  if ( nsec < 0 ) {
    sec -=1;
    nsec+=1000000000;
  }
  return compare_to < sec + nsec/1000000000.;
}
