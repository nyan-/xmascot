/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <sys/types.h>
#include <time.h>
#include "xmascot.h"

/* for SunOS 4.1.X */
#ifdef NEED_DIFFTIME
double 
difftime(time_t a, time_t b)
{
	return (double) (a - b);
}
#endif

extern XtAppContext app;

/* ���顼��ǡ�����¤�� */
Alarm           alarm_dat[ALARM_ALLNUM];

static void     set_alarm(int num);

/* ���顼��ư������ؿ� (���������μ¹� & �����) */
static
void 
AlarmHand(XtPointer cl, XtIntervalId * id)
{
	long            num = (long) cl;
	action_parse(alarm_dat[num].action);
	set_alarm(num);
}

/* ���顼��������� */
static
void 
set_alarm(int num)
{
	time_t          t, u;
	struct tm      *tmpt;
	u_long          dt;

	int             hour = alarm_dat[num].hour;
	int             min = alarm_dat[num].min;

	t = time(NULL);
	tmpt = localtime(&t);	/* �������� */

	msg_out("set_alarm now: %s", asctime(tmpt));

	/* �᤮�Ƥ�Τǡ������� */
	if (tmpt->tm_hour > hour ||
	    (tmpt->tm_hour == hour && tmpt->tm_min >= min))
		tmpt->tm_mday++;

	tmpt->tm_hour = hour;
	tmpt->tm_min = min;
	u = mktime(tmpt);

	msg_out("set_alarm set: %s", asctime(tmpt));

	dt = difftime(u, t) * 1000;
	msg_out("difftime %ld\n", dt);

	alarm_dat[num].id
		= XtAppAddTimeOut(app, dt, AlarmHand, (XtPointer) num);
}

/* �������ѤΥϥ�ɥ� */
static
void 
AlarmHand2(XtPointer cl, XtIntervalId * id)
{
	long            num = (long) cl;
	action_parse(alarm_dat[num].action);
	alarm_dat[num].sw = 0;
	alarm_dat[num].id = -1;
}

/* �����ޡ������� */
static
void 
set_timer(int num)
{
	int             hour = alarm_dat[num].hour;
	int             min = alarm_dat[num].min;
	alarm_dat[num].id =
		XtAppAddTimeOut(app, (hour * 60 + min) * 1000, AlarmHand2, (XtPointer) num);
}

/* ���㥤��Υϥ�ɥ� */
static
void 
AlarmHand3(XtPointer cl, XtIntervalId * id)
{
	long            num = (long) cl;
	action_parse(alarm_dat[num].action);
	/* 1���ָ� */
	alarm_dat[num].id
		= XtAppAddTimeOut(app, 60 * 60 * 1000, AlarmHand3, cl);
}

/* ���㥤�����Ͽ */
static
void 
set_chime(int num, int minute)
{
	time_t          t, u;
	struct tm      *tmpt;
	u_long          dt;

	t = time(NULL);
	tmpt = localtime(&t);	/* �������� */
	/* �᤮�Ƥ�Τǡ����λ��� */

	msg_out("set_chime now: %s", asctime(tmpt));

	if (tmpt->tm_min >= minute)
		tmpt->tm_hour++;
	tmpt->tm_min = minute;
	u = mktime(tmpt);

	msg_out("set_chime set: %s", asctime(tmpt));

	dt = difftime(u, t) * 1000;

	msg_out("difftime %ld\n", dt);

	alarm_dat[num].id
		= XtAppAddTimeOut(app, dt, AlarmHand3, (XtPointer) num);
}

/* ���󥿡��Х륿�����ѤΥϥ�ɥ� */
static
void 
AlarmHand4(XtPointer cl, XtIntervalId * id)
{
	long            num = (long) cl;
	int             hour = alarm_dat[num].hour;
	int             min = alarm_dat[num].min;
	action_parse(alarm_dat[num].action);
	alarm_dat[num].id =
		XtAppAddTimeOut(app, (hour * 60 + min) * 60000, AlarmHand4, (XtPointer) num);
}

/* ���󥿡��Х륿���ޡ������� */
static
void 
set_inter(int num)
{
	int             hour = alarm_dat[num].hour;
	int             min = alarm_dat[num].min;

	if (hour > 0 || min > 0)
		alarm_dat[num].id =
			XtAppAddTimeOut(app, (hour * 60 + min) * 60000, AlarmHand4, (XtPointer) num);
}

/* ���顼�����Ͽ */
void
set_alarms(void)
{
	int             i;
	for (i = 0; i < ALARM_ALLNUM; i++) {
		/* �Ȥꤢ��������ư��Τ�Τ��� */
		if (alarm_dat[i].id != -1)
			XtRemoveTimeOut(alarm_dat[i].id);
		alarm_dat[i].id = -1;
		if (alarm_dat[i].sw) {
			switch (i) {
			case ALARM_NUM:
				set_timer(i);
				break;
			case ALARM_NUM + 1:
				set_inter(i);
				break;
			case ALARM_NUM + 2:
				set_chime(i, 0);
				break;
			case ALARM_NUM + 3:
				set_chime(i, 30);
				break;
			default:
				set_alarm(i);
			}
		}
	}
}
