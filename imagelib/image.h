/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/
#ifndef __IMAGE_H
#define __IMAGE_H

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "etc.h"

#define MAX_COL 256

typedef struct{

/* �����ǡ��� */
	int         type;			/* �����μ��� */
	unsigned	width;			/* �����Υ����� */
	unsigned 	height;
	void    	*data;			/* �����ǡ��� */

/* �ѥ�å� (for pseudo color) */
	struct palette {			/* �����ѥ�åȥơ��֥� */
		u_char r,g,b,pad;
	} *pal;
	size_t npal;				/* ���� from palette table */

/* ���顼������ƴ�Ϣ */
	Display	*display;			/* �Ƽ�񸻳��������       */
	Window	 window;			/* �����оݤˤʤ륦����ɥ� */
	XVisualInfo vinfo;

	Colormap colormap;			/* ���顼�ޥå�	  	        */
	size_t	 ncmap;				/* ���顼�ޥåפΥ���ȥ�� */
	int		 depth;				/* pixmap ������            */
	
	int pixel_allocated;			/* ������ƴ�λ�ե饰         */
	u_long	*allocated_pixel;		/* ������Ƥ�줿�ԥ�����     */
	size_t	 npixel;				/* ������Ƥ�줿�ԥ������   */
	u_long   pixel_value[MAX_COL];	/* �ǡ���->pixel �Ѵ��ơ��֥� */
	
/* Ʃ�������� */
	enum {TRANS_NONE, TRANS_AUTO, TRANS_INDEX, TRANS_RGB} trans_flag;
	int	   trans_index;		/* psuedo color index   */	
	int    trans_rgb;		/* 24/32 bpp True Color */

	int ref_count;		/* ��ե���󥹥������*/

	Pixmap pixmap;		/* �������� pixmap */
	Pixmap mask;		/* �������� mask   */

} ImageData;

ImageData* image_new(void);
ImageData* image_attach(ImageData *img);

void image_delete(ImageData *img);
void image_delete_only_data(ImageData *img);

void image_free_data(ImageData *img);
void image_free_pixels(ImageData *img);
void image_free_pixmap(ImageData *img);

void image_set_col(ImageData *img, Display *dpy, Window win);

int  image_read_stream(ImageData *img, FILE *fp);
void image_load(ImageData *img, char *name, int c0, int r0);

Pixmap image_pixmap(ImageData *img, Pixmap *m);
Pixmap image_pixmap_with_shadow(ImageData *img, Pixmap *m, int shadow_len);

void *image_alloc_data(ImageData *img, int w, int h, int t);
void *image_alloc_palette(ImageData *img, size_t n);

#endif
