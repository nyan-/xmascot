/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
*/
#ifndef __CascadeMenuP_h
#define __CascadeMenuP_h

#ifndef XAW3D
#include <X11/Xaw/SimpleMenP.h>
#include <X11/Xaw/SmeBSBP.h>
#include <X11/Xaw/SmeLineP.h>
#include <X11/Xaw/SmeP.h>
#else
#include <X11/Xaw3d/SimpleMenP.h>
#include <X11/Xaw3d/SmeBSBP.h>
#include <X11/Xaw3d/SmeLineP.h>
#include <X11/Xaw3d/SmeP.h>
#endif

#include "CascadeMenu.h"

/*
typedef struct {
} CascadeMenuClassPart;
*/

typedef struct _CascadeMenuClassRec{
	CoreClassPart			core_class;
	CompositeClassPart		composite_class;
	ShellClassPart			shell_class;
	OverrideShellClassPart	override_shell_class;
	SimpleMenuClassPart		simpleMenu_Class;
/*	CascadeMenuClassPart	cascade_menu_clase; */
} CascadeMenuClassRec;

extern CascadeMenuClassRec cascadeMenuClassRec;

typedef struct _CascadeMenuPart {
	Widget child;
	SmeObject before;
	SmeObject calling;
	XtIntervalId timer;
	u_long delay;
	Widget parent;
} CascadeMenuPart;

typedef struct _CascadeMenuRec{
	CorePart            core;
	CompositePart       composite;
	ShellPart           shell;
	OverrideShellPart   override;
	SimpleMenuPart      simple_menu;
	CascadeMenuPart		cascade_menu;
} CascadeMenuRec;

#endif

