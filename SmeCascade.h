/*
   Cascade Menu Ver 2.0
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp

   SmeCascade Class for CascadeMenu
*/
#ifndef _SmeCascade_h
#define _SmeCascade_h

#include <X11/Xmu/Converters.h>
#include <X11/Xaw/SmeBSB.h>

typedef struct _SmeCascadeClassRec    *SmeCascadeObjectClass;
typedef struct _SmeCascadeRec         *SmeCascadeObject;

#define XtNmenu "menu"
#define XtCMenu "Menu"

extern WidgetClass smeCascadeObjectClass;

#endif /* _SmeCascade_h */
