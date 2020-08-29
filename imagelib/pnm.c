/*
   Modified By Go Watanabe

  original:
    ppm.cc,v 1.6 1997/05/05 03:44:26 watanabe Exp
	load routine for 'pm' format pictures
*/

#include <stdio.h>
#include <ctype.h>

#include "image.h"

extern int verbose;

static inline unsigned
get_short(FILE *fp)
{
	unsigned c0, c1;
	c0 = getc(fp),c1 = getc(fp);
	return (c1 << 8) | c0;
}

static int
get_int(FILE *fp)
{
	int c;

	/* note:  if it sees a '#' character, all characters from there to end of
	line are appended to the comment string */

	/* skip forward to start of next number */
	while(c = getc(fp),isspace(c))
		continue;

	/* eat comments */
	while(c == '#') {
		while(c = getc(fp),c != '\n')
			if(c == EOF) return EOF;
		while(c = getc(fp),isspace(c))
			continue;
	}

	/* we're at the start of a number, continue until we hit a non-number */
	if(isdigit(c)) {
		int deg = c - '0';

		while(c = getc(fp),isdigit(c))
			deg = deg * 10 + c - '0';

		return deg;
	}

	fprintf(stderr,"Error:ppm header value \"%c\" %d.\n",c,c);

	return c;
}

static int
get_bit(FILE *fp)
{
	int c;

	/* skip forward to start of next number */

	while(c = getc(fp),isspace(c))
		continue;

	/* eat comments */
	while(c == '#') {
		while(c = getc(fp),c != '\n')
			if(c == EOF) return EOF;

		while(c = getc(fp),isspace(c))
			continue;
	}

	return (c == '0' || c == '1') ? c - '0' : c;
}


/* pnm global variables */
static int type, width, height;
static int max_value;

static int
query_header(ImageData *img, FILE *fp)
{
	int c,c1,value;

/*
	read the first two bytes of the file to determine which format
	this file is.  "P1" = ascii bitmap, "P2" = ascii greymap,
	"P3" = ascii pixmap, "P4" = raw bitmap, "P5" = raw greymap,
	"P6" = raw pixmap 
*/
	c = getc(fp); c1 = getc(fp);
	if (c != 'P' || c1 < '1' || c1 > '6')
		return -1;

	/* read in header information */
	type   = c1 - '0';
	width  = get_int(fp);
	height = get_int(fp);

	/* if we're not reading a bitmap, read the 'max value' */
	if (!(c1 == '1' || c1 == '4')) {
		if ((value = get_int(fp)) < 0)
			return -1;
		max_value  = value;
	}

	return 0;
}

static int
load_pbm(ImageData *img, FILE *fp)
{
	int c,ct;
	unsigned w,h,bit;
	u_char *pic8;
	struct palette *pal;

	if (!(pal = image_alloc_palette(img,2)))
		return -1;
	/* B/W bitmaps have a two entry colormap */
	pal[0].r = pal[0].g = pal[0].b = 255;   /* entry #0 = white */
	pal[1].r = pal[1].g = pal[1].b = 0;     /* entry #1 = black */

	pic8 = (u_char *)img->data;
	if (type == 1) {	/* ascii */
		c = 0;
		bit = 0;
		for (h = 0; h < height; h++) {
			ct = 0;
			for (w = 0; w < width; w++) {
				if (!bit)
					bit = 7;
				if (get_bit(fp))
					*pic8 |= (1 << bit);
				bit--;
				if (++ct == 8)
					ct = 0, pic8++;
			}
		}
		return 0;
	}
					
	if (width % 8 == 0) {
		if (fread(pic8,1,width/8 * height,fp) < width/8 * height)
			perror("fread");
		return 0;
	}

	c = 0;
	bit = 0;

	for (h = 0; h < height; h++) {
		ct = 0;
		for (w = 0; w < width; w++) {
			if (!bit) {
				c = getc(fp);
				bit = 7;
			}
			if (c & 0x80)
				*pic8 |= (1 << bit);
			c <<= 1;
			bit--;
			if (++ct == 8)
				ct = 0, pic8++;
		}
	}
	return 0;
}

static int
load_pgm(ImageData *img, FILE *fp)
{
	u_char *pic8;
	int i,c,n,bitshift,value;
	struct palette *pal;

	pic8 = (u_char *)img->data;
	pal  = img->pal;

	/* if max_value >255, keep dropping bits until it's reasonable */

	value = max_value;
	for (bitshift = 0; value > 255; value >>= 1,bitshift++)
		continue;

	if (!(pal = image_alloc_palette(img, value + 1)))
		return -1;

	/* fill in a greyscale colormap where max_value maps to 255 */

	for (i = 0;i <= value; i++)
		pal[i].r = pal[i].g = pal[i].b = (i*255)/value;

	if (type == 2) {	/* ascii */
		for (i = width * height; i; i--) {
			if ((c = get_int(fp)) == EOF)
				return -1;
			*pic8++ = c >> bitshift;
		}
		return 0;
	}

	/* raw */

	if (max_value > 255) {
		for (i = width * height; i; i--)
			*pic8++ = get_short(fp) >> bitshift;
	}
	else {
		n = fread(pic8, 1, width * height, fp);
		if (n < width * height)
			return -1;
	}

	return 0;
}

static int
load_ppm(ImageData *img, FILE *fp)
{
	u_char *pic24;
	int i,c,n,bitshift,value;

	pic24 = (u_char *)img->data;

	// if value>255, keep dropping bits until it's reasonable
	value = max_value;
	for (bitshift = 0; value > 255; value >>= 1,bitshift++)
		continue;

	if (type == 3) {
		for (i = width * height * 3; i; i--) {
			if ((c = get_int(fp)) == EOF)
				return -1;
			*pic24++ = c >> bitshift;
		}
	}
	else { /* raw */
		if (max_value > 255) {
			for(i = width * height * 3; i; i--)
				*pic24++ = get_short(fp) >> bitshift;
		}
		else {
			n = fread(pic24, 1, width * height * 3, fp);
			if (n < width * height * 3)
				return -1;
		}
	}

	/*  have to scale all RGB values up 
		(Conv24to8 expects RGB values to range from 0-255 */

	if (value < 255) { 
		u_char scale[256];

		for (i = 0; i <= value; i++)
			scale[i] = (i * 255) / value;

		pic24 = (u_char *)img->data;

		for (i = width * height * 3; i; i--,pic24++)
			*pic24 = scale[*pic24];

		if (verbose)
			fprintf(stderr,"ppm scaled.\n");
	}

#if 0
	pic24 = (octet_t *)data;

	for (i = width * height * 3; i; i--,pic24+=3)
		fprintf(stderr,"ppm:%d %d %d",pic24[0],pic24[1],pic24[2]);
#endif

	return 0;
}


int
pnm_read_stream(ImageData *img, FILE *fp)
{
	if(query_header(img,fp) <0)
		return -1;

	switch(type) {
	case 1: 
	case 4: 
		return image_alloc_data(img,width,height,2) ? load_pbm(img,fp) : -1;
	case 2: 
	case 5:
		return image_alloc_data(img,width,height,8) ? load_pgm(img,fp) : -1;
	case 3: 
	case 6:
		return image_alloc_data(img,width,height,24) ? load_ppm(img,fp) : -1;
	}
	return -1;
}  
