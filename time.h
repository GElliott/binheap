//
//  time.h
//  binheap
//
//  Created by Glenn Elliott on 3/8/14.
//  Copyright (c) 2014 Glenn Elliott. All rights reserved.
//

#ifndef binheap_time_h
#define binheap_time_h

#include <time.h>
#include <sys/time.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

typedef enum
{
	CLK_REALTIME,
	CLK_THREAD_CPUTIME
} hosttime_t;

int clk_gettime(hosttime_t clk_id, struct timespec* ts)
{
	int ret = -1;
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;
#endif
	
	__sync_synchronize();
	switch(clk_id)
	{
#ifndef __MACH__
		case CLK_REALTIME:
			ret = clock_gettime(CLOCK_REALTIME, ts);
			break;
		case CLK_THREAD_CPUTIME:
			ret = clock_gettime(CLOCK_THREAD_CPUTIME_ID, ts);
			break;
#else
		case CLK_REALTIME:
		case CLK_THREAD_CPUTIME:  /* temporary */
			host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
			clock_get_time(cclock, &mts);
			mach_port_deallocate(mach_task_self(), cclock);
			ts->tv_sec = mts.tv_sec;
			ts->tv_nsec = mts.tv_nsec;
			ret = 0;
			break;
#endif
	}
	__sync_synchronize();

	return ret;
}

#endif
