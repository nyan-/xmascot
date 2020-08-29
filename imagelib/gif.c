/*
    Modified by Go Watanabe.

	- GIF loading code.

	watanabe@papilio.tutics.tut.ac.jp

   This source code based strongly on...

   gif2ras.c - Converts from a Compuserve GIF(tm) image to a Sun Raster image.

   Copyright (c) 1988, 1989 by Patrick J. Naughton

   Author: Patrick J. Naughton
   naughton@wind.sun.com

   Permission to use, copy, modify, and distribute this software and its
   documentation for any purpose and without fee is hereby granted,
   provided that the above copyright notice appear in all copies and that
   both that copyright notice and this permission notice appear in
   supporting documentation.

   This file is provided AS IS with no warranties of any kind.  The author
   shall have no liability with respect to the infringement of copyrights,
   trade secrets or any patents by this file or any part thereof.  In no
   event will the author be liable for any lost revenue or profits or
   other special, indirect and consequential damages.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "gif.h"

#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif

extern int verbose;

static int EGA_palette[16][3] = {
	{0,0,0},       {0,0,128},     {0,128,0},     {0,128,128}, 
	{128,0,0},     {128,0,128},   {128,128,0},   {200,200,200},
	{100,100,100}, {100,100,255}, {100,255,100}, {100,255,255},
	{255,100,100}, {255,100,255}, {255,255,100}, {255,255,255},
};

/* work varable */
static u_char *data;
static u_char *pic8,*raster;
static int xc, yc, pass;
static int bit_offset,bit_buf;
static int prefix[4096],suffix[4096];  /* The hash table */
static int out_code[4097]; /* An output array */
static struct palette global_pal[256];	/* global_palette */

static inline unsigned
get_short(FILE *fp)
{
	unsigned c,c1;
	c = getc(fp),c1 = getc(fp);
	return c|(c1 << 8);
}

static void 
aspect_extension(GifInfo *gi, FILE *fp)
{
	int sbsize,blocksize,aspect,aspden;

	blocksize = getc(fp);
	if (blocksize != 2) 
		fseek(fp,blocksize,SEEK_CUR);
	else {
		aspect = getc(fp);
		aspden = getc(fp);
		if (aspden > 0 && aspect > 0)
			gi->normal_aspect = (float) aspect / aspden;
		else {
			gi->normal_aspect = 1.0;
			aspect = aspden = 1;
		}
		if (verbose) {
			fprintf(stderr,"GIF87 aspect: %d:%d = %f\n",
					aspect, aspden,gi->normal_aspect);
		}
	}
	while (sbsize = getc(fp),sbsize > 0)
		fseek(fp,sbsize,SEEK_CUR);
}

static void 
plain_text_extension(GifInfo *gi, FILE *fp)
{
	int i,sbsize,ch;
	int tgLeft,tgTop,tgWidth,tgHeight,cWidth,cHeight,fg,bg;

	sbsize   = getc(fp);
	tgLeft   = get_short(fp);
	tgTop    = get_short(fp); 
	tgWidth  = get_short(fp); 
	tgHeight = get_short(fp); 
	cWidth   = getc(fp);
	cHeight  = getc(fp);
	fg       = getc(fp);
	bg       = getc(fp);
	fseek(fp,sbsize - 12,SEEK_CUR);

	if (verbose)
		fprintf(stderr,"PlainText\ntgrid=%d,%d %dx%d "
			"cell=%dx%d col=%d,%d\n",
			tgLeft,tgTop,tgWidth,tgHeight,cWidth,cHeight,fg,bg);

	/* read (and ignore) data sub-blocks */
	while (sbsize = getc(fp), sbsize >0) {
		if (verbose) {
			for (i = 0; i < sbsize; i++) {
				ch = getc(fp);
				putc(ch,stderr);
			}
		} else
			fseek(fp,sbsize,SEEK_CUR);
	}
	if (verbose)
		putc('\n',stderr);
}

static void
graphic_ctl_extension(GifInfo *gi, FILE *fp)
{
	int sbsize;
	u_char flag;
	u_char transparent;
	
	sbsize      = getc(fp);		/* must 4 */

	if (sbsize != 4){
		fseek(fp,sbsize,SEEK_CUR);
		goto end;
	}

	flag        = getc(fp);
	gi->delay_time  = get_short(fp);
	transparent = getc(fp);

	if (verbose)
		fprintf(stderr,"Graphic Control Extension: %x %d %x \n",
				flag, gi->delay_time, transparent );

	if (flag & 0x01) 	/* transparent */
		gi->transparent_index = transparent;

	gi->disposal_method = (flag >> 3) & 0x07;
	gi->user_input_flag = (flag & 0x40)?1:0;


 end:
	while (sbsize = getc(fp),sbsize > 0)
		fseek(fp,sbsize,SEEK_CUR);
}

static void
application_extension(GifInfo *gi, FILE *fp)
{
	int sbsize;
	char buf[20];
	sbsize = getc(fp);

	if (sbsize == 11) {
		fread(buf, 11, 1, fp);
		sbsize = getc(fp);
	    if (!strncmp(buf,"NETSCAPE",8) || !strncmp(buf,"netscape",8)) {
			if (sbsize == 3) {
				getc(fp);
				gi->loop_cnt = get_short(fp);
				sbsize = getc(fp);
				gi->loop_flag = 1;
				if (verbose)
					fprintf(stderr, "Netscape Loop Extension: %d\n",
							gi->loop_cnt);
			}	
		}
	}

	while (sbsize > 0) {
		fseek(fp,sbsize,SEEK_CUR);
		sbsize = getc(fp);
	}
}

static void
ignore_extension(FILE *fp)
{
	int sbsize;
	while (sbsize = getc(fp),sbsize > 0)
		fseek(fp,sbsize,SEEK_CUR);
}

static int 
readCode(int n)
{
    int x = bit_buf;

    while(n > bit_offset)
        x |= *raster++ << bit_offset, bit_offset += 8;

    bit_buf = x >> n;
    bit_offset -= n;

    return x;
}

/*
	Fetch the next code from the raster data stream.  The codes can be
	any length from 3 to 12 bits, packed into 8-bit bytes, so we have to
	maintain our location in the Raster array as a BIT Offset.  We compute
	the byte Offset into the raster array by dividing this by 8, pick up
	three bytes, compute the bit Offset into our 24-bit chunk, shift to
	bring the desired code to the bottom, then mask it off and return it. 
*/

static void
doInterlace(GifInfo *gi, int *outcode,int n)
{
	static int yc_inc[] = { 8,8,4 },
	           yc_next[] = { 4,2,1, },
	           pic_inc[] = { 7,7,3, };
 retry:
	switch(pass) {
	case 0:
	case 1:
	case 2:
		while(n--) {
			*pic8++ = *outcode--;
			if (++xc == gi->Width) {
				xc = 0;
				if ((yc += yc_inc[pass]) >= gi->Height) {
					yc = yc_next[pass];
					pic8 = (u_char *)data + yc * gi->Width;
					pass++;
					goto retry;
				}
				pic8 += pic_inc[pass] * gi->Width;
			}
		}
		break;
	case 3:
		while(n--) {
			*pic8++ = *outcode--;
			if (++xc == gi->Width) {
				xc = 0;
				yc += 2;
				pic8 += gi->Width;
			}
		}
		break;
    }
}

static int
decode_image(GifInfo *gi, ImageData *img, FILE *fp) 
{
	u_char *work;
	int i,npixels,maxpixels,size;
	int colormap_flag,interlace;
    int code,current_code,old_code,in_code;
	int code_size;

	int clear_code,EOF_code,free_code,first_free;
	int init_code_size,max_code,FinChar;
	int out_count;
    int read_mask;	/* Code AND mask for current code size */

	/* copy from global palette */
	struct palette *p = image_alloc_palette(img,256);
	for (i = 0; i < 256; i++)
		*p++ = global_pal[i];

	gi->LeftOfs = get_short(fp);
	gi->TopOfs  = get_short(fp);
	gi->Width   = get_short(fp);
	gi->Height  = get_short(fp);

	if (!(data = image_alloc_data(img,gi->Width,gi->Height,8)))
		return -1;

	colormap_flag = getc(fp);
	interlace     = (colormap_flag & INTERLACEMASK);

	if (verbose)
		fprintf(stderr,"GIF Image:local image (%dx%d)\n",gi->Width,gi->Height);

	/* read local colormap */
	if (colormap_flag & 0x80) {
		int colormap_size = 1 << ((colormap_flag & 7)+1);
		struct palette *p = img->pal;

		for (i = 0; i < colormap_size; i++,p++) {
			p->r = getc(fp);
			p->g = getc(fp);
			p->b = getc(fp);
		}
	}

	bit_offset = bit_buf = 0;
	code_size  = getc(fp);
	clear_code = (1 << code_size);
	EOF_code   = clear_code + 1;
	first_free = 
	free_code  = clear_code + 2;

	/* The GIF spec has it that the code size is the code size used to
	  compute the above values is the code size given in the file, but the
	  code size used in compression/decompression is the code size given in
	  the file plus one. (thus the ++).
	*/

   	init_code_size = ++code_size;
  	max_code       = (1 << code_size);
  	read_mask      = max_code - 1;

	if (verbose) {
		fprintf(stderr,"codesize %d init codesize %d\n",
				code_size,init_code_size);
	}

/* UNBLOCK:
	Read the raster data.  Here we just transpose it from the GIF array
	to the Raster array, turning it from a series of blocks into one long
	data stream, which makes life much easier for readCode().
*/

	work = (u_char *)malloc(maxpixels = gi->Width * gi->Height);
	if(work == NULL) {
		perror("malloc");
		fprintf(stderr,"GIF Image load failed.\n");
		return -1;
	}

	raster = work;
	while(size = getc(fp),size >0) {
		int read_size;
		if (raster + size > work + maxpixels) {
			char buf[256];

			read_size = fread(buf,1,size,fp);
            memcpy(raster,buf,maxpixels - (raster - work));
            raster += read_size;
            while(size = getc(fp),size > 0) {
                read_size = fread(buf,1,size,fp);
                raster += read_size;
            } 
            fprintf(stderr,"Error:Illegal GIF file size\n");
            break;
		}
		read_size = fread(raster,1,size,fp);
		raster += read_size;
	}

	raster = work;

/*	
	Decompress the file, continuing until you see the GIF EOF code.
   	One obvious enhancement is to add checking for corrupt files here.
*/

	npixels = out_count = 0;
	pic8    = (u_char *)data;
	xc = yc = pass = 0;
	old_code = code = readCode(code_size) & read_mask;
	FinChar = code & gi->bit_mask;

	while(code != EOF_code) {
		if(code == clear_code) {
	/* 
		Clear code sets everything back to its initial value, 
		then reads the immediately subsequent code as uncompressed data. 
	*/
			code_size = init_code_size;
			max_code = (1 << code_size);
			read_mask = max_code - 1;
			free_code = first_free;
			current_code = old_code = code = readCode(code_size)&read_mask;
			FinChar = current_code & gi->bit_mask;

			if(interlace)
				doInterlace(gi, &FinChar,1);
			else
				*pic8++ = FinChar;
	
			code = readCode(code_size) & read_mask;
			if(++npixels >= maxpixels)
				break;
			else
				continue;
		}
	/*	If not a clear code, must be data: 
			save same as current_code and in_code */

	/*	If we're at maxcode and didn't get a clear, stop loading */
		if(free_code >= 4096) 
			break;
		current_code = in_code = code;

	/* If greater or equal to free_code, not in the hash table yet;
		repeat the last character decoded */

		if(current_code >= free_code) {
			current_code = old_code;
			if(out_count > 4096) break;
			out_code[out_count++] = FinChar;
		}
     
	/* Unless this code is raw data, pursue the chain pointed to 
		by current_code through the hash table to its end;
		each code in the chain puts its associated output code 
		on the output queue. */

		while(current_code > gi->bit_mask) {
			if(out_count > 4096) break;  /* corrupt file */
			out_code[out_count++] = suffix[current_code];
			current_code = prefix[current_code];
		}
		if(out_count > 4096) break;

      /* The last code in the chain is treated as raw data. */

		FinChar = current_code & gi->bit_mask;
		out_code[out_count++] = FinChar;

      /* Now we put the data out to the Output routine.
         It's been stacked LIFO, so deal with it that way... */

      /* safety thing:  prevent exceeding range of 'pic8' */

		if(npixels + out_count > maxpixels)
			out_count = maxpixels - npixels;
		if(interlace)
			doInterlace(gi, out_code+out_count-1,out_count);
		else {
			for(i=out_count-1;i>=0;i--)
				*pic8++ = out_code[i];
		}
		npixels += out_count;
		out_count = 0;

	/* Build the hash table on-the-fly. No table is stored in the file. */

   		prefix[free_code] = old_code;
   		suffix[free_code] = FinChar;
   		old_code = in_code;

    /* 
    	Point to the next slot in the table.  If we exceed the current
		max_code value, increment the code size unless it's already 12.
		If it is, do nothing: the next code decompressed better be CLEAR
	*/
		if (++free_code >= max_code) {
			if(code_size < 12) {
				code_size++;
				max_code <<= 1;
				read_mask = (1 << code_size) - 1;
			}
		}
		code = readCode(code_size) & read_mask;
		if (npixels >= maxpixels) break;
	}
	if (npixels != maxpixels) {
		if(verbose)
			fprintf(stderr,"This GIF file seems to be truncated. "
					"Winging it.\n");
	    if(!interlace)  {
			/* clear->EOBuffer */
			memset(pic8 + npixels,0, maxpixels - npixels);
		}
	}
	free(work);

	return 0;
}

int
gif_query_header(GifInfo *gi, FILE *fp)
{
	int i,aspect;
	int ColorMapSize;
	char Type[6];

	int is89;
	int Graphics_flag;
	
	if(fread(Type,1,6,fp) != 6) {
		perror("fread");
		return -1;
	}

	if(strncmp(Type,"GIF87a",6) == 0)
		is89 = 0;
	else if(strncmp(Type,"GIF89a",6) == 0)
		is89 = 1;
	else return -1;

	gi->GlobalWidth  = get_short(fp);
	gi->GlobalHeight = get_short(fp);
	Graphics_flag = getc(fp);
	gi->background_index = getc(fp);

	ColorMapSize = 1 << ((Graphics_flag & 7)+1);
	gi->bit_mask = ColorMapSize - 1;

	gi->normal_aspect = 1.0f;
	aspect = getc(fp);
	if(aspect) {
	    if(is89) return -1;
		gi->normal_aspect = (aspect + 15) / 64.0;
    }

	if (verbose)
		fprintf(stderr,"debug:try to read gif %dx%d.\n",
				gi->GlobalWidth,gi->GlobalHeight);

	/* Read in global colormap. */
	{
		struct palette *p = global_pal;

		if (Graphics_flag  & COLORMAPMASK) {
			if (verbose)
				fprintf(stderr,"\tglobal pallette in colormap (%d).\n",
						ColorMapSize);
			for (i = 0; i < ColorMapSize; i++,p++) {
				p->r = getc(fp);
				p->g = getc(fp);
				p->b = getc(fp);
			}
		} else {
			if(verbose)
				fprintf(stderr,"\tuse EGA pallette.\n");
			for(i = 0; i < 256; i++) {
				p->r = EGA_palette[i&15][0];
				p->g = EGA_palette[i&15][1];
				p->b = EGA_palette[i&15][2];
			}
			gi->background_index = -1;
		}
	}
	return 0;
}

int
gif_read_stream_one(GifInfo *gi, ImageData *img, FILE *fp)
{

/*	---------------------------------------------------------
	possible things at this point are:
	an application extension block
	a comment extension block
	an (optional) graphic control extension block
	followed by either an image or a plaintext extension
*/
	for (;;) {
		int block = getc(fp);

		if (feof(fp))
			break;

		if(verbose)
			fprintf(stderr,"block type 0x%02x:",block);

		if (block == EXTENSION) {  /* parse extension blocks */
			int fn = getc(fp);

			if(verbose)
				fprintf(stderr," GIF extension type 0x%02x.\n", fn);
			switch(fn) {
			case 'R':	/* GIF87 aspect extension */
				aspect_extension(gi, fp);
				break;
			case 0xFE:	/* Comment Extension */
				ignore_extension(fp);
				break;
			case 0x01:	/* PlainText Extension */
				plain_text_extension(gi, fp);
				break;
			case 0xF9:	/* Graphic Control Extension */
				graphic_ctl_extension(gi, fp);
				break;
			case 0xFF:	/* Application Extension */
				application_extension(gi, fp);
				break;
			default:	/* unknown extension */
				if(verbose)
					fprintf(stderr,
						"GIF Image:unknown extension 0x%02x(ignored)\n", fn);
				ignore_extension(fp);
			}
		} else if(block == IMAGESEP) {
			if(verbose)
				fprintf(stderr,"data chunk. now reading image..\n");
			return decode_image(gi, img, fp);
		} else if(block == TRAILER) {      /* stop reading blocks */
			if(verbose)
				fprintf(stderr,"trailer\n");
			break;
		} else {	/* unknown block type */
			fprintf(stderr,"GIF Image:Unknown block type (0x%02x)"
					"at %lx(ignored)\n", block,ftell(fp));
			ignore_extension(fp);
		}
	}
	return -1;
}

int
gif_read_stream(ImageData *img, FILE *fp)
{
	GifInfo gi;
	
	if (gif_query_header(&gi, fp) < 0)
		return -1;
	gi.transparent_index = -1;

	if (gif_read_stream_one(&gi, img, fp) < 0)
		return -1;

	if (img->trans_flag == TRANS_AUTO && gi.transparent_index != -1) {
		img->trans_flag = TRANS_INDEX;
		img->trans_index = gi.transparent_index;
	}

	return 0;
}
