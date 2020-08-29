#ifndef __animate_h_
#define __animate_h_

#include <sys/types.h>
#include "image.h"

/* 実イメージの状態 */
typedef enum {
	RI_NONE,
	RI_PLANE,
	RI_PIXMAP
} RealImageMode;

#define TRANSPARENT_PIXEL 0xffffffff

/* 実イメージ */
typedef struct {
	RealImageMode mode;			/* 画像の状態 */
	int realized;

	/* for PI_PLANE */
	char *foreground_name;		/* Plane 時の色指定(名前)      */
	u_long foreground_pixel;	/* Plane 時の色指定(pixel番号) */

	/* for PI_PIXMAP */
	ImageData *image;				

} RealImage;

typedef enum {
	LOOP_NONE,
	LOOP_COUNT,
	LOOP_FOREVER
} LoopMode;

typedef enum {
	AP_NONE,	/* no pattern     */
	AP_NORMAL,	/* copy pattern   */
	AP_DIFF,	/* differntian pattern */
	AP_MASKED	/* mask effect         */
} AnimatePatMode;

/* 基本パターン表示用 */
typedef struct AnimatePat {
	struct AnimatePat *prev;	/* 前パターン */
	struct AnimatePat *next;	/* 次パターン */
	
	AnimatePatMode mode;		/* 表示モード     */
	RealImage *image;			/* 表示用イメージ */
	int interval;				/* 表示時間 */

/* 差分パターン用 */
	int src_x;					/* 表示元座標 */
	int src_y;					  
    int dst_x;					/* 表示先座標 */
	int dst_y;	
	int clip_w;					/* 矩形クリップ */
	int clip_h;	

/* for mask effect */
	int mask_mode;				/* 表示マスク種別 		*/
	int mask_cnt;				/* マスク表示用カウント */

/* for loop */
	LoopMode loop_flag;			/* ループフラグ   */
	int loop_cnt;				/* カウンタ       */
	int loop_max;				/* カウンタ最大値 */
	struct AnimatePat *loop;	/* ループポイント */

} AnimatePat, *AnimatePatList;

/* アニメーション全体 */
typedef struct Animate{
	/* アニメーションデータ */
	AnimatePatList list;		/* 表示パターンリスト */
	AnimatePatList point;		/* 表示ポインタ */

	unsigned w;					/* 基本画像サイズ */
	unsigned h;
	
	/* 表示用属性 */
	int realized;
	Display *dpy;
	Window	 win;	
	int		depth;
	Colormap colormap;		/* RI_PLANE 用カラーマップ */
	Pixmap	pixmap;			/* 表示バッファ用 pixmap   */
	Pixmap  mask;			/* 表示マスク用   pixmap   */
	GC		gc;				/* 通常描画用              */
	GC		gcm;			/* マスク描画用			   */
	GC		gc2;
	int unit;

	int shape_flag;		/* 現在 shape が有効    */
	int shape_mode;		/* shape を利用するか ? */

	Pixmap *mask_pattern;

	int dup;
} Animate;

/* anim_pat.c */

AnimatePat *animpat_new(void);
void animpat_delete(Animate *a, AnimatePat *p);

/* anim_image.c */

RealImage *realimage_new(void);
RealImage *realimage_new_plane(char *color_name);
RealImage *realimage_new_image(ImageData *img);
void       realimage_delete(Animate *a, RealImage *img);
void       realimage_realize(Animate *a, RealImage *img);

void animpat_draw(Animate *a, AnimatePat *pat);
void animate_expose(Animate *a, int x, int y, int w, int h);
void animate_expose_region(Animate *a, Region reg);
void animate_set_shape(Animate *a);

/* animate.c */

Animate *animate_new(void);
Animate *animate_dup(Animate *orig);
void animate_delete(Animate *a);

Window animate_create_win(Animate *a, Display *dpy, Window parent);
void   animate_attach_win(Animate *a, Display *dpy, Window win);
void   animate_init_all(Animate *a);
void   animate_realize(Animate *a);
void   animate_getsize(Animate *a);
void   animate_add_pattern(Animate *a, AnimatePat *pat);

int animate_go(Animate *a);

/* anim_y.y */
int  animate_parse(Animate *a, FILE *fp);
FILE *animate_fopen(const char *name);
void animate_fclose(FILE *fp);

/* anim_gif.c */
int anim_gif_read_stream(Animate *a, FILE *fp);

#endif
