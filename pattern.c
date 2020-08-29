/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe	 go@cclub.tutcc.tut.ac.jp
					      Tsuyoshi IIda	 iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "xmascot.h"
#include <X11/Shell.h>

#ifdef SHAPE
#include <X11/extensions/shape.h>
#endif

extern Widget   top, mascot, *chain;
extern AppData  adat;

extern int      map_fl;		/* 現在マスコットはマップされているか？ */

unsigned        ms;		/* マスコットの Pixmap の大きさ */
unsigned        mh;		/* マスコットの Pixmap 縦幅     */

#ifdef USE_CHAINPAT
unsigned        chain_w = CHAIN_SIZE / 2;
unsigned        chain_h = CHAIN_SIZE / 2;
#endif

#ifdef BIFF
extern Widget   biff;
unsigned        biff_r;
unsigned        biff_w_off;
unsigned        biff_h;
#ifdef USE_DOUBLE
double          biff_th;
#else
int             biff_th;
#endif
#endif

/* Widget にパターンを設定する */
void 
set_widget_pattern(Widget w, char *name, int c, int r)
{
	ImageData      *img;

	Pixmap          pix;
#ifdef SHAPE
	Pixmap          bit;
#endif

	img = image_new();
	image_load(img, name, c, r);
	if (img->data == NULL) {
		err_out("Can't Set \"%s\" pattern.\n", XtName(w));
		exit(1);
	}
	image_set_col(img, XtDisplay(w), XtWindow(w));

#ifdef SHAPE
#ifdef SHADOW
	pix = image_pixmap_with_shadow(img, &bit, adat.shadow);
#else
	pix = image_pixmap(img, &bit);
#endif
#else
	pix = image_pixmap(img, NULL);
#endif

	XtVaSetValues(w, XtNbackgroundPixmap, pix, NULL);
#ifdef SHADOW
	XtResizeWidget(w, (Dimension) (img->width + adat.shadow),
				   (Dimension) (img->height + adat.shadow), 0);
#else
	XtResizeWidget(w, (Dimension)img->width,
				   (Dimension)img->height, 0);
#endif


#ifdef SHAPE
	XShapeCombineMask(XtDisplay(w), XtWindow(w), ShapeBounding, 0, 0, bit, ShapeSet);
#endif
	image_free_pixmap(img);
	image_delete_only_data(img);
}

#ifdef USE_CHAINPAT
/* chain のパターンを設定する */
void 
set_chain_pat(char *name, int c, int r)
{
	int             i;
	ImageData      *img;
	Display        *dpy = XtDisplay(top);
	Window          w = XtWindow(top);

	Pixmap          pix;
#ifdef SHAPE
	Pixmap          bit;
#endif

	img = image_new();
	image_load(img, name, c, r);
	if (img->data == NULL) {
		err_out("Can't Set \"%s\" pattern.\n", XtName(top));
		exit(1);
	}
	image_set_col(img, XtDisplay(chain[0]), XtWindow(chain[0]));

	chain_w = img->width / 2;
	chain_h = img->height / 2;
	for (i = 0; i < adat.chain_num; i++)
		XtResizeWidget(chain[i],
					   img->width + adat.shadow, 
					   img->height + adat.shadow, 0);

#ifdef SHAPE
#ifdef SHADOW
	pix = image_pixmap_with_shadow(img, &bit, adat.shadow);
#else
	pix = image_pixmap(img, &bit);
#endif
	for (i = 0; i < adat.chain_num; i++)
		XShapeCombineMask(dpy, XtWindow(chain[i]),
				  ShapeBounding, 0, 0, bit, ShapeSet);
#else
	pix = image_pixmap(img, NULL);
#endif
	for (i = 0; i < adat.chain_num; i++)
		XtVaSetValues(chain[i], XtNbackgroundPixmap, pix, NULL);
	image_free_pixmap(img);
	image_delete_only_data(img);
}
#endif



Pixmap          pixmap[17];
#ifdef SHAPE
Pixmap          bitmap[17];
#endif

/* データの回転 */
static void 
roll(ImageData *img, double mmag)
{
	int             i;
	GC              gc, gc2;

	XImage         *img_data;
#ifdef SHAPE
	XImage         *img_mask;
#endif
	unsigned        w = img->width;	/* 横幅	 */
	unsigned        h = img->height;	/* 縦幅	 */
	unsigned        s = sqrt((double) (w * w + h * h)) * mmag;	/* 対角長 */

#ifdef SHADOW
	u_long          black;

	msg_out("in roll\n");

	if (adat.shadow){
		if (img->vinfo.class != TrueColor) {
			XColor          col;
			col.red = col.blue = col.green = 0;
			col.flags = DoRed | DoGreen | DoBlue;
			if (XAllocColor(img->display, img->colormap, &col)) {
				black = img->allocated_pixel[img->npixel++] = col.pixel;
			} else {
				err_out("black allocation failed\n");
				black = 0;
			}
		} else 
			black = 0;
	}
#endif

	/* pixmap の生成 */
	for (i = 0; i < 17; i++) {
		pixmap[i] = XCreatePixmap(img->display, img->window, s, s, img->depth);
#ifdef SHAPE
		bitmap[i] = XCreatePixmap(img->display, img->window, s, s, 1);
#endif
	}

	/* Image 生成 */
	img_data = XGetImage(img->display, pixmap[0], 0, 0, s, s, AllPlanes, ZPixmap);
#ifdef SHAPE
	img_mask = XGetImage(img->display, bitmap[0], 0, 0, s, s, 1, ZPixmap);
#endif

	/* gc の 生成 */
	gc = XCreateGC(img->display, pixmap[0], 0, NULL);
#ifdef SHAPE
	gc2 = XCreateGC(img->display, bitmap[0], 0, NULL);
#endif

	for (i = 0; i < 17; i++) {

		u_char *data        = img->data;
		u_long *pixel_value = img->pixel_value;

		int             x, y;
		int             c;
		int             d1 = s / 2;
		int             d2 = w / 2;
		int             d3 = h / 2;
		double          u, v, du, dv;
		double          co, si;
		double          d;
		msg_out("\rmaking rotete image %2d/17 ", i + 1);
		d = (8 - i) / 8.0 * sqrt((double) M_PI / 4);
		d = d * d * ((i < 8) ? -1.0 : 1.0);
		co = cos(d) / mmag;
		si = sin(d) / mmag;
		du = ((d1) * co + (d1) * si) + d2;
		dv = (-(d1) * si + (d1) * co) + d3;
		for (y = s - 1; y >= 0; y--) {
			u = du;
			v = dv;
			for (x = s - 1; x >= 0; x--) {
				c = (int)v*w + (int)u;
				if (u >= 0 && u < w && v >= 0 && v < h) {
#ifdef SHAPE
#ifdef SHADOW
					if (adat.shadow) {
						if (data[c] != img->trans_index) {
							XPutPixel(img_data, x, y,
									  pixel_value[data[c]]);
							XPutPixel(img_mask, x, y, 1);
							if ((x + y) & 1)
								if (x + adat.shadow < ms &&
								    y + adat.shadow < ms)
									XPutPixel(img_mask,
									  x + adat.shadow, y + adat.shadow, 1);
						} else {
							XPutPixel(img_data, x, y, black);
							XPutPixel(img_mask, x, y, 0);
						}
					} else
#endif
					{
						XPutPixel(img_data, x, y, pixel_value[data[c]]);
						XPutPixel(img_mask, x, y, 
							  (data[c] != img->trans_index)?1:0);
					}
#else				/* not SHAPE */
					XPutPixel(img_data, x, y, pixel_value[data[c]]);
#endif
				} else {
#ifdef SHADOW
					if (adat.shadow)
						XPutPixel(img_data, x, y, black);
					else
#endif
						XPutPixel(img_data, x, y,
								  img->pixel_value[data[c]]);
#ifdef SHAPE
					XPutPixel(img_mask, x, y, 0L);
#endif
				}
				u -= co;
				v += si;
			}
			du -= si;
			dv -= co;
		}
		XPutImage(img->display, pixmap[i], gc, img_data, 0, 0, 0, 0, s, s);
#ifdef SHAPE
		XPutImage(img->display, bitmap[i], gc2, img_mask, 0, 0, 0, 0, s, s);
#endif
	}
	msg_out("..done.\n");
	XDestroyImage(img_data);
	XFreeGC(img->display, gc);
#ifdef SHAPE
	XDestroyImage(img_mask);
	XFreeGC(img->display, gc2);
#endif
}

/* mascot の表示パターンを設定する */
void 
set_mas(XMascotData *adat)
{
	static ImageData *img = NULL;
	int             i;
	static int      inited = 0;

	Mascot *m = &adat->mascot_menus[adat->menu_no].
		mascots[adat->mascot_number];

	int             c = m->col0;
	int             r = m->rgb0;
	double          mmag = m->mmag * adat->magnify;

	if (map_fl)
		XtUnmapWidget(mascot);

	image_delete(img);
	img = image_new();
	image_load(img, m->fname, c, r);
	if(img->data == NULL){
		err_out("Can't Set MASCOT[%s].\n",m->title);
		if( inited )
			return;
		else
			exit(1);
	}

	image_set_col(img, XtDisplay(mascot), XtWindow(mascot));
	if(inited)
		for(i=0;i<17;i++){
			XFreePixmap(XtDisplay(mascot),pixmap[i]);
#ifdef SHAPE
			XFreePixmap(XtDisplay(mascot),bitmap[i]);
#endif
		}	
	else
		inited = 1;

	ms = sqrt((double)(img->width*img->width + img->height*img->height))*mmag; /* 対角線の長さ */
	mh = img->height / 2 * mmag;

#ifdef BIFF
	/* biff 到着ウィンドウ表示位置決定 */
	{
		Dimension       biff_w;

		XtVaGetValues(biff, XtNwidth, &biff_w, XtNheight, &biff_h, NULL);
		if (img->width > img->height)
			biff_r = img->width * mmag / 2;
		else
			biff_r = img->height * mmag / 2;
		if (m->biff_justify == XtJustifyCenter) {
			biff_th = 0;
			biff_w_off = biff_w * mmag / 2;
		} else {
			biff_th = (ANGLE_PI / 2) - atan((double) img->height / img->width) / RAD;
			biff_w_off = biff_w * mmag;
			if (m->biff_justify == XtJustifyRight) {
				biff_th = -biff_th;
				biff_w_off = 0;
			}
		}
	}
#endif

	XtResizeWidget(mascot, ms, ms, 0);

	roll(img, mmag);	/* 回転パターンの生成 */
	ms /= 2;
	if (map_fl)
		XtMapWidget(mascot);
}
