#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Core.h>

#include "AnimCore.h"
#include "AnimCoreP.h"

#include "animate.h"

static void Initialize(Widget, Widget, ArgList args, Cardinal * num);
static void Destroy(Widget);
static void Realize(Widget, XtValueMask*, XSetWindowAttributes *);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);

static XtResource resources[] = {
	{XtNfile, XtCFile, XtRString, sizeof(String),
		 XtOffsetOf(AnimCoreRec,animcore.filename), XtRImmediate, (String)NULL}
};

#define superclass (&coreClassRec)
AnimCoreClassRec animCoreClassRec = {
	{
		 /* superclass         */ (WidgetClass) superclass,
		 /* class_name         */ "AnimCore",
		 /* size               */ sizeof(AnimCoreRec),
		 /* class_initialize   */ NULL,
		 /* class_part_initialize */ NULL,
  	     /* Class init'ed      */ FALSE,
	     /* initialize         */ Initialize,
		 /* initialize_hook    */ NULL,
		 /* realize            */ Realize,
		 /* actions            */ NULL,
		 /* num_actions        */ 0,
		 /* resources          */ resources,
		 /* resource_count     */ XtNumber(resources),
		 /* xrm_class          */ NULLQUARK,
		 /* compress_motion    */ FALSE,
		 /* compress_exposure  */ FALSE,
		 /* compress_enterleave */ FALSE,
		 /* visible_interest   */ FALSE,
		 /* destroy            */ Destroy,
		 /* resize             */ XtInheritResize,
		 /* expose             */ XtInheritExpose,
		 /* set_values         */ SetValues,
		 /* set_values_hook    */ NULL,
		 /* set_values_almost  */ XtInheritSetValuesAlmost,
		 /* get_values_hook    */ NULL,
		 /* accept_focus       */ NULL,
		 /* intrinsics version */ XtVersion,
		 /* callback offsets   */ NULL,
		 /* tm_table		    */ NULL,
		 /* query_geometry	    */ NULL,
		 /* display_accelerator */ NULL,
		 /* extension	      */ NULL
  }
};

WidgetClass animCoreWidgetClass = (WidgetClass) & animCoreClassRec;

#define FILENAME    (pcw->animcore.filename)
#define ANIMATE     (pcw->animcore.animate)
#define TIMERID		(pcw->animcore.id)
#define OLDFILENAME (old->animcore.filename)
#define OLDANIMATE  (old->animcore.animate)
#define OLDTIMERID  (old->animcore.id)

static void
Initialize(Widget req, Widget new, ArgList args, Cardinal * num)
{
	AnimCoreWidget pcw = (AnimCoreWidget)new;
	ANIMATE = NULL;
	TIMERID = None;
	if (FILENAME) {
		FILE *fp;
		ANIMATE = animate_new();
		if ((fp = animate_fopen(FILENAME)) != NULL) {
			animate_parse(ANIMATE,fp);
			pcw->core.width  = ANIMATE->w;
			pcw->core.height = ANIMATE->h; 
			animate_fclose(fp);
		} else {
			animate_delete(ANIMATE);
			ANIMATE = NULL;
		}
	}
}

static void
Destroy(Widget widget)
{
	AnimCoreWidget pcw = (AnimCoreWidget)widget;
	if (TIMERID)
		XtRemoveTimeOut(TIMERID);
	animate_delete(ANIMATE);
}

static void
DrawPattern(XtPointer cl, XtIntervalId *id)
{
	AnimCoreWidget pcw = (AnimCoreWidget)cl;
	int interval;
	XtAppContext app = XtWidgetToApplicationContext((Widget)cl);
	interval = ANIMATE->point->interval;
	if (animate_go(ANIMATE))
		TIMERID = XtAppAddTimeOut(app, interval, DrawPattern, cl);
	else
		TIMERID = None;
}

static void
Realize(Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attr)
{
	AnimCoreWidget pcw = (AnimCoreWidget)widget;

	XtCreateWindow(widget, (u_int)InputOutput,
				   (Visual*)CopyFromParent, *value_mask, attr);

	if (ANIMATE != NULL) {
		animate_attach_win(ANIMATE, XtDisplay(widget), XtWindow(widget));
		animate_realize(ANIMATE);
		DrawPattern((XtPointer)widget,NULL);
	}
}

static Boolean
SetValues(Widget cur, Widget req, Widget new, ArgList args, Cardinal *num_args)
{
	AnimCoreWidget old = (AnimCoreWidget)cur;
	AnimCoreWidget pcw = (AnimCoreWidget)new;
	Boolean redisplay = FALSE;

	if (OLDFILENAME != FILENAME) {

		if (OLDTIMERID)
			XtRemoveTimeOut(OLDTIMERID);
		animate_delete(OLDANIMATE);

		ANIMATE = animate_new();
		if ((fp = animate_fopen(FILENAME)) != NULL) {
			animate_parse(ANIMATE,fp);
			pcw->core.width  = ANIMATE->w;
			pcw->core.height = ANIMATE->h; 
			animate_fclose(fp);

			if (XtIsRealized(cur)) {
				animate_attach_win(ANIMATE, 
								   XtDisplay(widget), XtWindow(widget));
				animate_realize(ANIMATE);
				DrawPattern((XtPointer)widget,NULL);
				redisplay = TRUE;
			}

		} else {
			animate_delete(ANIMATE);
			ANIMATE = NULL;
		}
	}
	return redisplay;
}

