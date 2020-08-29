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

/* 画像データ */
	int         type;			/* 画像の種別 */
	unsigned	width;			/* 画像のサイズ */
	unsigned 	height;
	void    	*data;			/* 画像データ */

/* パレット (for pseudo color) */
	struct palette {			/* 画像パレットテーブル */
		u_char r,g,b,pad;
	} *pal;
	size_t npal;				/* 色数 from palette table */

/* カラー割り当て関連 */
	Display	*display;			/* 各種資源割り当て用       */
	Window	 window;			/* 描画対象になるウィンドウ */
	XVisualInfo vinfo;

	Colormap colormap;			/* カラーマップ	  	        */
	size_t	 ncmap;				/* カラーマップのエントリ数 */
	int		 depth;				/* pixmap 生成用            */
	
	int pixel_allocated;			/* 割り当て完了フラグ         */
	u_long	*allocated_pixel;		/* 割り当てられたピクセル     */
	size_t	 npixel;				/* 割り当てられたピクセル数   */
	u_long   pixel_value[MAX_COL];	/* データ->pixel 変換テーブル */
	
/* 透明色指定 */
	enum {TRANS_NONE, TRANS_AUTO, TRANS_INDEX, TRANS_RGB} trans_flag;
	int	   trans_index;		/* psuedo color index   */	
	int    trans_rgb;		/* 24/32 bpp True Color */

	int ref_count;		/* リファレンスカウント*/

	Pixmap pixmap;		/* 生成ずみ pixmap */
	Pixmap mask;		/* 生成ずみ mask   */

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
