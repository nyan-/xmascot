/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

/* tif ファイルのロード */

#include<stdio.h>
#include<stdlib.h>
#include <X11/Xlib.h>
#include "image.h"

int             tiflzwdecode(u_char * data, FILE * fp);

/* TIFF タグ */
typedef struct {
	short           id;
	short           type;
	long            num;
	long            data;
}               TAG;

#define TAG_NewSubfileType                              0x0fe
#define TAG_SubfileType                                 0x0ff
#define TAG_ImageWidth                                  0x100
#define TAG_ImageLength                                 0x101
#define TAG_BitsPerSample                               0x102
#define TAG_Compression                                 0x103
#define TAG_PhotometricInterpretation                   0x106
#define TAG_FillOrder                                   0x10a
#define TAG_StripOffset                                 0x111
#define TAG_SamplesPerPixel                             0x115
#define TAG_RowsPerStrip                                0x116
#define TAG_StripsByteCount                             0x117
#define TAG_MaxSampleValue                              0x119
#define TAG_XResolution                                 0x11a
#define TAG_YResolution                                 0x11b
#define TAG_PlanerConfiguration                         0x11c
#define TAG_ColorMap                                    0x140

#define TAG_Software                                    0x131

#define BYTE            1
#define ASCII           2
#define SHORT           3
#define LONG            4
#define RATIONAL        5
#define DATALEN(x) (((x)==SHORT)?2:((x)==LONG)?4:((x)==RATIONAL)?8:1)

#define TAGCHK_SubfileType                              0x00000001UL
#define TAGCHK_ImageWidth                               0x00000002UL
#define TAGCHK_ImageLength                              0x00000004UL
#define TAGCHK_BitsPerSample                            0x00000008UL
#define TAGCHK_Compression                              0x00000010UL
#define TAGCHK_PhotometricInterpretation                0x00000020UL
#define TAGCHK_StripOffset                              0x00000040UL
#define TAGCHK_SamplesPerPixel                          0x00000080UL
#define TAGCHK_PlanerConfiguration                      0x00000100UL
#define TAGCHK_ColorMap                                 0x00000200UL
#define TAGCHK_RowsPerStrip                             0x00000400UL

#define TAGCHK_OK ( \
  TAGCHK_ImageWidth                 | \
  TAGCHK_ImageLength                | \
  TAGCHK_BitsPerSample              | \
  TAGCHK_Compression                | \
  TAGCHK_PhotometricInterpretation  | \
  TAGCHK_StripOffset                | \
  TAGCHK_SamplesPerPixel            | \
  TAGCHK_PlanerConfiguration)

#define Color2       1
#define Color16      2
#define Color256     3
#define Color32k     4
#define FullColor    5
#define ColorILLEGAL 0

/* tif ワークエリア */
typedef struct {
	int             sx, sy;
	int             comp;
	int             photo;
	unsigned long   stpofs;
	int             stpsize;
	int             stpnum;
	unsigned long   stpofsofs;
	unsigned long   stpsizeofs;
	int             rowsPerStp;
	int             PlanerConf;
	unsigned long   cmapofs;
	int             type;
	int             palnum;
	int             linebyte;
	int             pudding;
}               TIFWORK;

static TIFWORK *tw;

static int      big;		/* バイト順フラグ */
static unsigned char *data = NULL;

/* 32BIT 読みだし */
static int 
getint(FILE * fp)
{
	int             ret = 0x00;
	int             i;
	for (i = 0; i < 4; i++)
		if (big != 0)
			ret = (ret << 8) | (0xff & fgetc(fp));
		else
			ret = ret | ((0xff & fgetc(fp)) << (8 * i));
	return ret;
}

/* 16BIT 読みだし */
static int 
getshort(FILE * fp)
{
	int             ret = 0x00;

	ret = fgetc(fp);
	if (big != 0)
		ret = (ret << 8) | (fgetc(fp) & 0xff);
	else
		ret = ret | ((0xff & fgetc(fp)) << 8);
	return ret;
}

/* １つのタグ読みだし */
static int 
gettag(FILE * fp, TAG * tag)
{
	tag->id = getshort(fp);
	tag->type = getshort(fp);
	tag->num = getint(fp);
	if (tag->type == SHORT && DATALEN(tag->type) * (tag->num) <= 4) {
		tag->data = getshort(fp);
		getshort(fp);
	} else
		tag->data = getint(fp);
	return 0;
}

/* ヘッダのチェック */
static int 
query_tif(FILE * fp)
{
	int             c, c1;

	rewind(fp);
	c = fgetc(fp);
	c1 = fgetc(fp);
	if (c == 'I' && c1 == 'I')
		big = 0;
	else if (c == 'M' && c1 == 'M')
		big = 1;
	else
		return -1;
	return 0;
}

/* タグのチェック */
static int
read_tags(FILE * fp)
{
	int             tagnum;
	TAG             tag;
	unsigned long   add;
	unsigned long   tagchk = 0x0;
	int             i, j;

	tagnum = getshort(fp);
	for (i = 0; i < tagnum; i++) {
		gettag(fp, &tag);
		switch (tag.id) {
		case TAG_ImageWidth:
			if (tag.type != SHORT || tag.num != 1)
				break;
			tw->sx = tag.data;
			tagchk |= TAGCHK_ImageWidth;
			break;
		case TAG_ImageLength:
			if (tag.type != SHORT || tag.num != 1)
				break;
			tw->sy = tag.data;
			tagchk |= TAGCHK_ImageLength;
			break;
		case TAG_BitsPerSample:
			if (tag.type != SHORT || !(tag.num == 1 || tag.num == 3))
				break;
			if (tag.num == 1) {
				if (tag.data == 4)
					tw->type = Color16;
				else if (tag.data == 8)
					tw->type = Color256;
				else
					break;
			} else
				break;
			tagchk |= TAGCHK_BitsPerSample;
			break;
		case TAG_Compression:
			if (tag.type != SHORT || tag.num != 1)
				break;
			if (tag.data != 5 && tag.data != 1)
				break;
			tw->comp = (tag.data == 5) ? 1 : 0;
			tagchk |= TAGCHK_Compression;
			break;
		case TAG_PhotometricInterpretation:
			if (tag.type != SHORT || tag.num != 1)
				break;
			tw->photo = tag.data;
			tagchk |= TAGCHK_PhotometricInterpretation;
			break;
		case TAG_StripOffset:
			if (tag.type != LONG)
				break;
			tw->stpnum = tag.num;
			if (tag.num == 1)
				tw->stpofs = tag.data;
			else
				tw->stpofsofs = tag.data;
			tagchk |= TAGCHK_StripOffset;
			break;
		case TAG_SamplesPerPixel:
			if (tag.type != SHORT || tag.num != 1)
				break;
			if (tag.data != ((tw->type == FullColor) ? 3 : 1))
				break;
			tagchk |= TAGCHK_SamplesPerPixel;
			break;
		case TAG_RowsPerStrip:
			if ((tag.type != SHORT && tag.type != LONG) || tag.num != 1)
				break;
			tw->rowsPerStp = tag.data;
			tagchk |= TAGCHK_RowsPerStrip;
			break;
		case TAG_StripsByteCount:
			if (tag.type != LONG)
				break;
			if (tag.num == 1)
				tw->stpsize = tag.data;
			else
				tw->stpsizeofs = tag.data;
			break;
		case TAG_PlanerConfiguration:
			if (tag.type != SHORT || tag.num != 1)
				break;
			if (tag.data != 1)
				break;
			tagchk |= TAGCHK_PlanerConfiguration;
			break;
		case TAG_ColorMap:
			if (tag.type != SHORT)
				break;
			if (tag.num != 48 && tag.num != 768)
				break;
			tw->cmapofs = tag.data;
			tw->palnum = tag.num / 3;
			add = ftell(fp);
			fseek(fp, tag.data, 0);

			{
				struct palette *p = image_alloc_palette(img, tw->palnum);
				for (j = 0; j < tw->palnum; j++) {
					p[j].r = (getshort(fp) >> 8) & 0xff;
				}
				for (j = 0; j < tw->palnum; j++) {
					p[j].g = (getshort(fp) >> 8) & 0xff;
				}
				for (j = 0; j < tw->palnum; j++) {
					p[j].b = (getshort(fp) >> 8) & 0xff;
				}
			}

			fseek(fp, add, 0);
			tagchk |= TAGCHK_ColorMap;
			break;
		}
	}
	if ((tagchk & TAGCHK_OK) != TAGCHK_OK) {
		return -1;
	}
	if (tw->stpnum != 1 && (tagchk & TAGCHK_RowsPerStrip) == 0) {
		return -1;
	}
	switch (tw->type) {
	case Color16:
		if (!(tagchk & TAGCHK_ColorMap))
			return -1;
		tw->linebyte = (tw->sx + 1) / 2;
		break;
	case Color256:
		if (!(tagchk & TAGCHK_ColorMap))
			return -1;
		tw->linebyte = tw->sx;
		break;
	default:
		return -1;
	}
	tw->pudding = 0;
	if (tw->type == Color16)
		tw->pudding = ((tw->sx + 7) / 8) * 4 - tw->linebyte;

	return 0;
}

/* LZW圧縮データの読みだし */
static int 
read_lzwdata(u_char * p, int size, FILE * fp)
{
	int             i, j;
	int             cc;
	tiflzwdecode(p, fp);
	if (tw->type == Color16) {
		unsigned char  *q = p + ((tw->sx + 1) / 2) * size - 1;
		cc = *q--;
		p += tw->sx * size - 1;
		for (i = size - 1; i >= 0; i--) {
			for (j = tw->sx - 1; j >= 0; j--) {
				if (j % 2 == 0) {
					*p-- = cc & 0xf;
					cc = *q--;
				} else {
					*p-- = (cc >> 4) & 0xf;
				}
			}
		}
	}
	return 0;
}

/* RAWデータの読みだし */
static int 
read_rawdata(u_char * p, int size, FILE * fp)
{
	int             i, j;
	int             cc;
	if (tw->type == Color256) {
		for (i = 0; i < size; i++) {
			for (j = 0; j < tw->sx; j++) {
				*p++ = fgetc(fp);
			}
		}
	} else if (tw->type == Color16) {
		for (i = 0; i < size; i++) {
			for (j = 0; j < tw->sx; j++) {
				if (j % 2 == 0) {
					cc = fgetc(fp);
					*p++ = cc & 0xf;
				} else {
					*p++ = (cc >> 4) & 0xf;
				}
			}
		}
	}
	return 0;
}

/* 画像データの読みだし */
static int 
read_pixels(FILE * fp)
{
	unsigned long   add;
	unsigned char  *p;
	int             j;
	p = data;
	if (tw->stpnum == 1) {
		fseek(fp, tw->stpofs, 0);
		if (tw->comp != 0) {
			/* LZW data */
			if (read_lzwdata(p, tw->sy, fp))
				return -1;
		} else {
			/* RAW data */
			if (read_rawdata(p, tw->sy, fp))
				return -1;
		}
	} else {
		add = tw->stpofsofs;
		for (j = 0; j < tw->stpnum; j++, add += 4) {
			fseek(fp, add, 0);
			fseek(fp, getint(fp), 0);
			if (tw->comp != 0) {
				/* LZW data */
				if (read_lzwdata(p, tw->rowsPerStp, fp))
					return -1;
			} else {
				/* RAW data */
				if (read_rawdata(p, tw->rowsPerStp, fp))
					return -1;
			}
			p += tw->sx * tw->rowsPerStp;
		}
	}
	return 0;
}

static int
gettif(ImageData * img, FILE * fp)
{
	unsigned long   add;
	int             i;
	if (query_tif(fp))
		goto end;
	i = getshort(fp);
	add = getint(fp);
	fseek(fp, add, 0);
	if (read_tags(fp))
		goto end;

	if (tw->type == Color16 || tw->type == Color256) {
		if ((data = image_data_alloc(img, tw->sx, tw->sy, 8)) == NULL)
			return -1;
		if (read_pixels(fp))
			return -1;
		return 0;
	}
}

/* 色/マスクの設定 */
static void 
base(ImageData * img, int col0)
{
	int             i, j;
	unsigned char  *p;

	msg_out("making base data.");
	for (j = 0; j < tw->palnum; j++) {
		tw->n[j] = 256;
	}

	/* 透明色自動判定(^^;; */
	if (col0 == -1)
		col0 = p[0];

	for (i = 0; i < tw->sy; i++) {
		msg_out("\rcolor seaching line %d/%d ", i + 1, tw->sy);
		for (j = 0; j < tw->sx; j++, p++) {
#ifdef SHAPE
			if (*p == col0) {
				*q++ = *p = 0;
			} else {
				*q++ = 1;
				if (tw->n[*p] == 256)
					tw->n[*p] = image_get_col(img, tw->r[*p], tw->g[*p], tw->b[*p]);
				*p = tw->n[*p];
			}
#else
			if (tw->n[*p] == 256)
				tw->n[*p] = image_get_col(img, tw->r[*p], tw->g[*p], tw->b[*p]);
			*p = tw->n[*p];
#endif
		}
	}


	msg_out("..done.\n");
}

/* TIF ファイルのロード */
void 
load_tif(ImageData * img, FILE * fp, int col0, int rgb0)
{
	tw = (TIFWORK *) XtMalloc(sizeof(TIFWORK));
	if (gettif(fp)) {
		err_out("can't read TIF file!\n");
		exit(1);
	}
	msg_out("..done.\n");

	base(img, col0);
	XtFree((char *) tw);
}
