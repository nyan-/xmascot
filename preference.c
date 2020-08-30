/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

#ifndef XAW3D
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Toggle.h>
#else
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/AsciiText.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Scrollbar.h>
#include <X11/Xaw3d/Toggle.h>
#endif

#include "CascadeMenu.h"
#include "xmascot.h"

extern AppData  adat;		/* アプリケーションリソース */
extern Widget   top;

#ifdef BIFF
extern Widget   biff;
#endif

typedef struct {
	char           *name;
	char           *format;
	float           factor;
	float           data;
	float           offset;
	float           diff;
	Widget          text, bar;
	int             changed;
}               PrefDialog;

PrefDialog      param[] = {
	{"grav", "%4.2fG", 1, 0, 0, 0.01, NULL, NULL, 0},
	{"damp", "%4.2f ", 1, 0, 0, 0.01, NULL, NULL, 0},
#ifdef SHADOW
	{"shadow", "%4.0f ", 20, 0, 0, 1, NULL, NULL, 0},
#endif
	{"mag", "%4.1f ", 2.9, 0, 0.1, 0.1, NULL, NULL, 0},
	{"chain", "%4.0f ", 19, 0, 1, 1, NULL, NULL, 0},
};

static PrefDialog *pref;
static void 
set_pref(float data)
{
	char            str[20];
	sprintf(str, pref->format, data);
#if 0
	XtVaSetValues(pref->text, XtNlabel, XtNewString(str), NULL);
#endif
	XtVaSetValues(pref->text, XtNlabel, str, NULL);

	pref->data = data;
	XawScrollbarSetThumb(pref->bar, ((data - pref->offset) / pref->factor), 0);
	pref->changed = 0;
	pref++;
}

static int 
changed(void)
{
	if (pref->changed)
		return 1;
	pref++;
	return 0;
}

static float 
get_pref(void)
{
	float           ret = pref->data;
	pref++;
	return ret;
}

/* カスタマイズダイアログのセットアップ */
static void 
set_preference_dialog(Widget w, XtPointer dat, XtPointer call)
{
	pref = param;
	set_pref(adat.grav / 980.0);
	set_pref(adat.damping);
#ifdef SHADOW
	set_pref(adat.shadow);
#endif
	set_pref(adat.magnify);
	set_pref(adat.chain_num);
}

/* キャンセル */
static void 
popdown(Widget w, XtPointer dat, XtPointer call)
{
	XtPopdown(dat);
}

/* OK */
static void 
ok_preference(Widget w, XtPointer dat, XtPointer cal)
{
	int             reload_flag = 0;
	int             reload_chain = 0;

	XtPopdown(dat);

	pref = param;
	if (changed())
		adat.grav = get_pref() * 980;
	if (changed())
		adat.damping = get_pref();

#ifdef SHADOW
	if (changed()) {
		reload_flag = 1;
		adat.shadow = get_pref();
	}
#endif
	if (changed()) {
		reload_flag = 1;
		adat.magnify = get_pref();
	}
	if (changed()) {
		reload_chain = 1;
		create_chains(get_pref());
		RaiseAll();
	}
	if (reload_flag) {
		set_widget_pattern(top, adat.pin_pat, adat.pcol0, adat.prgb0);
#ifdef BIFF
		set_widget_pattern(biff, adat.biff_pat, adat.bcol0, adat.brgb0);
#endif
		set_mas(&adat);
	}
#ifdef USE_CHAINPAT
	if (reload_chain || reload_flag)
		set_chain_pat(adat.chain_pat, adat.ccol0, adat.crgb0);
#endif
	set_sim_param();
	reset_pos();
	set_pos();
}

/* スクロールバー処理用のコールバック */
static void 
bar_jump(Widget w, XtPointer dat, XtPointer a)
{
	char            str[20];
	float           data = *((float *) a);
	PrefDialog     *p = &param[(long) dat];
	sprintf(str, p->format, (p->data = data * p->factor + p->offset));
#if 0
	XtVaSetValues(p->text, XtNlabel, XtNewString(str), NULL);
#endif
	XtVaSetValues(p->text, XtNlabel, str, NULL);
	p->changed = 1;
}

static void 
bar_scroll(Widget w, XtPointer dat, XtPointer a)
{
	char            str[20];
	int             dir = (int) a;
	PrefDialog     *p = &param[(long) dat];
	if (dir < 0) {
		p->data -= p->diff;
		if (p->data < 0)
			p->data = 0;
	} else if (dir > 0) {
		p->data += p->diff;
		if (p->data > p->factor + p->offset)
			p->data = p->factor + p->offset;
	}
	sprintf(str, p->format, p->data);
#if 0
	XtVaSetValues(p->text, XtNlabel, XtNewString(str), NULL);
#endif
	XtVaSetValues(p->text, XtNlabel, str, NULL);
	XawScrollbarSetThumb(w, (p->data - p->offset) / p->factor, 0);
	p->changed = 1;
}

/* カスタマイズ用ダイアログの初期化 */
Widget 
preference_dialog(Widget top)
{
	int             i;
	char            name[20];
	PrefDialog     *p;

	Widget          base, form, box, ok, cancel;
	Widget          up;
	Widget          box_label, box_text, box_bar;

	base = XtVaCreatePopupShell("preferenceBase",
				    transientShellWidgetClass, top, NULL);
	XtAddCallback(base, XtNpopupCallback, set_preference_dialog, NULL);
	form = XtVaCreateManagedWidget("preference", formWidgetClass, base, NULL);
	up = XtVaCreateManagedWidget("title", labelWidgetClass, form, NULL);
	box_label = XtVaCreateManagedWidget("box", boxWidgetClass, form,
					    XtNfromVert, up,
				    XtNorientation, XtorientVertical, NULL);
	box_text = XtVaCreateManagedWidget("box", boxWidgetClass, form,
					   XtNfromVert, up,
					   XtNfromHoriz, box_label,
				    XtNorientation, XtorientVertical, NULL);
	box_bar = XtVaCreateManagedWidget("box", boxWidgetClass, form,
					  XtNfromVert, up,
					  XtNfromHoriz, box_text,
				    XtNorientation, XtorientVertical, NULL);
	up = box_label;
	for (i = 0; i < XtNumber(param); i++) {
		p = &param[i];
		sprintf(name, "%s%s", p->name, "_label");
		XtVaCreateManagedWidget(name, labelWidgetClass, box_label, NULL);
		sprintf(name, "%s%s", p->name, "_num");
		p->text = XtVaCreateManagedWidget(name, labelWidgetClass, box_text, NULL);
		sprintf(name, "%s%s", p->name, "_bar");
		p->bar = XtVaCreateManagedWidget(name, scrollbarWidgetClass, box_bar,
					 XtNorientation, XtorientHorizontal,
						 NULL);
		XtAddCallback(p->bar, XtNjumpProc, bar_jump, I2P(i));
		XtAddCallback(p->bar, XtNscrollProc, bar_scroll, I2P(i));
	}
	box = XtVaCreateManagedWidget("box", boxWidgetClass, form,
				      XtNfromVert, up,
				  XtNorientation, XtorientHorizontal, NULL);
	ok = XtVaCreateManagedWidget("ok", commandWidgetClass, box, NULL);
	XtAddCallback(ok, XtNcallback, ok_preference, base);
	cancel = XtVaCreateManagedWidget("cancel", commandWidgetClass, box, NULL);
	XtAddCallback(cancel, XtNcallback, popdown, base);
	return base;
}
