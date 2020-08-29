/*
   Modified by Go Watanabe

   original: bmp.cc,v 1.4 1996/12/11 09:35:03 watanabe Exp watanabe
   load routines for .BMP files (MS Windows 3.x)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"

static long     bfSize;
static short    bfReserved1, bfReserved2;
static long     bfOffBits, biSize, biWidth, biHeight;
static short    biPlanes, biBitCount;
static long     biCompression, biSizeImage;
static long     biXPelsPerMeter, biYPelsPerMeter;
static long     biClrUsed, biClrImportant;

static void    *data;

/* private funcs */
static int      load_1(FILE * fp);
static int      load_4(FILE * fp);
static int      load_8(FILE * fp);
static int      load_24(FILE * fp);

static int      load_4_RGB(FILE * fp);
static int      load_8_RGB(FILE * fp);

/* データ圧縮の種類 */

#define BI_RGB  0
#define BI_RLE8 1
#define BI_RLE4 2

/* BMPファイルの種類 */

#define WIN_OS2_OLD 12
#define WIN_NEW     40
#define OS2_NEW     64

static inline unsigned
get_short(FILE * fp)
{
	unsigned        c, c1;

	c = getc(fp), c1 = getc(fp);
	return (c1 << 8) | c;
}

static inline unsigned long
get_long(FILE * fp)
{
	unsigned long   c, c1, c2, c3;

	c = getc(fp), c1 = getc(fp), c2 = getc(fp), c3 = getc(fp);
	return (c3 << 24) | (c2 << 16) | (c1 << 8) | c;
}

#define FERROR(fp) (ferror(fp) || feof(fp))

/*
 * ------------------- BMPファイルのヘッダを読み込む
 * 
 * 指定したファイルストリームがBMPファイルでなければ-1
 * を返す。
 */

static int 
query_header(ImageData *img, FILE * fp)
{
	int             i, c, bPad;

	/* read the file type (first two bytes) */

	if (getc(fp) != 'B' || getc(fp) != 'M')
		return -1;

	bfSize      = get_long(fp);
	bfReserved1 = get_short(fp);
	bfReserved2 = get_short(fp);
	bfOffBits   = get_long(fp);
	biSize      = get_long(fp);

	if (biSize == WIN_NEW || biSize == OS2_NEW) {
		biWidth         = get_long(fp);
		biHeight        = get_long(fp);
		biPlanes        = get_short(fp);
		biBitCount      = get_short(fp);
		biCompression   = get_long(fp);
		biSizeImage     = get_long(fp);
		biXPelsPerMeter = get_long(fp);
		biYPelsPerMeter = get_long(fp);
		biClrUsed       = get_long(fp);
		biClrImportant  = get_long(fp);
	} else {
		/* old bitmap */
		biWidth    = get_short(fp);
		biHeight   = get_short(fp);
		biPlanes   = get_short(fp);
		biBitCount = get_short(fp);
		biCompression = BI_RGB;
		biSizeImage   = (((biPlanes * biBitCount * biWidth) + 31) / 32)
			* 4 * biHeight;
		biXPelsPerMeter =
		biYPelsPerMeter = 0;
		biClrUsed =
		biClrImportant = 0;
	}
	/* error checking */
	if ((biBitCount != 1 && biBitCount != 4 
		 && biBitCount != 8 && biBitCount != 24)
	    || biPlanes != 1 || biCompression > BI_RLE4) {
		fprintf(stderr, "Bogus BMP File! "
				"(bitCount=%d, Planes=%d, Compression=%ld)\n",
				biBitCount, biPlanes, biCompression);
		return -1;
	}
	if (((biBitCount == 1 || biBitCount == 24) && biCompression != BI_RGB)
	    || (biBitCount == 4 && biCompression == BI_RLE8)
	    || (biBitCount == 8 && biCompression == BI_RLE4)) {
		fprintf(stderr, "Bogus BMP File! (bitCount=%d, Compression=%ld)",
				biBitCount, biCompression);
		return -1;
	}
	bPad = 0;
	if (biSize != WIN_OS2_OLD) {
		/* skip ahead to colormap, using biSize */
		/* 40 bytes read from biSize to biClrImportant */

		c = biSize - 40;
		for (i = 0; i < c; i++)
			getc(fp);

		bPad = bfOffBits - (biSize + 14);
	}
	if (biBitCount != 24) {
		int i, cmaplen;
		struct	palette *p;

		cmaplen = (biClrUsed) ? biClrUsed : 1 << biBitCount;

		if (!(p = image_alloc_palette(img,cmaplen))) {
			fprintf(stderr, "bmp:read palette failed.\n");
			return -1;
		}
		for (i = 0; i < cmaplen; i++, p++) {
			p->b = getc(fp);
			p->g = getc(fp);
			p->r = getc(fp);
			if (biSize != WIN_OS2_OLD) {
				getc(fp);
				bPad -= 4;
			}
		}
	}
	if (biSize != WIN_OS2_OLD) {
		/*
		 * Waste any unused bytes between the colour map (if present)
		 * and the start of the actual bitmap data.
		 */
		while (bPad-- > 0)
			getc(fp);
	}
	return 0;
}

/*
 * --------------- BMPファイル読み込みサブルーチン
 */

static int
load_1(FILE * fp)
{
	int             i, j, c, ct, padw;
	char           *pic8;

	/* 'padw', padded to be a multiple of 32 */
	c = 0;
	padw = (((biWidth + 31) / 32) * 32 - biWidth) / 8;

	for (i = 0; i < biHeight; i++) {
		pic8 = (char *) data + (biHeight - i - 1) * biWidth;
		for (j = ct = 0; j < biWidth; j++, ct++) {
			if ((ct & 7) == 0) {
				if (c = getc(fp), c == EOF)
					return -1;
				ct = 0;
			}
			*pic8++ = (c & 0x80) ? 1 : 0;
			c <<= 1;
		}
		for (j = 0; j < padw; j++)
			getc(fp);
	}
	return 0;
}

static int
load_4_RGB(FILE * fp)
{
	int             i, j, c, padw;
	char           *pic8;

	/* 'padw' padded to a multiple of 8pix (32 bits) */
	c = 0;
	padw = (((biWidth + 7) / 8) * 8 - biWidth) / 2;

	for (i = 0; i < biHeight; i++) {
		pic8 = (char *) data + (biHeight - i - 1) * biWidth;

		for (j = 0; j < biWidth; j++) {
			if ((j & 1) == 1)
				*pic8++ = (c & 0x0f);
			else {
				if (c = getc(fp), c == EOF)
					return -1;
				*pic8++ = (c & 0xf0) >> 4;
			}
		}
		for (j = 0; j < padw; j++)
			getc(fp);
	}
	return 0;
}

static int
load_4(FILE * fp)
{
	int             i, c, c1, x, y;
	char           *pic8;

	switch (biCompression) {
	case BI_RGB:
		/* read uncompressed data */
		return load_4_RGB(fp);
	case BI_RLE4:
		/* read RLE4 compressed data */
		break;
	default:
		fprintf(stderr, "unknown BMP compression type 0x%0lx\n",
			biCompression);
		return -1;
	}
	c = c1 = 0;
	x = y = 0;
	pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;

	while (y < biHeight) {
		if (c = getc(fp), c == EOF)
			return -1;
		if (c) {	/* encoded mode */
			c1 = getc(fp);
			for (i = 0; i < c; i++, x++)
				*pic8++ = (i & 1) ? (c1 & 0x0f) : ((c1 >> 4) & 0x0f);
		} else {	/* c==0x00  :  escape codes */
			if (c = getc(fp), c == EOF)
				return -1;

			if (c == 0x00) {	/* end of line */
				x = 0;
				y++;
				pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;
			} else if (c == 0x01)
				break;	/* end of pic8 */
			else if (c == 0x02) {	/* delta */
				c = getc(fp);
				x += c;
				c = getc(fp);
				y += c;
				pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;
			} else {/* absolute mode */
				for (i = 0; i < c; i++, x++) {
					if ((i & 1) == 1)
						*pic8++ = (c1 & 0x0f);
					else {
						c1 = getc(fp);
						*pic8++ = (c1 & 0xf0) >> 4;
					}
				}
				/* read pad byte */
				if (((c & 3) == 1) || ((c & 3) == 2))
					getc(fp);
			}
		}		/* escape processing */
		if (FERROR(fp))
			return -1;
	}			/* while */
	return 0;
}

static int
load_8_RGB(FILE * fp)
{
	int             i, j, c, padw;
	char           *pic8;

	/* 'padw' padded to a multiple of 4pix (32 bits) */
	padw = ((biWidth + 3) / 4) * 4 - biWidth;

	for (i = 0; i < biHeight; i++) {
		pic8 = (char *) data + (biHeight - i - 1) * biWidth;
		for (j = 0; j < biWidth; j++) {
			if (c = getc(fp), c == EOF)
				return -1;
			*pic8++ = c;
		}
		for (j = 0; j < padw; j++)
			getc(fp);
	}
	return 0;
}

static int
load_8(FILE * fp)
{
	int             i, c, c1, x, y;
	char           *pic8;

	switch (biCompression) {
	case BI_RGB:
		/* read uncompressed data */
		return load_8_RGB(fp);
	case BI_RLE8:
		/* read RLE8 compressed data */
		break;
	default:
		fprintf(stderr, "unknown BMP compression type 0x%0lx\n"
			,biCompression);
		return -1;
	}
	x = y = 0;
	pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;

	while (y < biHeight) {
		if (c = getc(fp), c == EOF)
			return -1;
		if (c) {	/* encoded mode */
			c1 = getc(fp);
			for (i = 0; i < c; i++, x++)
				*pic8++ = c1;
		} else {	/* c==0x00  :  escape codes */
			if (c = getc(fp), c == EOF)
				return -1;
			if (c == 0x00) {	/* end of line */
				x = 0;
				y++;
				pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;
			} else if (c == 0x01)
				break;	/* end of pic8 */
			else if (c == 0x02) {	/* delta */
				c = getc(fp);
				x += c;
				c = getc(fp);
				y += c;
				pic8 = (char *) data + x + (biHeight - y - 1) * biWidth;
			} else {/* absolute mode */
				for (i = 0; i < c; i++, x++)
					*pic8++ = getc(fp);
				if (c & 1) {
					/*
					 * odd length run: read an extra pad
					 * byte
					 */
					getc(fp);
				}
			}
		}		/* escape processing */

		if (FERROR(fp))
			return -1;

	}			/* while */

	return 0;
}

static int
load_24(FILE * fp)
{
	int             i, j, pad;
	char           *pic24;

	/* # of pad bytes to read at EOscanline */
	pad = (4 - ((biWidth * 3) % 4)) & 0x03;

	for (i = 0; i < biHeight; i++) {
		pic24 = (char *) data + (biHeight - i - 1) * biWidth * 3;

		for (j = 0; j < biWidth; j++) {
			pic24[2] = getc(fp);	/* blue */
			pic24[1] = getc(fp);	/* green */
			pic24[0] = getc(fp);	/* red */
			pic24 += 3;
		}
		for (j = 0; j < pad; j++)
			getc(fp);
	}
	return 0;
}

int
bmp_read_stream(ImageData *img, FILE * fp)
{
	int ret;

	if (query_header(img,fp) < 0)
		return -1;

	switch (biBitCount) {
	case 1:
	case 4:
	case 8:
		if (!(data = image_alloc_data(img, biWidth, biHeight, 8)))
			return -1;
		break;
	case 24:
		if (!(data = image_alloc_data(img, biWidth, biHeight, 24)))
			return -1;
		break;
	}

	switch (biBitCount) {
	case 1:
		ret = load_1(fp);
		break;
	case 4:
		ret = load_4(fp);
		break;
	case 8:
		ret = load_8(fp);
		break;
	case 24:
		ret = load_24(fp);
		break;
	}

	switch (biBitCount) {
	case 1:
	case 4:
	case 8:
		if (img->trans_flag == TRANS_AUTO) {
			img->trans_flag = TRANS_INDEX;
			img->trans_index = ((u_char*)img->data)[0];
		}
		break;
	case 24:
		if (img->trans_flag == TRANS_AUTO) {
			img->trans_flag = TRANS_RGB;
			img->trans_rgb =
				 (((u_char*)img->data)[0] << 16)
				|(((u_char*)img->data)[1] << 8)
				| ((u_char*)img->data)[2];
		}
	}

	return ret;
}

