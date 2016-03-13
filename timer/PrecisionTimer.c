#include "PrecisionTimer.h"

struct Precision_Timer * PT_init(){
	struct Precision_Timer *pt = calloc(1 ,sizeof(struct Precision_Timer));
	clock_getres(CLOCK_MONOTONIC, & pt->res);
	return pt;
}

void PT_start(struct Precision_Timer *pt) {
}
int PT_compare_double(struct Precision_Timer *pt, double compareto){
	
	return 0;
}
void print_timeval(struct Precision_Timer *pt, FILE *file) {
	return;
}
