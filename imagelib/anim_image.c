#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include "animate.h"

#ifdef SHAPE
#include  <X11/extensions/shape.h>
#endif

RealImage *
realimage_new(void)
{
	RealImage *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("realimage_new");
		exit(1);
	}
	p->mode = None;
	p->realized = 0;
	p->foreground_name = NULL;
	p->foreground_pixel = 0;
	p->image = NULL;
	return p;
}

void
realimage_delete(Animate *a, RealImage *img)
{
	if (img) {
		switch(img->mode) {
		case RI_NONE:
			break;
		case RI_PLANE:
			if (img->foreground_name != NULL) {
				free(img->foreground_name);
			}
			if (img->realized == 1)
				XFreeColors(a->dpy, a->colormap,
							&img->foreground_pixel, 1, 0);
			break;
		case RI_PIXMAP:
			image_delete(img->image);
			break;
		}
		free(img);
	}
}

RealImage *
realimage_new_plane(char *color_name)
{
	RealImage *rimg = realimage_new();
	rimg->foreground_name = strdup(color_name);
	rimg->mode = RI_PLANE;
	return rimg;
}

RealImage *
realimage_new_allocated_pixel(u_long pixel)
{
	RealImage *rimg = realimage_new();
	rimg->foreground_pixel = pixel;
	rimg->realized = 2;
	return rimg;
}

RealImage *
realimage_new_image(ImageData *image)
{
	RealImage *rimg = realimage_new();
	rimg->image = image_attach(image);
	rimg->mode  = RI_PIXMAP;
	return rimg;
}

void
realimage_realize(Animate *a, RealImage *img)
{
	if (img && !img->realized) {
		switch (img->mode) {
		case RI_NONE:
			break;
		case RI_PLANE:
			/* alloc foreground color */
			{
				XColor s,e;
				if (!XAllocNamedColor(a->dpy, a->colormap, 	
									  img->foreground_name, &s, &e))
					fprintf(stderr,"can't get color named \"%s\". using 0",
							img->foreground_name);
				img->foreground_pixel = s.pixel;
			}
			break;
		case RI_PIXMAP:
			{
				Pixmap dmy;
				image_set_col(img->image, a->dpy, a->win);
				(void)image_pixmap(img->image, &dmy);
			}
			break;
		}
		img->realized = 1;
	}
}


/* マスクパターン描画 shape なし */
static void
maskpat_draw(Animate *a, AnimatePat *pat)
{
	RealImage *image = pat->image;
	int w = pat->clip_w;
	int h = pat->clip_h;
	int unit = a->unit;

	if (image->mode == RI_PLANE) {	/* plane screen */
		if (w == 0) w = a->w - pat->dst_x;
		if (h == 0) h = a->h - pat->dst_y;

		XSetForeground(a->dpy, a->gc, image->foreground_pixel);
		XSetClipMask(a->dpy, a->gc, None);
		XSetStipple(a->dpy, a->gc, 
					a->mask_pattern[(pat->mask_mode-1)*8+pat->mask_cnt]);
		XSetFillStyle(a->dpy, a->gc, FillStippled);
		XFillRectangle(a->dpy, a->pixmap, a->gc,
					   pat->dst_x, pat->dst_y, w, h);

	} else if (image->mode == RI_PIXMAP) {	/* DA_PIXMAP */
		if (w == 0) w = image->image->width;
		if (h == 0) h = image->image->height;

#if 0
		XSetClipMask(a->dpy, a->gc,
					 a->mask_pattern[(pat->mask_mode-1)*8+pat->mask_cnt]);
		XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
				  a->gc, pat->src_x, pat->src_y, w, h,
				  pat->dst_x, pat->dst_y);
#else
		{
			int i, j;
			int end_i = w / unit;
			int end_j = w / unit;
			int y2 = end_i * unit;
			int x2 = end_j * unit;
			XSetClipMask(a->dpy, a->gc,
						 a->mask_pattern[(pat->mask_mode-1)*8+pat->mask_cnt]);
			for (i=0;i<end_i;i++) {
				int y = i * unit;
				for (j=0;j<end_j;j++) {
					int x = j * unit;
					XSetClipOrigin(a->dpy, a->gc,
								   pat->dst_x + x, pat->dst_y + y);
					XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
							  a->gc, pat->src_x + x, pat->src_y + y,
							  unit, unit, pat->dst_x + x, pat->dst_y + y);
				}
				if (x2 < w) {
					XSetClipOrigin(a->dpy, a->gc, 
								   pat->dst_x + x2, pat->dst_y + y);
					XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
							  a->gc, pat->src_x + x2, pat->src_y + y, 
							  w - x2, unit, pat->dst_x+x2, pat->dst_y+y);
				}
			}
			if (y2 < h) {
				for (j=0;j<end_j;j++) {
					int x = j * unit;
					XSetClipOrigin(a->dpy, a->gc, 
								   pat->dst_x + x, pat->dst_y + y2);
					XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
							  a->gc, pat->src_x + x, pat->src_y + y2, 
							  unit, h - y2, pat->dst_x+x, pat->dst_y+y2);
				}
				if (x2 < w) {
					XSetClipOrigin(a->dpy, a->gc, 
								   pat->dst_x + x2, pat->dst_y + y2);
					XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
							  a->gc, pat->src_x + x2, pat->src_y + y2, 
							  w - x2, h - y2, pat->dst_x+x2, pat->dst_y+y2);
				}
			}
#endif 
		}
	}
}

/* 差分追加モード */
static void
diffpat_draw(Animate *a, AnimatePat *pat)
{
	RealImage *image = pat->image;
	int w = pat->clip_w;
	int h = pat->clip_h;

	if (image->mode == RI_PLANE) {	/* plane screen */

		if (w == 0) w = a->w - pat->dst_x;
		if (h == 0) h = a->h - pat->dst_y;

		if (image->foreground_pixel == TRANSPARENT_PIXEL) {
			XSetFunction(a->dpy, a->gcm, GXclear);
			XFillRectangle(a->dpy, a->mask, a->gcm,
						   pat->dst_x, pat->dst_y, w, h);
		} else {
			XSetForeground(a->dpy, a->gc, image->foreground_pixel);
			XSetClipMask(a->dpy, a->gc, None);
			XSetFillStyle(a->dpy, a->gc, FillSolid);
			XFillRectangle(a->dpy, a->pixmap, a->gc,
						   pat->dst_x, pat->dst_y, w, h);
		}
	} else if (image->mode == RI_PIXMAP) {	/* DA_PIXMAP */
		if (w == 0) w = image->image->width;
		if (h == 0) h = image->image->height;
		XSetClipMask(a->dpy, a->gc, image->image->mask);
		if (image->image->mask != None) {
			XSetClipOrigin(a->dpy, a->gc, 
						   pat->dst_x - pat->src_x,
						   pat->dst_y - pat->src_y);
			XSetFunction(a->dpy, a->gcm, GXor);
			XCopyArea(a->dpy, image->image->mask, a->mask, 
					  a->gcm, pat->src_x, pat->src_y, w, h,
					  pat->dst_x, pat->dst_y);
		}	
		XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
				  a->gc, pat->src_x, pat->src_y, w, h,
				  pat->dst_x, pat->dst_y);
		
	}
}

static void
changepat_draw(Animate *a, AnimatePat *pat)
{
	RealImage *image = pat->image;

	if (image->mode == RI_PLANE) {
		XSetForeground(a->dpy, a->gc, image->foreground_pixel);
		XSetFillStyle(a->dpy, a->gc, FillSolid);
		XFillRectangle(a->dpy, a->pixmap, a->gc, 0, 0, a->w, a->h);
		XSetFunction(a->dpy, a->gcm, GXclear);
		XFillRectangle(a->dpy, a->mask, a->gcm, 0, 0, a->w, a->h);
	} else if (image->mode == RI_PIXMAP) {
		if (image->image->mask != None) {
			XSetFunction(a->dpy, a->gcm, GXcopy);
			XCopyArea(a->dpy, image->image->mask, a->mask, 
					  a->gcm, 0, 0, a->w, a->h, 0, 0);
		} else {
			XSetFunction(a->dpy, a->gcm, GXset);
			XFillRectangle(a->dpy, a->mask, a->gcm, 0, 0, a->w, a->h);
		}
		XSetClipMask(a->dpy, a->gc, None);	
		XCopyArea(a->dpy, image->image->pixmap, a->pixmap,
				  a->gc, 0, 0, a->w, a->h, 0, 0);
	}
}

void
animpat_draw(Animate *a, AnimatePat *pat)
{
	if (pat->mode == AP_NONE || pat->image == NULL 
		|| pat->image->mode == RI_NONE)
		return;
	switch (pat->mode) {
	case AP_NORMAL:
		changepat_draw(a,pat);
		break;
	case AP_DIFF:
		diffpat_draw(a,pat);
		break;
	case AP_MASKED:
		maskpat_draw(a,pat);
		break;
	default:
		break;
	}
}

void
animate_expose(Animate *a, int x, int y, int w, int h)
{
 	XCopyArea(a->dpy, a->pixmap, a->win, a->gc2, x, y, w, h, x, y);
}

void
animate_expose_region(Animate *a, Region reg)
{
	XSetRegion(a->dpy, a->gc2, reg);
 	XCopyArea(a->dpy, a->pixmap, a->win, a->gc2, 0, 0, a->w, a->h, 0,0);
	XSetClipMask(a->dpy, a->gc2, None);
}

void
animate_set_shape(Animate *a)
{
#ifdef SHAPE
	XShapeCombineMask(a->dpy, a->win, ShapeBounding, 0, 0, a->mask, ShapeSet);
#endif
}
