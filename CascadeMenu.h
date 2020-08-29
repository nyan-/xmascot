/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
*/
#ifndef __CascadeMenu_h
#define __CascadeMenu_h

#ifndef XAW3D
#include <X11/Xaw/SimpleMenu.h>
#else
#include <X11/Xaw3d/SimpleMenu.h>
#endif

typedef struct _CascadeMenuClassRec	*CascadeMenuWidgetClass;
typedef struct _CascadeMenuRec		*CascadeMenuWidget;

extern WidgetClass cascadeMenuWidgetClass;

#endif

