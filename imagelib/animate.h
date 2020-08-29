#ifndef __animate_h_
#define __animate_h_

#include <sys/types.h>
#include "image.h"

/* �¥��᡼���ξ��� */
typedef enum {
	RI_NONE,
	RI_PLANE,
	RI_PIXMAP
} RealImageMode;

#define TRANSPARENT_PIXEL 0xffffffff

/* �¥��᡼�� */
typedef struct {
	RealImageMode mode;			/* �����ξ��� */
	int realized;

	/* for PI_PLANE */
	char *foreground_name;		/* Plane ���ο�����(̾��)      */
	u_long foreground_pixel;	/* Plane ���ο�����(pixel�ֹ�) */

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

/* ���ܥѥ�����ɽ���� */
typedef struct AnimatePat {
	struct AnimatePat *prev;	/* ���ѥ����� */
	struct AnimatePat *next;	/* ���ѥ����� */
	
	AnimatePatMode mode;		/* ɽ���⡼��     */
	RealImage *image;			/* ɽ���ѥ��᡼�� */
	int interval;				/* ɽ������ */

/* ��ʬ�ѥ������� */
	int src_x;					/* ɽ������ɸ */
	int src_y;					  
    int dst_x;					/* ɽ�����ɸ */
	int dst_y;	
	int clip_w;					/* �������å� */
	int clip_h;	

/* for mask effect */
	int mask_mode;				/* ɽ���ޥ������� 		*/
	int mask_cnt;				/* �ޥ���ɽ���ѥ������ */

/* for loop */
	LoopMode loop_flag;			/* �롼�ץե饰   */
	int loop_cnt;				/* ������       */
	int loop_max;				/* �����󥿺����� */
	struct AnimatePat *loop;	/* �롼�ץݥ���� */

} AnimatePat, *AnimatePatList;

/* ���˥᡼��������� */
typedef struct Animate{
	/* ���˥᡼�����ǡ��� */
	AnimatePatList list;		/* ɽ���ѥ�����ꥹ�� */
	AnimatePatList point;		/* ɽ���ݥ��� */

	unsigned w;					/* ���ܲ��������� */
	unsigned h;
	
	/* ɽ����°�� */
	int realized;
	Display *dpy;
	Window	 win;	
	int		depth;
	Colormap colormap;		/* RI_PLANE �ѥ��顼�ޥå� */
	Pixmap	pixmap;			/* ɽ���Хåե��� pixmap   */
	Pixmap  mask;			/* ɽ���ޥ�����   pixmap   */
	GC		gc;				/* �̾�������              */
	GC		gcm;			/* �ޥ���������			   */
	GC		gc2;
	int unit;

	int shape_flag;		/* ���� shape ��ͭ��    */
	int shape_mode;		/* shape �����Ѥ��뤫 ? */

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
