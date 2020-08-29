/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp

   SmeCascade Class for CascadeMenu
*/
#ifndef _XawSmeCascadeP_h
#define _XawSmeCascadeP_h

#ifndef XAW3D
#include <X11/Xaw/SmeBSBP.h>
#else
#include <X11/Xaw3d/SmeBSBP.h>
#endif
#include "SmeCascade.h"

/* class */

typedef struct{
	Pixmap default_right_bitmap;
} SmeCascadeClassPart;

typedef struct _SmeCascadeClassRec {
    RectObjClassPart	rect_class;
    SmeClassPart		sme_class;
#ifdef XAW3D
    SmeThreeDClassPart  sme_threeD_class;
#endif
    SmeBSBClassPart		sme_bsb_class;
	SmeCascadeClassPart	sme_cas_class;
} SmeCascadeClassRec;

extern SmeCascadeClassRec smeCascadeClassRec;

typedef Widget (*XtMakeMenuProc)(Widget w,XtPointer arg);

/* instance */
typedef struct {
	Widget menu;
} SmeCascadePart;

typedef struct _SmeCascadeRec {
    ObjectPart		object;
    RectObjPart		rectangle;
    SmePart			sme;
#ifdef XAW3D
    SmeThreeDPart   sme_threeD;
#endif
    SmeBSBPart		sme_bsb;
	SmeCascadePart	sme_cascade;
} SmeCascadeRec;

#endif /* _XawSmeCascade_h */
