/*
   XMascot Ver 2.5   image-lib
   Copyright(c)1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "image.h"

extern int verbose;

extern int mag_read_stream(ImageData *, FILE *fp);
extern int gif_read_stream(ImageData *, FILE *fp);
extern int bmp_read_stream(ImageData *, FILE *fp);
extern int pnm_read_stream(ImageData *, FILE *fp);

static struct {
	int (*read_stream)(ImageData *, FILE *fp);
} loadors[] = {
	{ gif_read_stream },
	{ mag_read_stream },
	{ bmp_read_stream },
	{ pnm_read_stream },
};

/* 新規イメージの作成 */
ImageData*
image_new(void)
{
	ImageData *img;

	if ((img = malloc(sizeof(*img))) == NULL) {
		perror("image_new");	
		exit(1);
	}

	img->display  = NULL;
	img->window   = None;
	img->colormap = None;
	img->depth    = 0;

	img->pixel_allocated = 0;
	img->allocated_pixel = NULL;
	img->npixel = 0;

	img->width  = 0;
	img->height = 0;
	img->data = NULL;

	img->pal  = NULL;
	img->npal = 0;
	img->trans_flag = TRANS_AUTO;

	img->pixmap = None;
	img->mask   = None;
	
	img->ref_count = 1;

	return img;
}
	
/* 同じイメージを返す */
ImageData *
image_attach(ImageData *img)
{
	img->ref_count++;
	return img;
}

/* イメージデータの解放 */
void
image_delete(ImageData *img)
{
	if (img) {
		img->ref_count--;
		if (img->ref_count <= 0) {
			image_free_pixmap(img);
			image_free_pixels(img);
			image_free_data(img);
			free(img);
		}	
	}
}	

/* pixel/pixmap を解放しない */
void
image_delete_only_data(ImageData *img)
{
	if (img) {
		img->ref_count--;
		if (img->ref_count <= 0) {
			image_free_data(img);
			free(img);
		}	
	}
}	

/* データ部分を解放する */
void
image_free_data(ImageData *img)
{
	if (img) {
		if (img->data != NULL) {
			free(img->data);
			img->data = NULL;
		}
		if (img->pal != NULL){
			free(img->pal);
			img->npal = 0;
			img->pal = NULL;
		}
	}		
}

/* 色を解放する */
void
image_free_pixels(ImageData *img)
{
	if (img && img->allocated_pixel) {
		int i;
		if (verbose) {
			fprintf(stderr,"free pixels: %d\n",img->npixel);
			for(i=0;i<img->npixel;i++)
				fprintf(stderr,"%d:%lx\n",i,img->allocated_pixel[i]);
		}
		XFreeColors(img->display,img->colormap,
					img->allocated_pixel,img->npixel,0);
		free(img->allocated_pixel);
		img->allocated_pixel = NULL;
		img->npixel = 0;
	}
}

void
image_free_pixmap(ImageData *img)
{
	if (img) {
		if (img->pixmap != None) {
			XFreePixmap(img->display, img->pixmap);
			img->pixmap = None;
		}
		if (img->mask != None) {
			XFreePixmap(img->display, img->mask);
			img->mask = None;
		}
	}
}

/*
    original: ximage.cc,v 1.4 1997/05/05 04:48:07 watanabe Exp

    reformed by watanabe@dsl.tutics.tut.ac.jp

    このファイルにはVissual ClassがPseudo Colorであるサーバの
    XImageを生成するためのプログラムが含まれています。

    University of Pennsylvania の John Bradley氏が作成された
    xvのプログラムを参考に記述しています。
    
 *  Author:    John Bradley, University of Pennsylvania
 *               (bradley@cis.upenn.edu)

*/

/*
   イメージ全体を操作して、ピクセル単位のヒストグラムを作成する。
   関数の返り値は画像データで実際に利用されている色の数。
   histにヒストグラムが返される。
*/
static int
make_histogram(ImageData *img, int hist[MAX_COL])
{
	int i, ncols;

	u_char *p = img->data;
	int     n = img->width * img->height;

	if (verbose)
		fprintf(stderr,"making histgram...");

	/* initialize histogram and compute it */
	for(i = 0; i < MAX_COL; i++) 
		hist[i] = 0;

	for(i = n; i; i--, p++) 
		hist[*p]++;

	ncols = 0;
	for(i = 0; i < MAX_COL; i++) {
		if(!hist[i]) continue;
		ncols++;
	}

	if (verbose)
		fprintf(stderr," get color num: %d\n",ncols);

	return ncols;
}

struct color_table{
	u_char r,g,b,pad;
	u_long pixel;
};

/*
   内部カラーマップ(map)に必要な色を優先順位をつけて格納する。優先度は、
   使用頻度と確保されている全ての色との色ベクトルの距離(なるべく遠いも
   のを優先)をもとに計算される。
   関数の返り値はmapに格納されたカラー情報数。
   orderには元の画素値との変換テーブルが返される。
*/
static void
sort_colors(ImageData *img, 
			int hist[256],int order[256], struct color_table c_tbl[256],
			int ncols)
{
    struct cent {
        int r,g,b;       /* actual value of color */
        int rtrans;      /* its index in the old colormap */
        int use;         /* # of pixels of this color */
        int min_dist;    /* min distance to a selected color */
    };
    struct cent c0[256], c1[256], *c;

	int nmap  = img->npal;
    int i, j, entry, d;

    c = c0;
    for (i = 0; i < MAX_COL ; i++) {
        if(!hist[i])
            continue;
        c->rtrans = i;
        c->use = hist[i];

        if (i >= nmap)
            c->r = c->g = c->b = 0;
        else {
            c->r = c_tbl[i].r;
            c->g = c_tbl[i].g;
            c->b = c_tbl[i].b;
        }
        c->min_dist = 1000000; /* 255^2 * 3 = 195075 */
        c++;
    }

    /* find most-used color, put that in c1[0] */

    {
        int max_use = c0[entry = 0].use;
        for(i = 1; i < ncols; i++) {
            if(max_use < c0[i].use)
                max_use = c0[entry = i].use;
        }
    }

    c1[0] = c0[entry];
    c0[entry].use = 0;   /* dealt with */
/*
    sort rest of colormap.  Half of the entries are allocated on the
    basis of distance from already allocated colors, and half on the
    basis of usage. (NB: 'taxicab' distance is used throughout this file.)

    Mod:  pick first 10 colors based on maximum distance.  pick remaining
    colors half by distance and half by usage   -- JHB

    To obtain O(n^2) performance, we keep each unselected color
    (in c[], with use>0) marked with the minimum distance to any of
    the selected colors(in c1[]).  Each time we select a color, we
    can update the minimum distances in O(n) time.

    mod by Tom Lane   Tom.Lane@g.gp.cs.cmu.edu
*/

    for (i = 1; i < ncols; i++) {
        int r, g, b;

        /* Get RGB of color last selected  and choose selection method */
        r = c1[i-1].r;
        g = c1[i-1].g;
        b = c1[i-1].b;

        /* Now find the i'th most different color */

        if((i & 1) || i < 10) {
            /* select the unused color that has the greatest min_dist */
            int max_min_dist = -1;
            entry = -1;

            for (j = 0, c = c0; j < ncols; j++,c++) {
                if (!c->use) continue;
                /* this color has not been marked already */

                /* update min_dist */

                d = (c->r - r) * (c->r - r) +
                    (c->g - g) * (c->g - g) +
                    (c->b - b) * (c->b - b);
                if (d < c->min_dist)
                    c->min_dist = d;

                if (max_min_dist < c->min_dist)
                    max_min_dist = c->min_dist, entry = j;
            }
        } else {
            /* select the unused color that has the greatest usage */
            int max_use = -1;
            entry = -1;

            for (j = 0, c = c0; j < ncols; j++,c++) {
                if (!c->use) continue;
                /* this color has not been marked already */

                /* update min_dist */
                d = (c->r - r) * (c->r - r) +
                    (c->g - g) * (c->g - g) +
                    (c->b - b) * (c->b - b);
                if(d < c->min_dist)
                    c->min_dist = d;

                if(max_use < c->use)
                    max_use = c->use, entry = j;


            }
        }

        /* c0[entry] is the next color to put in the map.  do so */
        c1[i] = c0[entry];
        c0[entry].use = 0;
    }

    for (i = 0; i < ncols; i++) {
        order[i]   = c1[i].rtrans;
        c_tbl[i].r = c1[i].r;
        c_tbl[i].g = c1[i].g;
        c_tbl[i].b = c1[i].b;
    }
}

/*
   内部カラーマップmapの情報を元に、読み込み専用のカラーを確保する。
*/
static void
alloc_read_only_colors(ImageData *img,
					   struct color_table c_tbl[256],
					   int ncols)
{
    int i, j, unique;
    XColor defs[MAX_COL], *ctab;
    int failed[MAX_COL],index[MAX_COL];
    unsigned long pixel;

    u_long *alloced_pixel;
	Display *display  = img->display;
	Colormap colormap = img->colormap;
	int ncells        = img->vinfo.colormap_size;

	if ((img->allocated_pixel = malloc(sizeof(u_long)* ncols + 1)) == NULL) {
		perror("alloc_read_only_colors");
		exit(1);
	}
	img->npixel  = 0;
    alloced_pixel = img->allocated_pixel;

/*
    FIRST PASS COLOR ALLOCATION:
    for each color in the 'desired colormap', try to get it via
    XAllocColor().  If for any reason it fails, mark that pixel
    'unallocated' and worry about it later.  Repeat.
*/

/*
    attempt to allocate first ncols entries in colormap
    note: On displays with less than 8 bits per RGB gun, it's quite
    possible that different colors in the original picture will be
    mapped to the same color on the screen.  X does this for you
    silently.  However, this is not-desirable for this application,
    because when I say 'allocate me 32 colors' I want it to allocate
    32 different colors, not 32 instances of the same 4 shades...
*/

    unique = 0;

	for (i = 0; i < ncols; i++) {
        defs[i].red   = c_tbl[i].r << 8;
        defs[i].green = c_tbl[i].g << 8;
        defs[i].blue  = c_tbl[i].b << 8;
        defs[i].flags = DoRed | DoGreen | DoBlue;

        if (!XAllocColor(display,colormap,&defs[i]))
            failed[i] = 1;
        else {
            failed[i] = 0;
            pixel = c_tbl[i].pixel = defs[i].pixel;

            /* see if the newly allocated color is new and different */

            for (j = 0; j < img->npixel; j++)
                if (alloced_pixel[j] == pixel) break;

            if (j == img->npixel)
                unique++;

            index[img->npixel] = i;
            alloced_pixel[img->npixel++] = pixel;
        }
    }

	if(img->npixel == ncols)
        return;
	
/*
    SECOND PASS COLOR ALLOCATION:

    Allocating 'exact' colors failed.
    Now try to allocate 'closest' colors.

    Read entire X colormap(or first 256 entries) in from display.
    for each unallocated pixel, find the closest color that actually
    is in the X colormap.  Try to allocate that color(read only).
    If that fails, the THIRD PASS will deal with it
*/

	/* only do SECOND PASS if there IS a colormap to read */
	if ((ctab = (XColor *)alloca(sizeof ctab[0] * ncells)) == NULL) {
        perror("alloca");
        fprintf(stderr,"panic:alloc_read_only_colors failed.\n");
        abort();
    }

    /* read entire colormap into 'ctab' */
    /*  assume StaticGray, Grayscale, Pseudo, Static Visual */

    for (i = 0; i < ncells; i++)
        ctab[i].pixel = i;
    XQueryColors(display,colormap,ctab,ncells);

    for (i = 0; i < ncols; i++) {
        int min_dist, close;
        int ri, gi, bi;

        if(!failed[i])
            continue; /* an allocated pixel */

        min_dist = 1000000;
        close = -1;
        ri = c_tbl[i].r;
        gi = c_tbl[i].g;
        bi = c_tbl[i].b;

        for (j = 0; j < ncells; j++) {
            int d, rd, gd, bd;

            rd = ri - (ctab[j].red >> 8);
            gd = gi - (ctab[j].green >> 8);
            bd = bi - (ctab[j].blue >> 8);

            d = rd * rd + gd * gd + bd * bd;
            if(d < min_dist)
                min_dist = d, close = j;
        }
        if (XAllocColor(display, colormap, &ctab[close])) {
            defs[i] = ctab[close];
            failed[i] = 0;
            pixel = c_tbl[i].pixel = ctab[close].pixel;

            index[img->npixel] = i;
            alloced_pixel[img->npixel++] = pixel;
            unique++;
        }

    }  /* SECOND PASS */

	if(img->npixel == ncols)
        return;

/*
    THIRD PASS COLOR ALLOCATION:

    We've alloc'ed all the colors we can.  Now, we have to map any
    remaining unalloced pixels into either the colors that we DID get
*/

    for (i = 0; i < ncols; i++) {
        int min_dist, close;
        int ri, gi, bi;

        if (!failed[i])
            continue;  /* an allocated pixel */

        min_dist = 1000000;
        close = -1;
        ri = c_tbl[i].r;
        gi = c_tbl[i].g;
        bi = c_tbl[i].b;

        /* search the alloc'd colors */

        for (j = 0; j < img->npixel; j++) {
            int k, d, rd, gd, bd;

            k = index[j];
            rd = ri - (defs[k].red >> 8);
            gd = gi - (defs[k].green >> 8);
            bd = bi - (defs[k].blue >> 8);

            d = rd * rd + gd * gd + bd * bd;
            if(d < min_dist)
                min_dist = d,close = k;
        }

        defs[i] = defs[close];
        c_tbl[i].pixel  = defs[i].pixel;

    }  /* THIRD PASS */
}

#ifndef ULONG_HIGH_BITS
#define ULONG_HIGH_BITS 32
#endif

static inline int
highbit(u_long mask)
{
    unsigned long hb = 1UL << (ULONG_HIGH_BITS - 1);
    int i;
    /* printf("%lx\n",hb); */
    for (i = ULONG_HIGH_BITS - 1; i >= 0; i--, mask <<= 1)
        if (mask & hb) break;
    return i;
}

void
image_set_col(ImageData *img, Display *dpy, Window win)
{
	struct palette *pal = img->pal;
	u_long *pixel_value = img->pixel_value;

	/* 再割り当てしようとしている */
	if (img->pixel_allocated) {
#ifdef DEBUG
		fprintf(stderr,"image_set_col: allocate again...");
#endif
		if (dpy != img->display || win != img->window) {
			image_free_pixels(img);
			image_free_pixmap(img);
#ifdef DEBUG
			fprintf(stderr,"allocated\n");
#endif
		} else {
#ifdef DEBUG
			fprintf(stderr,"not allocated\n");
#endif
			return;
		}
	}

	img->display = dpy;
	img->window  = win;

	/* colormap setup */
	{
		XWindowAttributes attr;
		XGetWindowAttributes(dpy, win, &attr);
		img->depth    = attr.depth;
		img->colormap = attr.colormap;
		/* めんどくさいけど Visual 構造体の中身はみちゃだめ */
		{
			int matched;
			XVisualInfo *vinfolist;
			XVisualInfo vinfo;
			vinfo.visualid = XVisualIDFromVisual(attr.visual);
			vinfolist = 
				XGetVisualInfo(img->display,VisualIDMask,&vinfo,&matched);
			if (!matched) {
				fprintf(stderr,"can't get visual info.");
				exit(1);
			}
			img->vinfo = vinfolist[0];
			XFree(vinfolist);
		}
	}

	switch (img->type) {
	case 2:
		fprintf(stderr,"no supported depth...\n");
		exit(1);
		break;
	case 8:
		if (img->vinfo.class == TrueColor) {
			int    i;
			u_long rmask, gmask, bmask;
			int    rshift, gshift, bshift;
			
			msg_out("8bit true color\n");

			rshift = highbit(rmask = img->vinfo.red_mask  ) - 7;
			gshift = highbit(gmask = img->vinfo.green_mask) - 7;
			bshift = highbit(bmask = img->vinfo.blue_mask ) - 7;

			msg_out("shift r:[%d] g:[%d] b:[%d]\n", rshift, gshift, bshift);

			for (i = 0; i < img->npal; i++, pal++) {
				u_long rv, gv, bv;
				rv = rshift < 0 ? pal->r >> -rshift : pal->r << rshift;
				gv = gshift < 0 ? pal->g >> -gshift : pal->g << gshift;
				bv = bshift < 0 ? pal->b >> -bshift : pal->b << bshift;
				pixel_value[i] = (rv & rmask)|(gv & gmask)|(bv & bmask);
			}
			for (; i < 256; i++)
				pixel_value[i] = 0L;
	
		} else {
			/* for 8-bit pseudo colors */
			int i, ncols, hist[MAX_COL], order[MAX_COL];
			struct color_table c_tbl[MAX_COL];
			
			msg_out("8bit pseudo color\n");

			ncols = make_histogram(img,hist);
			for (i=0; i< img->npal; i++, pal++) {
				c_tbl[i].r = pal->r;
				c_tbl[i].g = pal->g;
				c_tbl[i].b = pal->b;
			}
			sort_colors(img, hist, order, c_tbl, ncols);
			alloc_read_only_colors(img, c_tbl, ncols);

			for (i=0; i<ncols; i++)
				pixel_value[order[i]] = c_tbl[i].pixel;
		}
		break;
	case 24:
		if (img->vinfo.class != TrueColor) {
			fprintf(stderr,"no supported mode...\n");
			exit(1);
		}
		break;
	}
	img->pixel_allocated = 1;
}

int
image_read_stream(ImageData *img, FILE *fp)
{
	size_t n_loador = sizeof loadors / sizeof loadors[0];
	int i;
	for (i=0; i< n_loador; i++) {
		if (!loadors[i].read_stream(img,fp))
			return 0;
        rewind(fp);
	}
	return -1;
}


/* イメージデータの取得 */
void 
image_load(ImageData *img, char *name, int c0, int r0)
{
	FILE *fp;
	char *fname;
	int i;

	size_t n_loador = sizeof loadors / sizeof loadors[0];

	/* already loaded */
	if (img->data != NULL)
		return;

	if ((fname = search(name))== NULL)
		return;

	if ((fp = fopen(fname,"r"))== NULL) {
		perror(fname);
		return;
	}

	if (c0 >= 0) {
		img->trans_flag = TRANS_INDEX;
		img->trans_index = c0;
	} 
#if 0
else if (r0 >= 0) {
		img->trans_flag = TRANS_RGB;
		img->trans_rgb   = r0;
	}
#endif
	for (i=0; i< n_loador; i++) {
		if (!loadors[i].read_stream(img,fp))
			return;
        rewind(fp);
	}

	fprintf(stderr,"can't load %s.\n", fname);
	exit(1);

#if 0
	fclose(fp);
	if ((suffix = strrchr(name,'.'))== NULL) {
        err_out("File \"%s\" has no suffix.\n",name);
        return;
    }
    suffix++;

	{
		char cmdline[100];
		int fd[2];
		pid_t pid;

		if (pipe(fd)< 0) {
			perror("pipe");	exit(1);
		}
		if ((pid = fork())< 0) {
			perror("fork");	exit(1);
		}
		if (pid == 0) {
			if (close(1)< 0) {
				perror("close"); exit(1);
			}
			if (dup(fd[1])!= 1) {
				perror("dup"); exit(1);
			}
			close(fd[1]);
			sprintf(cmdline,"%stopnm",suffix);
			execlp(cmdline, cmdline, fname, (char*)NULL);
			sprintf(cmdline,"%stoppm",suffix);
			execlp(cmdline, cmdline, fname, (char*)NULL);
			sprintf(cmdline,"%stopgm",suffix);
			execlp(cmdline, cmdline, fname, (char*)NULL);
			sprintf(cmdline,"%stopbm",suffix);
			execlp(cmdline, cmdline, fname, (char*)NULL);
			perror("execlp");
			err_out("Can't find %s\n",cmdline);
			err_out("Bad suffix \"%s\".\n",suffix);
			exit(1);
		}
		close(fd[1]);
		if ((fp = fdopen(fd[0], "r"))== NULL) {
			perror("fdopen");
			exit(1);
		}
		pnm_read_stream(img,fp);
	}
#endif

}

#ifdef SHADOW

/* イメージをpixmapに変換する */
Pixmap
image_pixmap_with_shadow(ImageData *img, Pixmap *m, int shadow_len)
{
	int i,j;
	GC gc;

	unsigned w            = img->width;
	unsigned h            = img->height;
	u_long   *pixels      = img->allocated_pixel;
    u_long   *pixel_value = img->pixel_value;
	u_char   *data        = (u_char*)img->data;

	XImage *img_data;
	Pixmap p;
	u_long black;

#ifdef SHAPE
	XImage *img_mask;
	Pixmap b;
	int trans_index = img->trans_index;
	int make_mask;
	make_mask = (m && img->trans_flag == TRANS_INDEX)?1:0;
#endif

	/* 再割り当てしようとしている */
	if (img->pixmap != None) {
		if (m)
			*m = img->mask;
		return img->pixmap;
	}

	if (make_mask && shadow_len) {
		if (img->vinfo.class != TrueColor) {
			XColor col;
			col.red = col.blue = col.green = 0;
			col.flags = DoRed | DoGreen | DoBlue;
			if (XAllocColor(img->display,img->colormap,&col)) {
				black = pixels[img->npixel++] = col.pixel;
			} else {
				err_out("black allocation failed\n");
				black = 0;
			}
		}else
			black = 0;
	}	

	p = XCreatePixmap(img->display, img->window,
					  w+shadow_len,h+shadow_len,img->depth);
	img_data = XGetImage(img->display,p,0,0,w+shadow_len,h+shadow_len,
						 AllPlanes,ZPixmap);

#ifdef SHAPE
	if (make_mask) {
		b = XCreatePixmap(img->display, img->window,
						  w+shadow_len,h+shadow_len,1);
		img_mask = XGetImage(img->display,b,0,0,w+shadow_len,h+shadow_len,
							 1,ZPixmap);
	}
#endif /* SHAPE */

#ifdef SHAPE
	if (make_mask) {
		if (shadow_len) {
			for(i=h+shadow_len-1;i>=0;i--)
				for(j=w+shadow_len-1;j>=0;j--)
					if (i<h && j<w && data[i*w+j] != trans_index) {
						XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
						XPutPixel(img_mask,j,i,1);
						if ((i+j)& 1)
							XPutPixel(img_mask,j+shadow_len,i+shadow_len,1);
					} else {
						XPutPixel(img_data,j,i,black);
						XPutPixel(img_mask,j,i,0);
					}
		} else {
			for (i=0;i<h;i++)
				for (j=0;j<w;j++)
					if (data[i*w+j] != trans_index) {
						XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
						XPutPixel(img_mask,j,i,1);
					}else{
						XPutPixel(img_mask,j,i,0);
					}
		}
	}else
#endif /* SHAPE */
	{
		for(i=0;i<h;i++)
			for(j=0;j<w;j++)
				XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
	}

	gc = XCreateGC(img->display,p,0,NULL);
	XPutImage(img->display,p,gc,img_data,0,0,0,0,w+shadow_len,h+shadow_len);
	XFreeGC(img->display,gc);
	XDestroyImage(img_data);

#ifdef SHAPE
	if (m) {
		if(make_mask) {
			gc = XCreateGC(img->display,b,0,NULL);
			XPutImage(img->display,b,gc,img_mask,0,0,0,0,
					  w+shadow_len,h+shadow_len);
			*m = img->mask = b;
			XFreeGC(img->display,gc);
			XDestroyImage(img_mask);
		} else
			*m = img->mask = None;
	}
#endif /* SHAPE */

	if (verbose)
		fprintf(stderr,"convert image -> pixmap done.\n");

	img->pixmap = p;
	return p;
}

#endif /* SHADOW */


/* イメージをpixmapに変換する */
Pixmap
image_pixmap(ImageData *img, Pixmap *m)
{
	int i,j;
	GC gc;
	unsigned w            = img->width;
	unsigned h            = img->height;
    u_long   *pixel_value = img->pixel_value;
	
	u_char   *data        = img->data;
	XImage *img_data;
	Pixmap p;

#ifdef SHAPE
	XImage *img_mask;
	Pixmap b;
	int trans_index = img->trans_index;
	int make_mask;
	make_mask = (m && img->trans_flag == TRANS_INDEX)?1:0;
#endif /* SHAPE */

	/* 再割り当てしようとしている */
	if (img->pixmap != None) {
		if (m)
			*m = img->mask;
		return img->pixmap;
	}

	p = XCreatePixmap(img->display, img->window, w, h, img->depth);
	img_data = XGetImage(img->display,p,0,0,w,h,AllPlanes,ZPixmap);

#ifdef SHAPE
	if (make_mask) {
		b = XCreatePixmap(img->display, img->window, w, h, 1);
		img_mask = XGetImage(img->display,b,0,0,w,h,1,ZPixmap);
	}
#endif /* SHAPE */

#ifdef SHAPE
	if (make_mask) {
		for (i=0;i<h;i++)
			for (j=0;j<w;j++)
				if (data[i*w+j] != trans_index) {
					XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
					XPutPixel(img_mask,j,i,1);
				}else{
					XPutPixel(img_mask,j,i,0);
				}
	}else
#endif /* SHAPE */
	{
		for (i=0;i<h;i++)
			for (j=0;j<w;j++)
				XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
	}

	gc = XCreateGC(img->display,p,0,NULL);
	XPutImage(img->display,p,gc,img_data,0,0,0,0,w,h);
	XFreeGC(img->display,gc);
	XDestroyImage(img_data);

#ifdef SHAPE
	if (m) {
		if (make_mask) {
			gc = XCreateGC(img->display,b,0,NULL);
			XPutImage(img->display,b,gc,img_mask,0,0,0,0,w,h);
			*m = b;
			img->mask = b;

			XFreeGC(img->display,gc);
			XDestroyImage(img_mask);
		}else
			*m = img->mask = None;
	}
#endif /* SHAPE */

	if (verbose)
		fprintf(stderr,"convert image -> pixmap done.\n");

	img->pixmap = p;
	return p;
}

void *
image_alloc_palette(ImageData *img, size_t n)
{
	struct palette *p;
    size_t size = sizeof p[0] * n ;

    p = img->pal ? realloc(img->pal,size) : malloc(size);
    if (p) {
        img->pal  = p;
		img->npal = n;
    } else {
        perror(img->pal ? "realloc" : "malloc");
        fprintf(stderr,"image-lib:alloc_palette failed."
				"%u,%u(%#x) bytes\n", n, size, size);
		exit(1);
    }
    return p;
}

void *
image_alloc_data(ImageData *img, int w, int h, int t)
{
    size_t size;
    void *p;

    switch(t) {
    case 2:
        size = (w + 7)/8 * h;
		break;
    case 8:
        size = w * h;
        break;
    case 24:
        size = w * h * 3;
        break;
    default:
        fprintf(stderr,"Warning:no support image type:%d\n",t);
        return NULL;
    }

    if ((p = img->data ? realloc(img->data,size) : malloc(size)) == NULL) {
        perror(img->data ? "realloc" : "malloc");
		fprintf(stderr,"image:alloc_data "
				"%dx%d %u(%#x) bytes failed.\n", w,h,size,size);
		exit(1);
	}

	img->width  = w;
	img->height = h;
	img->data   = p;
	img->type   = t;

    return p;
}
