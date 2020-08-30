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

#include "SmeCascade.h"
#include "CascadeMenu.h"
#include "PixCore.h"

#include "xmascot.h"

extern Pixmap   select_mark;	/* 選択マーク           */
extern Pixmap   icon;		/* アイコンのパターン   */

extern AppData  adat;

extern Alarm    alarm_dat[];

/* メニューに線を追加 */
void 
menu_add_line(Widget menu)
{
	XtVaCreateManagedWidget("line", smeLineObjectClass, menu, NULL);
}

/* メニューにカスケードメニューを追加 */
Widget 
menu_add_cascade(Widget menu, char *title, Widget cascade)
{
	Widget entry 
		= XtVaCreateManagedWidget(title, smeCascadeObjectClass, menu,
								  XtNmenu, cascade, NULL);
	return entry;
}

/* メニューの場所ぎめ */
static void 
menu_setpos(Widget ww)
{
	Dimension       w, h;
	Window          root, child;
	int             rx, ry;
	int             wx, wy;
	unsigned        mask;

	if (!XtIsRealized(ww))
		XtRealizeWidget(ww);

	XtVaGetValues(ww, XtNwidth, &w, XtNheight, &h, NULL);
	/* 現在のマウスのポイントを得る */
	XQueryPointer(XtDisplay(ww), XtWindow(ww),
		      &root, &child, &rx, &ry, &wx, &wy, &mask);
	rx -= w / 2;
	ry -= h / 2;

	if (rx < 0)
		rx = 0;
	else if (rx > (wx = WidthOfScreen(XtScreen(ww)) - w))
		rx = wx;
	if (ry < 0)
		ry = 0;
	else if (ry > (wy = HeightOfScreen(XtScreen(ww)) - h))
		ry = wy;
	XtVaSetValues(ww, XtNx, (Position) rx, XtNy, (Position) ry, NULL);
}

/* 通常のダイアログ用の呼び出しコールバック */
static void 
popup_menu(Widget w, XtPointer dat, XtPointer call)
{
	Widget          m = (Widget) dat;
	menu_setpos(m);
	XtPopup(m, XtGrabNone);
}

/* メニューに通常のダイアログを追加 */
Widget 
menu_add_dialog(Widget menu, char *title, Widget dialog)
{
	Widget entry 
		= XtVaCreateManagedWidget(title, smeBSBObjectClass, menu, NULL);
	XtAddCallback(entry, XtNcallback, popup_menu, (XtPointer) dialog);
	return entry;
}

/* メニューにコールバックを追加 */
Widget 
menu_add_callback(Widget menu, char *title, XtCallbackProc callback)
{
	Widget entry 
		= XtVaCreateManagedWidget(title, smeBSBObjectClass, menu, NULL);
	XtAddCallback(entry, XtNcallback, callback, NULL);
	return entry;
}

/* キャンセル用コールバック */
static void 
popdown(Widget w, XtPointer dat, XtPointer a)
{
	XtPopdown(dat);
}


/* About ダイアログ生成 */
Widget 
about_dialog(Widget top)
{
	Widget          base, form, cmd;
	base = XtVaCreatePopupShell("aboutBase",
				    transientShellWidgetClass, top, NULL);
	form = XtVaCreateManagedWidget("about", boxWidgetClass, base, NULL);
	XtVaCreateManagedWidget("logo", pixCoreWidgetClass, form, NULL);
	XtVaCreateManagedWidget("label1", labelWidgetClass, form,
							XtNleftBitmap,icon, XtNheight,(Dimension)40, NULL);
	XtVaCreateManagedWidget("label2", labelWidgetClass, form, NULL);
	cmd = XtVaCreateManagedWidget("ok", commandWidgetClass, form, NULL);
	XtAddCallback(cmd, XtNcallback, popdown, base);
	return base;
}

static void 
set_change_menu(Widget w, XtPointer dat, XtPointer call)
{
	int             i, num = (int) dat;
	Mascot         *mas = adat.mascot_menus[num].mascots;
	int             n   = adat.mascot_menus[num].n_mascots;
	if (num == 0 && mas[n].entry != NULL)
		n++;

	if (num != adat.menu_no) {
		for (i = 0; i < n; i++)
			XtVaSetValues(mas[i].entry, XtNleftBitmap, None, NULL);
	} else {
		for (i = 0; i < n; i++)
			if (i == adat.mascot_number)
				XtVaSetValues(mas[i].entry, XtNleftBitmap, select_mark, NULL);
			else
				XtVaSetValues(mas[i].entry, XtNleftBitmap, None, NULL);
	}
}

/* 変更用メニュー 生成 */
Widget 
change_menu(Widget top, int num)
{
	long            i;
	Widget          menu, entry;
	char            name[20];
	Mascot         *mas;
	int             n;

	mas = adat.mascot_menus[num].mascots;
	n   = adat.mascot_menus[num].n_mascots;

	menu = XtVaCreatePopupShell("change", cascadeMenuWidgetClass, top, NULL);
	XtAddCallback(menu, XtNpopupCallback, set_change_menu, I2P(num));
	XtVaCreateManagedWidget("line", smeLineObjectClass, menu, NULL);
	for (i = 0; i < n; i++) {
		sprintf(name, "mascot%ld", i);
		entry = XtVaCreateManagedWidget(name, smeBSBObjectClass, menu, NULL);
		XtAddCallback(entry, XtNcallback, change_mascot_with_sound,
					  (XtPointer) ((num << 8) + i));
		XtVaSetValues(entry, XtNleftMargin, (Dimension) 20, NULL);
		if (mas[i].title == NULL) {
			XtVaSetValues(entry, XtNlabel, (String) "None",
						  XtNsensitive, False, NULL);
		} else {
			XtVaSetValues(entry, XtNlabel, (String) mas[i].title,
						  XtNsensitive,
					  (search(mas[i].fname) == NULL) ? False : True, NULL);
		}
		mas[i].entry = entry;
	}

	/* 一つめのメニューのみ */
	/* コマンドラインから指定したもの */
	if (num == 0) {
		if (mas[i].title != NULL) {
			XtVaCreateManagedWidget("line2", smeLineObjectClass, menu, NULL);
			sprintf(name, "mascot%ld", i);
			entry = XtVaCreateManagedWidget(name,
											smeBSBObjectClass, menu, NULL);
			XtAddCallback(entry, XtNcallback, change_mascot_with_sound,
						  (XtPointer) i);
			XtVaSetValues(entry, XtNleftMargin, (Dimension) 20, NULL);
			XtVaSetValues(entry, XtNlabel, mas[i].title, XtNsensitive,
						  (search(mas[i].fname) == NULL) ? False : True, NULL);
			mas[i].entry = entry;
		} else
			mas[i].entry = NULL;
	}
	return menu;
}

/* アラームの設定 */

Widget alarm_sw[ALARM_ALLNUM];
Widget alarm_hour[ALARM_ALLNUM];
Widget alarm_min[ALARM_ALLNUM];
Widget alarm_act[ALARM_ALLNUM];

#ifdef BIFF
extern String   biff_action;
Widget          biff_act;
#endif

void 
set_new_string(String * s, String new)
{
	if (*s != NULL)
		XtFree(*s);
	*s = XtNewString(new);
}

/* アラームダイアログの設定 */
void 
set_alarm_dialog(Widget w, XtPointer dat, XtPointer call)
{
	int             i;
	char            str[20];
	for (i = 0; i < ALARM_ALLNUM; i++) {
		if (alarm_dat[i].sw)
			XtVaSetValues(alarm_sw[i], XtNstate, True, NULL);
		else
			XtVaSetValues(alarm_sw[i], XtNstate, False, NULL);
		/* 時刻の設定がある */
		if (i <= ALARM_NUM + 1) {
			sprintf(str, "%02d", alarm_dat[i].hour);
#if 0
			XtVaSetValues(alarm_hour[i], XtNstring, XtNewString(str), NULL);
			sprintf(str, "%02d", alarm_dat[i].min);
			XtVaSetValues(alarm_min[i], XtNstring, XtNewString(str), NULL);
#endif
			XtVaSetValues(alarm_hour[i], XtNstring, str, NULL);
			sprintf(str, "%02d", alarm_dat[i].min);
			XtVaSetValues(alarm_min[i], XtNstring, str, NULL);
			XtVaSetValues(alarm_hour[i], XtNinsertPosition, (Position)2, NULL);
			XtVaSetValues(alarm_min[i], XtNinsertPosition, (Position)2, NULL);
		}
		if (alarm_dat[i].action != NULL)
#if 0
			XtVaSetValues(alarm_act[i], XtNstring,
				    XtNewString(alarm_dat[i].action), NULL);
#endif
		    XtVaSetValues(alarm_act[i], XtNstring,
				    alarm_dat[i].action, NULL);

		else
#if 0
			XtVaSetValues(alarm_act[i], XtNstring, XtNewString(""), NULL);
#endif
			XtVaSetValues(alarm_act[i], XtNstring, "", NULL);
	}
#ifdef BIFF
	/* for biff */
	if (biff_action != NULL)
#if 0
		XtVaSetValues(biff_act, XtNstring, XtNewString(biff_action), NULL);
#endif
		XtVaSetValues(biff_act, XtNstring, biff_action, NULL);
	else
#if 0
		XtVaSetValues(biff_act, XtNstring, XtNewString(""), NULL);
#endif
		XtVaSetValues(biff_act, XtNstring, "", NULL);
#endif
}

/* アラームの設定完了 */
static void 
ok_alarm(Widget w, XtPointer dat, XtPointer a)
{
	int             i, h, m;
	Boolean         sw;
	String          str;

	XtPopdown(dat);
	for (i = 0; i < ALARM_ALLNUM; i++) {
		XtVaGetValues(alarm_sw[i], XtNstate, &sw, NULL);
		alarm_dat[i].sw = (sw == True);
		/* 時刻の設定がある */
		if (i <= ALARM_NUM + 1) {
			XtVaGetValues(alarm_hour[i], XtNstring, &str, NULL);
			h = atoi(str);
			XtVaGetValues(alarm_min[i], XtNstring, &str, NULL);
			m = atoi(str);
			if (h >= 24 || h < 0)
				h = 0;
			if (m >= 60 || m < 0)
				m = 0;
			alarm_dat[i].hour = h;
			alarm_dat[i].min = m;
		}
		XtVaGetValues(alarm_act[i], XtNstring, &str, NULL);
		set_new_string(&alarm_dat[i].action, str);
	}

#ifdef BIFF
	/* for biff */
	XtVaGetValues(biff_act, XtNstring, &str, NULL);
	set_new_string(&biff_action, str);
#endif
	put_rcfile();
	set_alarms();
}

/* アラームの動作試験 */
static void 
test_alarm(Widget w, XtPointer dat, XtPointer a)
{
	String          str;
	Widget          w2 = (Widget) dat;
	XtVaGetValues(w2, XtNstring, &str, NULL);
	action_parse(str);
}

/* アラームダイアログの設定用 */
static Widget 
setup(Widget form, Widget upper, char *title, int i, int mode)
{
	Widget          test;

	upper = XtVaCreateManagedWidget(title, labelWidgetClass, form,
					XtNfromVert, upper, NULL);
	upper = XtVaCreateManagedWidget("box", boxWidgetClass, form,
					XtNorientation, "horizontal",
					XtNfromVert, upper, NULL);
	alarm_sw[i] = XtVaCreateManagedWidget("sw", toggleWidgetClass, upper,
					      XtNbitmap, select_mark, NULL);
	if (mode) {
		alarm_hour[i] = XtVaCreateManagedWidget("hour", asciiTextWidgetClass,
					    upper, XtNeditType, XawtextEdit,
					     XtNtype, XawAsciiString, NULL);
		XtVaCreateManagedWidget("sep", labelWidgetClass, upper,
					XtNlabel, (String) ":",
					XtNborderWidth, (Dimension) 0,
				     XtNinternalWidth, (Dimension) 0, NULL);
		alarm_min[i] = XtVaCreateManagedWidget("min", asciiTextWidgetClass,
					    upper, XtNeditType, XawtextEdit,
					     XtNtype, XawAsciiString, NULL);
	}
	alarm_act[i] = XtVaCreateManagedWidget("action", asciiTextWidgetClass,
				     upper, XtNeditType, XawtextEdit, NULL);
	test = XtVaCreateManagedWidget("test", commandWidgetClass, upper, NULL);
	XtAddCallback(test, XtNcallback, test_alarm, (XtPointer) alarm_act[i]);
	return upper;
}

/* アラームダイアログ */
Widget 
alarm_dialog(Widget top)
{
	int             i;
	Widget          base, form, test, upper;

	base = XtVaCreatePopupShell("alarmBase",
				    transientShellWidgetClass, top, NULL);
	XtAddCallback(base, XtNpopupCallback, set_alarm_dialog, NULL);
	form = XtVaCreateManagedWidget("alarm", formWidgetClass, base, NULL);
	upper = XtVaCreateManagedWidget("title", labelWidgetClass, form, NULL);
	for (i = 0; i < ALARM_NUM; i++)
		upper = setup(form, upper, "alarm", i, 1);
	upper = setup(form, upper, "timer", ALARM_NUM, 1);
	upper = setup(form, upper, "interval", ALARM_NUM + 1, 1);
	upper = setup(form, upper, "hourchime", ALARM_NUM + 2, 0);
	upper = setup(form, upper, "halfchime", ALARM_NUM + 3, 0);

#ifdef BIFF
	/* for biff */
	upper = XtVaCreateManagedWidget("biff", labelWidgetClass, form,
					XtNfromVert, upper, NULL);
	upper = XtVaCreateManagedWidget("biff_box", boxWidgetClass, form,
					XtNorientation, "horizontal",
					XtNfromVert, upper, NULL);
	biff_act = XtVaCreateManagedWidget("action", asciiTextWidgetClass, upper,
					   XtNeditType, XawtextEdit, NULL);
	test = XtVaCreateManagedWidget("test", commandWidgetClass, upper, NULL);
	XtAddCallback(test, XtNcallback, test_alarm, (XtPointer) biff_act);
#endif

	/* OK/CANCEL */
	upper = XtVaCreateManagedWidget("ok_box", boxWidgetClass, form,
					XtNorientation, XtorientHorizontal,
					XtNfromVert, upper, NULL);
	test = XtVaCreateManagedWidget("ok", commandWidgetClass, upper, NULL);
	XtAddCallback(test, XtNcallback, ok_alarm, base);
	test = XtVaCreateManagedWidget("cancel", commandWidgetClass, upper, NULL);
	XtAddCallback(test, XtNcallback, popdown, base);
	return base;
}
