#include <stdio.h>
#include <stdlib.h>
#include "animate.h"

/* mask patterns */
static u_char bitpat[][8][8] = {
	{
		{0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00},
		{0x44, 0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00},
		{0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88},
		{0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22},
		{0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44},
		{0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00, 0x11},
		{0x22, 0x00, 0x88, 0x00, 0x22, 0x00, 0x88, 0x00},
		{0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00}
	},
	{
		{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
		{0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
		{0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
		{0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
		{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10},
		{0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20},
		{0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
	},
	{
		{0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
		{0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7f},
		{0x00, 0x7e, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x3e, 0x00},
		{0x00, 0x00, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x04, 0x04, 0x1c, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00},
	},
	{
		{0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x18, 0x10, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x04, 0x04, 0x1c, 0x00, 0x00},
		{0x00, 0x00, 0x3c, 0x20, 0x20, 0x20, 0x00, 0x00},
		{0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x3e, 0x00},
		{0x00, 0x7e, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x7f},
		{0xff, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
	},

};

#define MASK_NO  (sizeof(bitpat)/sizeof(*bitpat))
typedef struct screeninfo {
	struct screeninfo *next;
	Display *dpy;
	int screen_no;
	Pixmap mask_pattern[MASK_NO * 8];
	int cnt;
} ScreenInfo;

static ScreenInfo *head = NULL;

static void
animate_mask_init(Animate *a)
{
	int i, j;
	int unit;
	ScreenInfo *p = head;

	XWindowAttributes attr;
	XGetWindowAttributes(a->dpy, a->win, &attr);

	/* search */
	while (p != NULL) {
		if (p->dpy == a->dpy &&
			p->screen_no == XScreenNumberOfScreen(attr.screen)) {
			a->mask_pattern = p->mask_pattern;
			p->cnt++;
			return;
		}
	}

	if ((p = malloc(sizeof *p)) == NULL) {
		perror("animate_mask_init");
		exit(1);
	}

	/* add to list */
	p->next = head;
	p->dpy  = a->dpy;
	p->screen_no = XScreenNumberOfScreen(attr.screen);
	p->cnt = 1;
	head = p;

	unit = BitmapUnit(a->dpy);
	
	for (i=0;i<MASK_NO;i++) {
#if 0
		Pixmap pix, pix2;
		GC gc = None;
		for (j=0;j<8;j++) {
			pix = XCreateBitmapFromData(a->dpy,a->win,bitpat[i][j],8,8);
			if (gc == None) {
				gc = XCreateGC(a->dpy, pix, 0, NULL);
				XSetFillStyle(a->dpy, gc, FillStippled);
				XSetForeground(a->dpy, gc, 1);
			}
			pix2 = p->mask_pattern[i*8+j] =
				XCreatePixmap(a->dpy,a->win,a->w,a->h,1);
			XSetStipple(a->dpy, gc, pix);
			XFillRectangle(a->dpy, pix2, gc, 0, 0, a->w, a->h);
			XFreePixmap(a->dpy, pix);
		}
		XFreeGC(a->dpy,gc);
#else
		for (j=0;j<8;j++) {
			switch (unit) {
			case 32:
				{
					u_char dat[4*32];
					int k;
					for (k=0;k<32*4;k+=4) 
						dat[k] = dat[k+1] = dat[k+2] = dat[k+3] = 
							bitpat[i][j][(k/4)%8];
					p->mask_pattern[i*8+j] =
						XCreateBitmapFromData(a->dpy,a->win,dat,32,32);
					a->unit = 32;
				}
				break;
			default:
				p->mask_pattern[i*8+j] =
					XCreateBitmapFromData(a->dpy,a->win,bitpat[i][j],8,8);
				a->unit = 8;
			}
		}
#endif
	}
	a->mask_pattern = p->mask_pattern;
}

static void
animate_mask_free(Animate *a)
{
	ScreenInfo *p = head, **q = &head;
	
	if (a->mask_pattern) {
		/* search */
		while (p != NULL) {
			if (p->mask_pattern == a->mask_pattern) {
				if (--(p->cnt) <= 0) {
					int i;
					for (i=0;i<MASK_NO*8;i++)
						XFreePixmap(p->dpy,p->mask_pattern[i]);
					*q = p->next;
					free(p);
					return;
				} else
					return;
			}
			q = &(p->next);
			p = p->next;
		}
	}
}


int
animate_go(Animate *a)
{
	AnimatePat *pat = a->point;

	if (a->point == a->list) {	/* パターンが完了 */
		a->point = a->list->next;
		return 0;
	}

	animpat_draw(a, pat);
	/* 次のパターンの選択 */

	if (pat->mask_mode) {
		pat->mask_cnt++;
		if (pat->mask_cnt == 8) {
			pat->mask_cnt = 0;
			goto chk_loop;
		}else
			goto end;
	}

 chk_loop:
	if (pat->loop_flag != LOOP_NONE && pat->loop) {
	void animate_getsize(Animate *a);
	if (pat->loop_flag == LOOP_FOREVER) {
			a->point = pat->loop;
		} else if (pat->loop_flag == LOOP_COUNT) {
			pat->loop_cnt++;
			if (pat->loop_cnt == pat->loop_max) {
				pat->loop_cnt = 0;
				a->point = pat->next;
			} else
				a->point = pat->loop;
		}		
	}else
		a->point = pat->next;

 end:
	return 1;
}


Window
animate_create_win(Animate *a, Display *dpy, Window parent)
{
	a->dpy = dpy;
	a->win = XCreateSimpleWindow(dpy, parent, 0, 0, a->w, a->h, 0, 0, 0);
	return a->win;
}

void
animate_attach_win(Animate *a, Display *dpy, Window win)
{
	if (!a->realized) {
		a->dpy = dpy;
		a->win = win;
	}
}				   

void
animate_init_all(Animate *a)
{
	AnimatePat *p = a->list->next;
	while (p != a->list) {
		p->loop_cnt = 0;
		p->mask_cnt = 0;
		p = p->next;
	}
	a->point = a->list->next;
}

void
animate_realize(Animate *a)
{
	XWindowAttributes attr;

	if (!a->realized) {
		XGetWindowAttributes(a->dpy, a->win, &attr);
		a->colormap = attr.colormap;
		a->depth    = attr.depth;
		a->pixmap   = XCreatePixmap(a->dpy, a->win, a->w, a->h, attr.depth);
		a->mask     = XCreatePixmap(a->dpy, a->win, a->w, a->h, 1);
		a->gc  = XCreateGC(a->dpy, a->win,  0, NULL);
		a->gcm = XCreateGC(a->dpy, a->mask, 0, NULL);
		a->gc2 = XCreateGC(a->dpy, a->win,  0, NULL);

		animate_mask_init(a);

		{
			AnimatePat *p = a->list->next;
			while (p != a->list) {
				realimage_realize(a, p->image);
				p = p->next;
			}
		}

		a->realized = 1;
	}

	animate_init_all(a);
}

static void
animate_init(Animate *p)
{
	p->point = p->list;
	p->w = p->h = 0;
	p->realized = 0;
	p->dpy = NULL;
	p->win = None;
	p->colormap = None;
	p->pixmap = p->mask = None;
	p->gc = p->gcm = p->gc2 = None;
	p->unit = 0;
	p->shape_flag = 0;
	p->shape_mode = 0;
	p->mask_pattern = NULL;
}

Animate *
animate_new(void)
{
	Animate *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("animate_new");
		exit(1);
	}
	p->list = animpat_new();
	animate_init(p);

	p->dup = 0;
	return p;
}

Animate *
animate_dup(Animate *orig)
{
	Animate *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("animate_dup");
		exit(1);
	}
	p->list = orig->list;
	animate_init(p);

	p->dup = 1;
	return p;
}

void
animate_delete(Animate *a)
{
	if (a) {
		if (!a->dup) {
			if (a->list) {
				AnimatePat *p, *q;
				p = a->list->next;
				while (p != a->list) {
					q = p;
					p = p->next;
					animpat_delete(a, q);
				}
				animpat_delete(a, a->list);
			}
		}
		if (a->pixmap)
			XFreePixmap(a->dpy, a->pixmap);
		if (a->mask)
			XFreePixmap(a->dpy, a->mask);
		if (a->gc)
			XFreeGC(a->dpy, a->gc);
		if (a->gcm)
			XFreeGC(a->dpy, a->gcm);
		if (a->gc2)
			XFreeGC(a->dpy, a->gc2);
		animate_mask_free(a);
		free(a);
	}
}

#define MAX(a,b) (((a)>(b))?(a):(b))
void
animate_getsize(Animate *a)
{
	unsigned w, h;
	AnimatePat *p = a->list->next;
	w = a->w;
	h = a->h;
	/* AnimatePat の List から最大の画像サイズを get */
	while (p != a->list) {
		if (p->image == NULL) {
			fprintf(stderr,"warn: pattern %p not contain image data.",p);
		} else if(p->image->mode == RI_PIXMAP) {
			w = MAX(w, p->image->image->width);
			h = MAX(h, p->image->image->height);
		}
		p = p->next;
	}
	a->w = w;
	a->h = h;
#if 0
	fprintf(stderr,"animate size [%d,%d]\n", a->w, a->h);
#endif
}
#undef MAX

void
animate_add_pattern(Animate *a, AnimatePat *pat)
{
	pat->prev  = a->list->prev;
	pat->next  = a->list;
	a->list->prev->next = pat;
	a->list->prev = pat;
}

