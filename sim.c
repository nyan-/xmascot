/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

#include "xmascot.h"

extern Widget   mascot, *chain;
extern double   disp_dpm;	/* dot per mm */
extern volatile int time_fl;	/* 描画タイミングフラグ */
extern volatile int time_cnt;	/* カウンタ             */

#ifdef BIFF
extern Widget   biff;
extern unsigned biff_r;
extern unsigned biff_h;
extern unsigned biff_w_off;

#ifdef USE_DOUBLE
extern double   biff_th;
#else
extern int      biff_th;
#endif

extern int      mbox_flag;
#endif

extern Pixmap   pixmap[17];	/* mascot 用 pixmap */
#ifdef SHAPE
extern Pixmap   bitmap[17];	/* mask   用 bitmap */
#endif

extern int      px, py;		/* pin Geometry */
extern unsigned ms;		/* mascot size  */

#ifdef USE_CHAINPAT
extern unsigned chain_w;
extern unsigned chain_h;
#define CHAIN_W chain_w
#define CHAIN_H chain_h
#else
#define CHAIN_W (CHAIN_SIZE/2)
#define CHAIN_H (CHAIN_SIZE/2)
#endif

double          sim_param;

#ifdef USE_DOUBLE
double          th;		/* 角度            */
double          om = 0;		/* 角速度          */
#else
int             th;		/* 角度            */
int             om = 0;		/* 角速度          */
#endif

static int      so = 8, mxo = 0, myo = 0;	/* 以前の状態 */

extern AppData  adat;		/* 基本リソース */
extern unsigned mh;
extern int      map_fl;		/* マスコットは表示されているか */

/* シミュレーション用パラメータ設定 */
void 
set_sim_param(void)
{
	int             i;

	sim_param = adat.grav * disp_dpm * 10 / adat.chain_len;

	/* 鎖の表示個数決定 (隠れる部分は表示しない) */

	if (adat.chain_len < mh - CHAIN_H) {
		adat.chain_disp_num = 0;
	} else {
		adat.chain_disp_num = adat.chain_num -
			(mh - CHAIN_H) / (adat.chain_len / (adat.chain_num + 1));
	}
	if (map_fl) {
		for (i = 0; i < adat.chain_disp_num; i++)
			XtMapWidget(chain[i]);
		for (; i < adat.chain_num; i++)
			XtUnmapWidget(chain[i]);
	}
}

void 
reset_pos(void)
{
	so = mxo = myo = -9999;
}

/* 位置の計算 および 表示パターンの変更 */
void 
set_pos(void)
{
	int             mx, my;
	int             i, s;
	Window          win = XtWindow(mascot);
	Display        *dpy = XtDisplay(mascot);

#ifdef USE_DOUBLE
	double bx, by, dx, dy;
	double sn = sin(th) * adat.chain_len;
	double cs = cos(th) * adat.chain_len;
	mx = px + sn;
	my = py + cs;
#else
	int bx, by, dx, dy;
	int sn = isin(th) * adat.chain_len;
	int cs = icos(th) * adat.chain_len;
	mx = px + (sn >> 8);
	my = py + (cs >> 8);
#endif

	if (mxo != mx || myo != my) {	/* 表示位置が動いた */
		/* 表示パターンの決定 */
#ifdef USE_DOUBLE
		s = sqrt(fabs(th)) * 8 / sqrt(ANGLE_PI / 4);
#else
		s = sqrt(abs(th)) * 8 / sqrt(ANGLE_PI / 4);
#endif
		if (th < 0)
			s = s + 8;
		else
			s = 8 - s;
		if (s > 16)
			s = 16;
		if (s < 0)
			s = 0;
		if (s != so) {	/* 表示パターンの変更 */
			XSetWindowBackgroundPixmap(dpy, win, pixmap[s]);
#ifdef SHAPE
			XShapeCombineMask(dpy, win, ShapeBounding, 0, 0,
							  bitmap[s], ShapeSet);
#endif
			XClearWindow(dpy, win);
			so = s;
		}
#ifdef BIFF
		if (mbox_flag) {
#ifdef USE_DOUBLE
			double          th2;
#else
			int             th2;
#endif
			/* 角度制限 */
			if (th > ANGLE_PI / 4)
				th2 = biff_th + ANGLE_PI / 4;
			else if (th < -ANGLE_PI / 4)
				th2 = biff_th - ANGLE_PI / 4;
			else
				th2 = biff_th + th;
			if (th2 > ANGLE_PI)
				th2 -= ANGLE_PI * 2;
			else if (th2 <= -ANGLE_PI)
				th2 += ANGLE_PI * 2;

#ifdef USE_DOUBLE
			bx = mx - biff_r * sin(th2);
			by = my - biff_r * cos(th2);
#else
			bx = mx - ((biff_r * isin(th2)) >> 8);
			by = my - ((biff_r * icos(th2)) >> 8);
#endif
			XtMoveWidget(biff, (Position) (bx - biff_w_off),
				     (Position) (by - biff_h));
		}
#endif

		/* 鎖の表示 */
#ifdef USE_DOUBLE
		dx = sn / (adat.chain_num + 1);
		dy = cs / (adat.chain_num + 1);
#else
		dx = sn / (adat.chain_num + 1) >> 8;
		dy = cs / (adat.chain_num + 1) >> 8;
#endif

		bx = px + dx - CHAIN_W;
		by = py + dy - CHAIN_H;

		XtMoveWidget(mascot, (Position) (mx - ms), (Position) (my - ms));
		for (i = 0; i < adat.chain_num; i++, bx += dx, by += dy)
			XtMoveWidget(chain[i], (Position) bx, (Position) by);

		/* 全部描くまで待つ */
		XSync(dpy, False);
		mxo = mx;
		myo = my;
	}
}


/* 振子のシミュレーション */
void 
sim(void)
{
#ifdef USE_DOUBLE
	double          dom;
#else
	int             dom;
#endif
	while (time_cnt > 0) {
#ifdef USE_DOUBLE
		dom = sim_param * sin(th);
#else
		dom = sim_param * isin(th);
#endif
		om -= dom * DT / 1000.0 + adat.damping * om;
		th += om * DT / 1000.0;
		while (th > ANGLE_PI)
			th -= ANGLE_PI * 2;
		while (th <= -ANGLE_PI)
			th += ANGLE_PI * 2;

#ifdef USE_DOUBLE
		if (fabs(om) < 0.02 && fabs(dom) < 0.02) {
#else
		if (abs(om) < 5 && abs(dom) < 5) {
#endif
			stop_timer();
			om = 0;
			dom = 0;
		}
		time_cnt--;
	}
	time_fl = 0;
	set_pos();
}
