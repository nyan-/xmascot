#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Core.h>

#include "PixCore.h"
#include "PixCoreP.h"

#include "image.h"

static void Initialize(Widget, Widget, ArgList args, Cardinal * num);
static void Destroy(Widget);
static void Realize(Widget, XtValueMask*, XSetWindowAttributes *);
static Boolean SetValues(Widget, Widget, Widget, ArgList, Cardinal *);

static XtResource resources[] = {
	{XtNfile, XtCFile, XtRString, sizeof(String),
		 XtOffsetOf(PixCoreRec,pixcore.filename), XtRImmediate, (String)NULL}
};

#define superclass (&coreClassRec)
PixCoreClassRec pixCoreClassRec = {
	{
		 /* superclass         */ (WidgetClass) superclass,
		 /* class_name         */ "PixCore",
		 /* size               */ sizeof(PixCoreRec),
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

WidgetClass pixCoreWidgetClass = (WidgetClass) & pixCoreClassRec;

#define FILENAME    (pcw->pixcore.filename)
#define IMAGE       (pcw->pixcore.image)
#define OLDFILENAME (old->pixcore.filename)
#define OLDIMAGE    (old->pixcore.image)

static void
Initialize(Widget req, Widget new, ArgList args, Cardinal * num)
{
	PixCoreWidget pcw = (PixCoreWidget)new;
	IMAGE    = image_new();
	if (FILENAME) {
		image_load(IMAGE,FILENAME,-1,-1);
		pcw->core.width  = IMAGE->width;
		pcw->core.height = IMAGE->height; 
	}
}

static void
Destroy(Widget widget)
{
	PixCoreWidget pcw = (PixCoreWidget)widget;
	image_delete(IMAGE);
}

static void
Realize(Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attr)
{
	PixCoreWidget pcw = (PixCoreWidget)widget;

	XtCreateWindow(widget, (u_int)InputOutput,
				   (Visual*)CopyFromParent, *value_mask, attr);

	if (IMAGE->data != NULL) {
		XSetWindowAttributes attributes;
		Pixmap p;

		image_set_col(IMAGE, XtDisplay(widget), XtWindow(widget));
		p = image_pixmap(IMAGE, NULL);
		attributes.background_pixmap = p;
		XChangeWindowAttributes(XtDisplay(widget), 
								XtWindow(widget), CWBackPixmap, &attributes);
	
		image_free_data(IMAGE);
		image_free_pixmap(IMAGE);
	}
}

static Boolean
SetValues(Widget cur, Widget req, Widget new, ArgList args, Cardinal *num_args)
{
	PixCoreWidget old = (PixCoreWidget)cur;
	PixCoreWidget pcw = (PixCoreWidget)new;
	Boolean redisplay = FALSE;
	
	if (OLDFILENAME != FILENAME) {

		image_delete(OLDIMAGE);

		IMAGE = image_new();
		image_load(IMAGE,FILENAME,-1,-1);
		pcw->core.width  = IMAGE->width;
		pcw->core.height = IMAGE->height; 

		if (XtIsRealized(cur)) {
			XSetWindowAttributes attributes;
			Pixmap p;

			image_set_col(IMAGE, XtDisplay(new), XtWindow(new));
			p = image_pixmap(IMAGE, NULL);
			attributes.background_pixmap = p;
			XChangeWindowAttributes(XtDisplay(new), XtWindow(new),
									CWBackPixmap, &attributes);
	
			image_free_data(IMAGE);
			image_free_pixmap(IMAGE);
			redisplay = TRUE;
		}
	}
	
	return redisplay;
}

	
