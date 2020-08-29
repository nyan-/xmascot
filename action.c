/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xmascot.h"

extern XtAppContext app;
extern Atom wm_protocols[2];

extern Widget top, mascot, *chain;

#ifdef BIFF
extern Widget biff;
extern int mbox_flag;
#endif

extern AppData  adat;		/* リソース群 */

#ifdef USE_DOUBLE
extern double   th;		/* 角度           */
extern double   om;		/* 角速度         */
#else
extern int      th;		/* 角度           */
extern int      om;		/* 角速度         */
#endif


/* マスコットの変更 */
void
change_mascot(Widget w, XtPointer dat, XtPointer call)
{
	adat.menu_no = (int) dat >> 8;
	adat.mascot_number = (int) dat & 0xff;
	set_mas(&adat);
	set_sim_param();
	reset_pos();		/* 再描画 */
	set_pos();
}

/* 音声つきマスコット変更 */
void
change_mascot_with_sound(Widget w, XtPointer dat, XtPointer call)
{
	change_mascot(w, dat, call);
#ifdef SOUND
	xmascot_sound(&adat, SOUND_START);
#endif
}

/* 全体を raise する */
void
RaiseAll(void)
{
	int             i;
	for (i = 0; i < adat.chain_num; i++)
		XRaiseWindow(XtDisplay(chain[i]), XtWindow(chain[i]));
	XRaiseWindow(XtDisplay(top), XtWindow(top));
	XRaiseWindow(XtDisplay(mascot), XtWindow(mascot));
#ifdef BIFF
	if (mbox_flag)
		XRaiseWindow(XtDisplay(biff), XtWindow(biff));
#endif
}

/* 終了処理 */
void
ExitApp(void)
{
#ifdef SOUND
	xmascot_sound(&adat, SOUND_END);
#endif
	put_rcfile();
	exit(0);
}

/* 終了処理 */
void
Quit(Widget w, XEvent * e, String * p, Cardinal * n)
{
	ExitApp();
}

/* 終了メッセージの受理 */
void
QuitMsg(Widget w, XEvent * e, String * p, Cardinal * n)
{
	if (e->xclient.data.l[0] == wm_protocols[0] ||
	    e->xclient.data.l[0] == wm_protocols[1]) {
		ExitApp();
	}
}

int             map_fl = 0;	/* 現在マスコットはマップされているか？ */

/* マッピング */
void
MapWin(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef SOUND
	static int      firstmap = 0;
#endif
	int             i;
	set_sim_param();
	reset_pos();
	set_pos();
	for (i = 0; i < adat.chain_disp_num; i++)
		XtMapWidget(chain[i]);
	XtMapWidget(mascot);
#ifdef BIFF
	if (mbox_flag) {
		XtMapWidget(biff);
	}
#endif
	start_timer();
	RaiseAll();
#ifdef SOUND
	if (firstmap == 0) {
		firstmap = 1;
		xmascot_sound(&adat, SOUND_START);
	}
#endif
	map_fl = 1;
}

/* アンマップ */
void
UnMapWin(Widget w, XEvent * e, String * p, Cardinal * n)
{
	int             i;
	stop_timer();
	for (i = 0; i < adat.chain_num; i++)
		XtUnmapWidget(chain[i]);
	XtUnmapWidget(mascot);
#ifdef BIFF
	XtUnmapWidget(biff);
#endif
	map_fl = 0;
}

int             px, py;		/* pin の geometry */

/* 場所の移動 */
void
ConfigWin(Widget w, XEvent * e, String * p, Cardinal * n)
{
	Window          dmy;
	Dimension       pin_w, pin_h;
	Display        *dpy = XtDisplay(w);

	XtVaGetValues(w, XtNwidth, &pin_w, XtNheight, &pin_h, NULL);
	XTranslateCoordinates(dpy, XtWindow(w), RootWindowOfScreen(XtScreen(w)),
			      0, 0, &px, &py, &dmy);
	px += pin_w / 2;
	py += pin_h / 2;
	set_pos();
}

static int      motion_fl;	/* ドラッグしたか?             */

/* ボタンを押した */
void
Press(Widget w, XEvent * e, String * p, Cardinal * n)
{
	stop_timer();		/* 振子のシミュレーションを停止 */
	motion_fl = 0;

	XDefineCursor(XtDisplay(w), XtWindow(w), adat.cursor_click);
	RaiseAll();
}

/* マスコット ボタンを離した */
void
ReleaseMascot(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef USE_DOUBLE
	om = (motion_fl) ? 0 : om + ((om < 0) ? -1 : 1) * M_PI;
#else
	om = (motion_fl) ? 0 : om + ((om < 0) ? -1 : 1) * M_PI * 256;
#endif
	XtUngrabPointer(w, CurrentTime);

	XDefineCursor(XtDisplay(w), XtWindow(w), adat.cursor_normal);
	restart_timer();	/* 振子のシミュレーションの再開 */
}

/* Pin ボタンを離した */
void
ReleasePin(Widget w, XEvent * e, String * p, Cardinal * n)
{
	XtUngrabPointer(w, CurrentTime);
	XDefineCursor(XtDisplay(w), XtWindow(w), adat.cursor_normal);
	restart_timer();	/* 振子のシミュレーションの再開 */
}

/* Mascot の移動 */
void
MotionMascot(Widget w, XEvent * e, String * p, Cardinal * n)
{
	XEvent          ev;
	Window          root, child;
	int             rx, ry, x, y;
	u_int           key;

	if (!motion_fl) {
		XtGrabPointer(w, False, ButtonReleaseMask | Button1MotionMask,
					  GrabModeAsync, GrabModeAsync,
					  None, adat.cursor_drag, CurrentTime);
		motion_fl = 1;
	}
	while (XCheckMaskEvent(XtDisplay(w), Button1MotionMask, &ev));
	if (!XQueryPointer(XtDisplay(w), XtWindow(w), &root, &child,
			   &rx, &ry, &x, &y, &key))
		return;
	x = rx - px;
	y = ry - py;
	th = atan2(x, y) / RAD;
	adat.chain_len = sqrt((double) (x * x + y * y));
	set_sim_param();
	set_pos();
}

/* PIN の移動 */
void
MotionPin(Widget w, XEvent * e, String * p, Cardinal * n)
{
	XEvent          ev;
	Window          root, child;
	Display        *dpy = XtDisplay(w);
	int             rx, ry, x, y;
	u_int           key;
	Dimension       pin_w, pin_h;

	if (!motion_fl) {
		XtGrabPointer(w, False, ButtonReleaseMask | Button1MotionMask,
					  GrabModeAsync, GrabModeAsync,
					  None, adat.cursor_drag, CurrentTime);
		motion_fl = 1;
	}
	while (XCheckMaskEvent(dpy, Button1MotionMask, &ev));
	if (!XQueryPointer(dpy, XtWindow(top), &root, &child,
					   &rx, &ry, &x, &y, &key))
		return;
	XtVaGetValues(w, XtNwidth, &pin_w, XtNheight, &pin_h, NULL);
	XtMoveWidget(w, rx - pin_w / 2, ry - pin_h / 2);
}


/* アクション マスコットの変更 指定した番号 */
void
ChangeMascot(Widget w, XEvent * e, String * p, Cardinal * n)
{
	long            m, i;
	if (*n == 1) {
		i = atoi(p[0]);
		if (i >= 0 && i < adat.mascot_menus[0].n_mascots)
			change_mascot(w, (XtPointer) i, NULL);
	} else if (*n == 2) {
		i = atoi(p[0]);
		m = atoi(p[1]);
		if (i >= 0 && i < adat.mascot_menus[m].n_mascots)
			change_mascot(w, (XtPointer) ((m << 8) + i), NULL);
	}
}

/* アクション マスコットの変更 ファイルを読み込む */
void
ChangeMascotFile(Widget w, XEvent * e, String * p, Cardinal * n)
{
	if (*n > 0) {
		MascotMenu *m = &adat.mascot_menus[0];
		m->mascots[m->n_mascots].fname = p[0];
		m->mascots[m->n_mascots].title = (*n > 2) ? p[1] : p[0];
		change_mascot(w, (XtPointer)(m->n_mascots), NULL);
	}
}

/* アクション マスコットの変更 次のエントリ */
void
ChangeMascotNext(Widget w, XEvent * e, String * p, Cardinal * n)
{
	MascotMenu *m = &adat.mascot_menus[adat.menu_no];
	long i = adat.mascot_number + 1;
	if (i >= m->n_mascots)
		i = 0;
	change_mascot(w, (XtPointer) ((adat.menu_no << 8) + i), NULL);
}

/*
 * アクション マスコットの変更 次のエントリ
 * (全メニュー)
 */
void
ChangeMascotNextAll(Widget w, XEvent * e, String * p, Cardinal * n)
{
	MascotMenu *m = &adat.mascot_menus[adat.menu_no];
	long mn = adat.menu_no;
	long i = adat.mascot_number + 1;
	if (i >= m->n_mascots) {
		i = 0;
		mn++;
		if (mn >= adat.menus_num) {
			mn = 0;
		}
	}
	change_mascot(w, (XtPointer) ((mn << 8) + i), NULL);
}

/* アクション マスコットの変更 ランダム */
void
ChangeMascotRandom(Widget w, XEvent * e, String * p, Cardinal * n)
{
	MascotMenu *m = &adat.mascot_menus[adat.menu_no];
	long i = rand() % m->n_mascots;
	change_mascot(w, (XtPointer) ((adat.menu_no << 8) + i), NULL);
}

/* アクション マスコットの変更 ランダム 全メニュー */
void
ChangeMascotRandomAll(Widget w, XEvent * e, String * p, Cardinal * n)
{
	long            m = rand() % adat.menus_num;
	long            i = rand() % adat.mascot_menus[m].n_mascots;
	change_mascot(w, (XtPointer) ((m << 8) + i), NULL);
}

/* アクション 揺れる */
void
StartMove(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef USE_DOUBLE
	om = om + ((om < 0) ? -1 : 1) * M_PI;
#else
	om = om + ((om < 0) ? -1 : 1) * M_PI * 256;
#endif
	restart_timer();
}

/* アクション 音声の再生 */
void
Sound(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef SOUND
	sounds_play(p, *n);
#endif
}

/* アクション 開始時音声 */
void
SoundStart(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef SOUND
	xmascot_sound(&adat, SOUND_START);
#endif
}

/* アクション クリック時音声 */
void
SoundClick(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef SOUND
	xmascot_sound(&adat, SOUND_CLICK);
#endif
}

/* アクション 終了時音声 */
void
SoundEnd(Widget w, XEvent * e, String * p, Cardinal * n)
{
#ifdef SOUND
	xmascot_sound(&adat, SOUND_END);
#endif
}

/* アクション メール音声 */
void
SoundMail(Widget w, XEvent * e, String * p, Cardinal * n)
{
#if defined(BIFF) && defined(SOUND)
	xmascot_sound(&adat, SOUND_MAIL);
#endif
}

/* アクション 外部コマンド呼びだし */
void
System(Widget w, XEvent * e, String * p, Cardinal * n)
{
	if (*n > 0)
		system(p[0]);
}

/* アクション パラメータ変更 */
void
ChangeParam(Widget w, XEvent * e, String * p, Cardinal * n)
{
	int             i;
	for (i = 0; i < *n; i += 2) {
		if (!strcmp(p[i], "clen")) {
			adat.chain_len = atoi(p[i + 1]);
			set_sim_param();
			reset_pos();
			set_pos();
		} else if (!strcmp(p[i], "damp")) {
			adat.damping = atof(p[i + 1]);
			if (adat.damping < 0)
				adat.damping = 0;
			if (adat.damping > 1)
				adat.damping = 1;
		} else if (!strcmp(p[i], "mag")) {
			adat.magnify = atof(p[i + 1]);
			if (adat.magnify < 0.01)
				adat.magnify = 0.01;
			change_mascot(w, (XtPointer) ((long)adat.mascot_number), NULL);
		} else if (!strcmp(p[i], "grav")) {
			adat.grav = atoi(p[i + 1]);
			set_sim_param();
		}
	}
}

/* アクション ベルをならす */
void
Bell(Widget w, XEvent * e, String * p, Cardinal * n)
{
	int             i;
	if (*n > 0) {
		i = atoi(p[0]);
		if (i < -100)
			i = -100;
		if (i > 100)
			i = 100;
		XBell(XtDisplay(w), i);
	} else
		XBell(XtDisplay(w), 100);
}
