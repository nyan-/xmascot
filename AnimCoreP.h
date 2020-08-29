#ifndef __AnimCoreP_h
#define __AnimCoreP_h

#include <X11/CoreP.h>
#include "animate.h"
#include "AnimCore.h"

/*
typedef struct{
} AnimCoreClassPart;
*/

typedef struct _AnimCoreClassRec{
	CoreClassPart core_class;
/*	AnimCoreCassPart animcore_class; */
} AnimCoreClassRec;

extern AnimCoreClassRec animCoreClassRec;

typedef struct _AnimCorePart {
	Animate    *animate;
	String filename;
	XtIntervalId id;
} AnimCorePart;

typedef struct _AnimCoreRec {
	CorePart	core;
	AnimCorePart animcore;
} AnimCoreRec;

#endif
