/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <X11/Intrinsic.h>
#include "xmascot.h"

extern XtAppContext app;
extern AppData  adat;		/* 基本リソース */

static int      stop_fl;
volatile int    time_fl;
volatile int    time_cnt;

static volatile XtIntervalId id;

static void 
TimerHand(XtPointer cl, XtIntervalId * i)
{
	if (!stop_fl) {
		time_cnt++;
		id = XtAppAddTimeOut(app, DT, TimerHand, NULL);
		if (time_cnt > adat.draw_timing)
			time_fl = 1;
	}
}

void 
start_timer(void)
{
	id = XtAppAddTimeOut(app, DT, TimerHand, NULL);
	stop_fl = 0;
	time_fl = 0;
}

void 
stop_timer(void)
{
	if (!stop_fl) {
		stop_fl = 1;
		XtRemoveTimeOut(id);
	}
}

void 
restart_timer(void)
{
	if (stop_fl) {
		stop_fl = 0;
		id = XtAppAddTimeOut(app, DT, TimerHand, NULL);
	}
}
