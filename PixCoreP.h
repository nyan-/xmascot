#ifndef __PixCoreP_h
#define __PixCoreP_h

#include <X11/CoreP.h>
#include "image.h"
#include "PixCore.h"

/*
typedef struct{
} PixCoreClassPart;
*/

typedef struct _PixCoreClassRec{
	CoreClassPart core_class;
/*	PixCoreCassPart; */
} PixCoreClassRec;

extern PixCoreClassRec pixCoreClassRec;

typedef struct _PixCorePart {
	ImageData  *image;
	String filename;
} PixCorePart;

typedef struct _PixCoreRec {
	CorePart	core;
	PixCorePart pixcore;
} PixCoreRec;

#endif
