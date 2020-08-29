/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp

   SmeCascade Class for CascadeMenu
*/
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#ifndef XAW3D
#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/SimpleMenu.h>
#else
#include <X11/Xaw3d/XawInit.h>
#include <X11/Xaw3d/SimpleMenu.h>
#endif

#include "SmeCascadeP.h"

#define offset(field) XtOffsetOf(SmeCascadeRec, sme_cascade.field)
static XtResource resources[] = {
	{XtNmenu, XtCMenu, XtRWidget, sizeof(Widget),
		 offset(menu), XtRImmediate, NULL },
};
#undef offset

#define superclass (&smeBSBClassRec)
SmeCascadeClassRec smeCascadeClassRec = {
  {
    /* superclass         */    (WidgetClass)superclass,
	/* class_name         */    "SmeCascade",
    /* size               */    sizeof(SmeCascadeRec),
    /* class_initializer  */	XawInitializeWidgetSet,
    /* class_part_initialize*/	NULL,
    /* Class init'ed      */	FALSE,
	/* initialize         */    NULL,
    /* initialize_hook    */	NULL,
	/* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    0,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
	/* resize             */    NULL,
	/* expose             */    XtInheritExpose,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		      */    NULL,
    /* query_geometry	  */    XtInheritQueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	      */    NULL
  },{
    /* SimpleMenuClass Fields */
	/* highlight              */	XtInheritHighlight,
    /* unhighlight            */	XtInheritUnhighlight,
    /* notify                 */	XtInheritNotify,		
    /* extension	          */	NULL
  },
#ifdef XAW3D
  {
	  /* ThreeDClass Fields */
      /* shadowdraw         */    XtInheritXawSme3dShadowDraw
  },
#endif
  {
	/* BSBClass Fields */
    /* extension	      */    NULL
  }
};

WidgetClass smeCascadeObjectClass = (WidgetClass)&smeCascadeClassRec;
