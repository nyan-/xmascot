/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

/* mag ファイルのロード */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef SunOS
extern char *sys_errlist[];
#define strerror(n) (sys_errlist[n])
#endif

#include "image.h"

extern int verbose;

static inline unsigned
get_short(FILE *fp)
{
	unsigned c0, c1;
    c0 = getc(fp), c1 = getc(fp);
    return (c1 << 8)|c0;
}

static inline unsigned long
get_long(FILE *fp)
{
    unsigned long c0, c1, c2, c3;
    c0 = getc(fp), c1 = getc(fp);
    c2 = getc(fp), c3 = getc(fp);
	return (c3 << 24)|(c2 << 16)|(c1 << 8)|c0;
}

/* mag global variables */
static int width, height;
static int screen_mode,max_color, double_line;
static long flagA_offset, flagB_offset, flagB_size;
static long pixel_offset, pixel_size;
static u_char *data;

static int
query_header(ImageData *img, FILE *fp)
{
    int i,ch;
	char buf[10];
    int left, top, right, bottom;

	/* MAG header */
	if (fread(buf,1,8,fp) < 8) {
		perror("fread");
		return -1;
	}

	if (strncmp(buf,"MAKI02  ",8) != 0)
		return -1;

	if (verbose)
		fprintf(stderr, "read mag file...\n");

	/* ignore comment */
	while ((ch = getc (fp)) != 0x1a)	
		if (ch == EOF)
			return -1;

    getc(fp);	/* head */
    getc(fp);	/* machine code */
    getc(fp);	/* machine dependence */

    screen_mode = getc(fp);
    left        = get_short(fp);
    top         = get_short(fp);
    right       = get_short(fp);
    bottom      = get_short(fp);

    flagA_offset = get_long(fp);
    flagB_offset = get_long(fp);
    flagB_size   = get_long(fp);
    pixel_offset = get_long(fp);
    pixel_size   = get_long(fp);

    width  = (right & 0xff8) - (left & 0xff8) + 8;
    height = bottom - top + 1;

	if ((screen_mode & 0x80) == 0) {
		max_color = 16;
		double_line = (screen_mode & 1) ? 1 : 0;
		if (double_line)
			height *= 2;
	} else {
		max_color = 256;
		double_line = 0;
    }

	/* read palette table */
	{
		struct palette *p = image_alloc_palette(img,max_color);
		for(i = 0; i < max_color; i++, p++) {
			p->g = getc(fp);
			p->r = getc(fp);
			p->b = getc(fp);
		}
		if(verbose)
			fprintf(stderr,"debug:MAG file has %d colors.\n",max_color);
		if (feof(fp))
			return -1;
	}

	return 0;
}

static int dx_orig[] = {
	0,1,2,4,0,1,0,1,2,0,1,2,0,1,2,0 
};

static int dy_orig[] = { 
	0,0,0,0,1,1,2,2,2,4,4,4,8,8,8,16 
};

static int bit_field[] = {
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

static void
decode16(u_char *flagA,u_char *flagB,u_char *buf,FILE *fp)
{
    int DX[32],DY[32];
	int i,flag,c0,c1;
	int x,y,gy,xx,x8,xp;
    int width8;
	u_char *pic8 = (u_char *)data;

    for (i = 0; i < 16; i++) {
		DX[i] = dx_orig[i];
		DY[i] = dy_orig[i] * width;
    }

    width8 = (width - 1) >> 3;

    xx = gy = 0;
    for (y = 0; y < height; y++, gy += width)
		for (x = 0; x <= width8; x++, xx++) {
			x8 = x * 8;
			xx &= 7;
			if (*flagA & bit_field[xx])
				buf[x] ^= *flagB++;
			if (xx == 7)
				flagA++;

			flag = (buf[x] >> 4) & 0x0f;
			if (flag == 0) {
				c0 = getc(fp);
				pic8[x8 + gy] = (c0 >> 4) & 0x0f;
				pic8[x8 + gy + 1] = c0 & 0x0f;
				c1 = getc(fp);
				pic8[x8 + gy + 2] = (c1 >> 4) & 0x0f;
				pic8[x8 + gy + 3] = c1 & 0x0f;
			} else {
				xp = x8 - (DX[flag] << 2);
				pic8[x8 + gy] = pic8[xp + gy - DY[flag]];
				pic8[x8 + gy + 1] = pic8[xp + gy - DY[flag] + 1];
				pic8[x8 + gy + 2] = pic8[xp + gy - DY[flag] + 2];
				pic8[x8 + gy + 3] = pic8[xp + gy - DY[flag] + 3];
			}

			flag = buf[x] & 0x0f;
			if (flag == 0) {
				c0 = getc(fp);
				pic8[x8 + gy + 4] = (c0 >> 4) & 0x0f;
				pic8[x8 + gy + 5] = c0 & 0x0f;
				c1 = getc(fp);
				pic8[x8 + gy + 6] = (c1 >> 4) & 0x0f;
				pic8[x8 + gy + 7] = c1 & 0x0f;
			} else {
				xp = x8 + 4 - (DX[flag] << 2);
				pic8[x8 + gy + 4] = pic8[xp + gy - DY[flag] ];
				pic8[x8 + gy + 5] = pic8[xp + gy - DY[flag] + 1];
				pic8[x8 + gy + 6] = pic8[xp + gy - DY[flag] + 2];
				pic8[x8 + gy + 7] = pic8[xp + gy - DY[flag] + 3];
			}
		}
}

static void
decode256(u_char *flagA,u_char *flagB,u_char *buf,FILE *fp)
{
    int DX[32],DY[32];
    int i,flag;
	int x,y,gy,xx,x8,xp;
    int width8;
	u_char *pic8 = (u_char *)data;

    for (i = 0;i < 16;i++) {
		DX[i] = dx_orig[i];
		DY[i] = dy_orig[i] * width;
    }

    width8 = (width-1) >> 2;

    xx = gy = 0;
    for (y = 0; y < height; y++, gy += width)
		for (x = 0; x <= width8; x++, xx++) {
			x8 = x * 4;
			xx &= 7;
			if(*flagA & bit_field[xx])
				buf[x] ^= *flagB++;
			if(xx == 7)
				flagA++;

			flag = (buf[x] >> 4) & 0x0f;
			if(flag == 0) {
				pic8[x8 + gy] = getc(fp);
				pic8[x8 + gy + 1] = getc(fp);
			}
			else {
				xp = x8 - DX[flag] * 2;
				pic8[x8 + gy] = pic8[xp + gy - DY[flag]];
				pic8[x8 + gy + 1] = pic8[xp + gy - DY[flag] + 1];
			}

			flag = buf[x] & 0x0f;
			if(flag == 0) {
				pic8[x8 + gy + 2] = getc(fp);
				pic8[x8 + gy + 3] = getc(fp);
			}
			else {
				xp = x8 + 2 - DX[flag] * 2;
				pic8[x8 + gy + 2] = pic8[xp + gy - DY[flag]];
				pic8[x8 + gy + 3] = pic8[xp + gy - DY[flag] + 1];
			}
		}
}

int
mag_read_stream(ImageData *img, FILE *fp)
{
    int x,y,i,j;
	u_char *work, *flagA, *flagB;	/* work area */
	size_t a,b,size;

	if (query_header(img, fp) <0)
		return -1;

	if (!(data = image_alloc_data(img, width, height, 8)))
		return -1;
	
	size = (width + 16)
		+ (a = flagB_offset - flagA_offset )+ 256
		+ (b = pixel_offset - flagB_offset )+ 256;

	if ((work = (u_char *)alloca(size)) == NULL) {
		fprintf(stderr,"alloca:%s\n"
				"Mag Image:read_stream image failed.\n",strerror(errno)); 
		return -1;
	}

	flagA = work + width + 16;
	flagB = flagA + a + 256;

    if (fread(flagA,1,a,fp) < a || fread(flagB,1,b,fp) < b) {
		fprintf(stderr,"fread:%s\n"
				"Mag Image:read_stream image failed.\n",strerror(errno)); 
		return -1;
	}

    for (i = 0; i < width; i++)
        work[i] = 0;

    if (max_color == 16)
		decode16(flagA,flagB,work,fp);
    else
		decode256(flagA,flagB,work,fp);

	/* 縦2倍 */
    if (double_line) {
		u_char *pic8 = (u_char *)img->data;

		for (y = height / 2 - 1;y >= 0;y--) {
			i = y * 2 * width;
			j = y * width;
			for (x = width - 1;x >= 0;x--) {
				pic8[x + i] = pic8[x + j];
				pic8[x + i + width] = pic8[x + j];
			}
		}
    }

	if (img->trans_flag == TRANS_AUTO) {
		img->trans_flag  = TRANS_INDEX;
		img->trans_index = ((u_char*)img->data)[0];
	}
	return 0;
}
