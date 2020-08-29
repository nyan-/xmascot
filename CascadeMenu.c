/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
*/
#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#ifndef XAW3D
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSBP.h>
#include <X11/Xaw/SmeLineP.h>
#include <X11/Xaw/SmeP.h>
#else
#include <X11/Xaw3d/XawInit.h>
#include <X11/Xaw3d/Cardinals.h>
#include <X11/Xaw3d/SimpleMenP.h>
#include <X11/Xaw3d/SmeBSBP.h>
#include <X11/Xaw3d/SmeLineP.h>
#include <X11/Xaw3d/SmeP.h>
#endif

#include "SmeCascadeP.h"
#include "CascadeMenuP.h"

/* semi public */
static void     Initialize(Widget req, Widget new, ArgList args, Cardinal * num);

/* private */
static void     CascadePopupStart(Widget w, XEvent * ev, String * p, Cardinal * n);
static void     CascadePopup(XtPointer w, XtIntervalId * id);
static void     CascadePopdown(Widget w, XEvent * ev, String * p, Cardinal * n);

static XtActionsRec actions[] =
{
	{"cascade_popup", CascadePopupStart},
	{"cascade_popdown", CascadePopdown},
};

static char     translations[] =
"<EnterWindow>: highlight()\n"
"<LeaveWindow>: unhighlight()\n"
"<Motion>: highlight() cascade_popup()\n"
"<BtnUp>: cascade_popdown() notify() unhighlight()";

#define superclass (&simpleMenuClassRec)
CascadeMenuClassRec cascadeMenuClassRec = {
	{
		 /* superclass         */ (WidgetClass) superclass,
		 /* class_name         */ "CascadeMenu",
		 /* size               */ sizeof(CascadeMenuRec),
		 /* class_initialize   */ XawInitializeWidgetSet,
		 /* class_part_initialize */ NULL,
		 /* Class init'ed      */ FALSE,
		 /* initialize         */ Initialize,
		 /* initialize_hook    */ NULL,
		 /* realize            */ XtInheritRealize,
		 /* actions            */ actions,
		 /* num_actions        */ XtNumber(actions),
		 /* resources          */ NULL,
		 /* resource_count     */ 0,
		 /* xrm_class          */ NULLQUARK,
		 /* compress_motion    */ FALSE,
		 /* compress_exposure  */ FALSE,
		 /* compress_enterleave */ FALSE,
		 /* visible_interest   */ FALSE,
		 /* destroy            */ NULL,
		 /* resize             */ XtInheritResize,
		 /* expose             */ XtInheritExpose,
		 /* set_values         */ NULL,
		 /* set_values_hook    */ NULL,
		 /* set_values_almost  */ XtInheritSetValuesAlmost,
		 /* get_values_hook    */ NULL,
		 /* accept_focus       */ NULL,
		 /* intrinsics version */ XtVersion,
		 /* callback offsets   */ NULL,
		 /* tm_table		  	  */ translations,
		 /* query_geometry	  */ NULL,
		 /* display_accelerator */ NULL,
		 /* extension	      */ NULL
	}, {
		 /* geometry_manager   */ XtInheritGeometryManager,
		 /* change_managed     */ XtInheritChangeManaged,
		 /* insert_child	      */ XtInheritInsertChild,
		 /* delete_child	      */ XtInheritDeleteChild,
		 /* extension	      */ NULL
	}, {
		 /* Shell extension	  */ NULL
	}, {
		 /* Override extension */ NULL
	}, {
		 /* Simple Menu extension */ NULL
	}
};

WidgetClass     cascadeMenuWidgetClass = (WidgetClass) & cascadeMenuClassRec;

#define CHILD   cmw->cascade_menu.child
#define BEFORE  cmw->cascade_menu.before
#define CALLING cmw->cascade_menu.calling
#define TIMER   cmw->cascade_menu.timer
#define DELAY   200
#define PARENT  cmw->cascade_menu.parent

static void 
Initialize(Widget req, Widget new, ArgList args, Cardinal * num)
{
	CascadeMenuWidget cmw = (CascadeMenuWidget) new;
	CHILD = NULL;
	BEFORE = NULL;
	TIMER = None;
	PARENT = NULL;
}

static void 
CascadePopupStart(Widget w, XEvent * ev, String * p, Cardinal * n)
{
	CascadeMenuWidget cmw = (CascadeMenuWidget) w;
	SmeObject       entry = cmw->simple_menu.entry_set;
	Widget          menu;

	if (BEFORE != entry) {
		BEFORE = entry;
		if (TIMER) {
			XtRemoveTimeOut(TIMER);
			TIMER = None;
		}
		if (entry && XtIsSensitive((Widget) entry) &&
		    XtIsSubclass((Widget) entry, smeCascadeObjectClass)
		 && (menu = ((SmeCascadeObject) entry)->sme_cascade.menu)) {
			CALLING = entry;
			TIMER = XtAppAddTimeOut(XtWidgetToApplicationContext(w),
					DELAY, CascadePopup, (XtPointer) w);
		} else {
			if (CHILD) {
				XtPopdown(CHILD);
				CHILD = NULL;
			}
		}
	}
}

static void 
CascadePopup(XtPointer w, XtIntervalId * id)
{
	Position        x, y, my;
	CascadeMenuWidget cmw = (CascadeMenuWidget) w;
	SmeObject       entry = CALLING;
	Widget          menu = ((SmeCascadeObject) entry)->sme_cascade.menu;

	if (!XtIsRealized(menu))
		XtRealizeWidget(menu);

	if (CHILD && CHILD != menu)
		XtPopdown(CHILD);
	x = cmw->core.x + cmw->core.width;
	if (x > WidthOfScreen(XtScreen(cmw)) - menu->core.width)
		x = cmw->core.x - menu->core.width;
	y = cmw->core.y + entry->rectangle.y;
	if (y > (my = HeightOfScreen(XtScreen(cmw)) - menu->core.height))
		y = my;
	XtVaSetValues(menu, XtNx, (Position) x, XtNy, (Position) y, NULL);

	/* CascadeMenu なら、親情報をひきわたす */
	if (XtIsSubclass((Widget) menu, cascadeMenuWidgetClass))
		((CascadeMenuWidget) menu)->cascade_menu.parent = w;

	XtPopup(menu, XtGrabNonexclusive);
	CHILD = menu;
	TIMER = None;
}

static void 
CascadePopdown(Widget w, XEvent * ev, String * p, Cardinal * n)
{
	CascadeMenuWidget cmw = (CascadeMenuWidget) w;
	if (TIMER) {
		XtRemoveTimeOut(TIMER);
		TIMER = None;
	}
	if (CHILD) {
		XtPopdown(CHILD);
		CHILD = NULL;
	}
	if (PARENT) {
		XtPopdown(w);
		XtPopdown(PARENT);
		PARENT = NULL;
	} else
		XtPopdown(w);

}
