#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "image.h"
#include "animate.h"
#include "namelist.h"

Atom wm_protocols[2];
XtAppContext app;

Animate *anim = NULL;

void
QuitMsg(Widget w, XEvent *e, String *p, Cardinal *n)
{
	if (e->xclient.data.l[0] == wm_protocols[0] ||
		e->xclient.data.l[0] == wm_protocols[1]) {
		exit(1);
/*		XtDestroyWidget(w); */
	}
}

/* bad function */
void
expose(Widget w, XEvent *e, String *p, Cardinal *n)
{
	XExposeEvent *ee = (XExposeEvent*)e;
	if (anim->dpy == ee->display && anim->win == ee->window)
		animate_expose(anim,ee->x, ee->y, ee->width, ee->height);
}

/* command-line options */
static XrmOptionDescRec options[] = {};

/* actions */
static XtActionsRec actions[] = {
	{"quitmsg", QuitMsg},
	{"expose", expose}
};

/* translations */
String trans_top =
"<Message>WM_PROTOCOLS: quitmsg()\n"
"<Expose>: expose()\n";

extern int verbose;

void
set_animate(char *name)
{
	FILE *fp;
	anim = animate_new();
	if ((fp = animate_fopen(name)) == NULL) {
		perror("animate_fopen");
		exit(1);
	}
	if (animate_parse(anim,fp)) {
		fprintf(stderr,"animate file load error.\n");
		exit(1);
	}
}

typedef struct TimerDat {
	struct TimerDat *next;
	Animate *animate;
	u_int interval;
} TimerDat, *TimerDatList;

TimerDat*
timer_new(void)
{
	TimerDat *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("timer_new");
		exit(1);
	}
	p->next  = NULL;
	p->animate = NULL;
	p->interval = 0;
	return p;
}

void
timer_delete(TimerDat *p)
{
	if (p)
		free(p);
}

void
timer_reentry(TimerDatList list, Animate *animate,
			  TimerDat *new, int interval)
{
	TimerDat *t = list->next;
	
	if (t != NULL) {
		/* search entry point */
		while (t->next != NULL) {
			if (t->interval > interval)
				break;
			interval -= t->interval;
			t = t->next;
		}
		/* $B%j%9%H$KDI2C(B */
		new->animate  = animate;
		new->next     = t->next;
		new->interval = interval;
		t->next->interval -= interval;
		t->next = new;
	} else {
		new->animate  = animate;
		new->next     = NULL;
		new->interval = interval;
		list->next = new;
	}
}

void
timer_entry(TimerDatList list, Animate *animate)
{
	TimerDat *new = timer_new();
	timer_reentry(list, animate, new, 0);
}

static void timer_go(TimerDatList list);

static volatile XtIntervalId id;

static void
TimerHand(XtPointer cl, XtIntervalId *id)
{
	timer_go((TimerDatList)cl);
}

static void
timer_go(TimerDatList list)
{
	TimerDat *p = list->next;
	int interval;

	if (list != NULL && (p = list->next) != NULL) {
		interval = p->animate->point->interval;
		fprintf(stderr,"interval:[%d]\n",interval);
		list->next = p->next;
		if (animate_go(p->animate)) {
			timer_reentry(list, p->animate, p, interval);
		} else {
			timer_delete(p);
		}

		if (list->next)
			id = XtAppAddTimeOut(app, list->next->interval, TimerHand, list);
	}
}

int
main(int argc, char **argv)
{
	Widget top;
	TimerDatList tlist = timer_new();
	verbose = 1;

	top = XtVaOpenApplication(&app, "AnimTest", options, XtNumber(options),
							  &argc, argv, NULL, 
							  sessionShellWidgetClass, NULL);

	if (argv[1]) {

		wm_protocols[0] = 
			XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW", False);
		wm_protocols[1] = 
			XInternAtom(XtDisplay(top), "WM_SAVEYOURSELF", False);

		XtAppAddActions(app, actions, XtNumber(actions));
		XtOverrideTranslations(top, XtParseTranslationTable(trans_top));
		XtVaSetValues(top, XtNwidth,(Dimension)100, 
					  XtNheight,(Dimension)100, NULL);
			   
		XtRealizeWidget(top);
		XSetWMProtocols(XtDisplay(top), XtWindow(top), wm_protocols, 2);

		set_animate(argv[1]);
		animate_attach_win(anim, XtDisplay(top), XtWindow(top));
		animate_realize(anim);
		animate_init_all(anim);

		fprintf(stderr,"timer\n");

		timer_entry(tlist,anim);
		timer_go(tlist);

		XtAppMainLoop(app);
	} else {
		fprintf(stderr, "usage: %s filename\n", argv[0]);
		exit(1);
	}
	exit(0);
}

