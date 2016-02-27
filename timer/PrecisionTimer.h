#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/** @{
 * Making a timer/stopwatch from a monotonic clock
 */
struct Precision_Timer {
	/** Storing the resolution of the timer*/
	struct timespec res;
	/** start time of the timer*/
	struct timespec tp;
};
/**
 * Resetting and implicitly starting the clock
 */
void PT_start(struct Precision_Timer *pt);
/**
 * Checks if there has passed more seconds from pt than limit
 */
int PT_compare_double(struct Precision_Timer *pt, double limit);
void print_timeval(struct Precision_Timer *pt, FILE *file);


/**
 * @}
 */
