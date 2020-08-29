/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

/* pnm ファイルのロード */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "image.h"

int image_get_col(ImageData *img,int r, int g, int b)
{
   return 0;
}

typedef struct{
	unsigned char *data;
	int w,h;
	int format;
} PNM;

/* pnm ファイルの読み込み */
static PNM getpnm(FILE *fp)
{
	PNM ret = { NULL, 0, 0 ,0};
	char tmp[100];
	int i,x,y,z;
	char ch;
	int size;

 	if( fgetc(fp)!='P')
		goto endfunc;
 	ret.format = fgetc(fp) - '0';
 	if( ret.format != 4 && ret.format != 5 && ret.format != 6 )
		goto endfunc;

	do {
		ch = fgetc(fp);
	}while(!(isdigit(ch)||(ch == '#')));

	ungetc(ch,fp);
	i = (ret.format == 4) ? 1 : 0;
	while( i<3 ){
		fgets(tmp,99,fp);
		ch = '\0';
		sscanf(tmp,"%c",&ch);
		if( ch !='#' ){
			ch = 0;
			switch(i){
			case 0:
				ch = sscanf(tmp,"%d%d%d",&x,&y,&z);
				break;
			case 1:
				ch = sscanf(tmp,"%d%d",&y,&z);
				break;
			case 2:
				ch = sscanf(tmp,"%d",&z);
			}
			i += ch;
		}
	}
	if(ret.format == 4 ) {
		x = y;
		y = z;
	}

	ret.data = (unsigned char*)XtMalloc(x*y*3);
	ret.w    = x;
	ret.h    = y;
	size = (ret.format == 4)? ((x+7)/8)*y : x*y;
	if( fread((char *)ret.data,(ret.format == 6)?3:1,size, fp) != size ){
		XtFree( (char*)(ret.data) );
		ret.data = NULL;
	}

 endfunc:
	fclose(fp);
	return ret;
}

/* pnm -> ImageData */
static void pnm2image(ImageData *img, PNM *pnm, int rgb0)
{
	int i,j;

	unsigned w = pnm->w;
	unsigned h = pnm->h;
	u_char   *p;
	int r,g,b;
	int q;
#ifdef SHAPE
	int r0,g0,b0;
#endif

	img->w    = w;
	img->h    = h;
	img->data = (u_char*)XtMalloc( w*h );
	p = pnm->data;
	q = 0;

#ifdef SHAPE
	/* 透明色自動判定(^^;; */
    if (rgb0 == -1){
		switch (pnm->format) {
		case 4:
			r0 = g0 = b0 = (*p&0x80)?255:0;
			break;
		case 5:
			r0 = g0 = b0 = *p;
			break;
		default:
			r0 = p[0];
			g0 = p[1];
			b0 = p[2];
			break;
		}
	}else{
		r0 = (rgb0 >> 16) & 0xff;
		g0 = (rgb0 >> 8 ) & 0xff;
		b0 = (rgb0      ) & 0xff;
	}

	img->mask = (unsigned char*)XtMalloc( w*h );
	msg_out( "transient color: R:%02x G:%02x B:%02x\n", r0, g0, b0 );
#endif

	msg_out("making mascot data.\n");
	msg_out("PNM format type[%d]\n",pnm->format);
	
	if(pnm->format == 4) {
		for(i=0;i<h;i++){
			msg_out("\rcolor seaching line %d/%d ",i+1,h);
			for(j=0;j<w;j++,q++){
				r = (*p&0x80)?255:0;
#ifdef SHAPE
				img->mask[q] = (r==r0 && r==g0 && r==b0)?0:1;
#endif
				img->data[q] = image_get_col(img,r,r,r);
				if(j%8 == 7 || j == w-1)
					p++;
				else
					*p <<= 1;
			}
		}
	} else if(pnm->format == 5) {
		for(i=0;i<h;i++){
			msg_out("\rcolor seaching line %d/%d ",i+1,h);
			for(j=0;j<w;j++,q++){
				r = *p++;
#ifdef SHAPE
				img->mask[q] = (r==r0 && r==g0 && r==b0)?0:1;
#endif
				img->data[q] = image_get_col(img,r,r,r);

			}
		}
	} else {
		for(i=0;i<h;i++){
			msg_out("\rcolor seaching line %d/%d ",i+1,h);
			for(j=0;j<w;j++,q++){
				r = *p++;
				g = *p++;
				b = *p++;
#ifdef SHAPE
				img->mask[q] = (r==r0 && g==g0 && b==b0)?0:1;
#endif
				img->data[q] = image_get_col(img,r,g,b);
			}
		}
	}
	msg_out("..done.\n");
}

/* pnm 形式のロード */
ImageData load_pnm(ImageData *img, FILE *fp, int col0, int rgb0)
{
	PNM pnm;
	pnm = getpnm(fp);
	pnm2image(img, &pnm, rgb0);
	XtFree((char*)(pnm.data));
}

