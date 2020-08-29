static inline int
highbit(u_long mask)
{
    unsigned long hb = 1UL << (ULONG_HIGH_BITS - 1);
    int i;
    /* printf("%lx\n",hb); */
    for(i = ULONG_HIGH_BITS - 1; i >= 0; i--, mask <<= 1)
        if(mask & hb) break;
    return i;
}

XImage*
create_Pimage8(ImageData *img, XVisualInfo *vinfo)
{
	int i,j;
	unsigned w            = img->width;
	unsigned h            = img->height;
    u_long   *pixel_value = img->pixel_value;
	u_char   *data        = img->data;

	XImage *image = create_image(display,vinfo->visual,
								 w, h, ZPixmap,vinfo->depth);
	for (i=0;i<h;i++)
		for (j=0;j<w;j++)
			XPutPixel(image,j,i,pixel_value[data[i*w+j]]);

	return image;
}

XImage*
create_Timage24(pseudo_image *pi,XVisualInfo *vinfo)
{
    u_long rmask, gmask, bmask;
    int rshift, gshift, bshift;
	
    rshift = highbit(rmask = vinfo->red_mask)   - 8;
    gshift = highbit(gmask = vinfo->green_mask) - 8;
    bshift = highbit(bmask = vinfo->blue_mask)  - 8;

    if (debug) {
        printf("visual mask info\n"
			   "red mask %lu(%lx) shift %d\n"
			   "green mask %lu(%lx) shift %d\n"
			   "blue mask %lu(%lx) shift %d\n",
			   rmask,rmask,rshift,
			   gmask,gmask,gshift,
			   bmask,bmask,bshift );
    }

    const u_char *p24 = (u_char *)img->data;

    XImage *image = create_image(display,vinfo->visual,
								 img->width,img->height,ZPixmap,img->depth);
    if (image) {
        int x,y;
        u_long rv, gv, bv;

        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                rv = rshift < 0 ? p24[0] >> -rshift : p24[0] << rshift;
                gv = gshift < 0 ? p24[1] >> -gshift : p24[1] << gshift;
                bv = bshift < 0 ? p24[2] >> -bshift : p24[2] << bshift;
                XPutPixel(image, x, y, 
                    (rv & rmask)|(gv & gmask)|(bv & bmask));
                p24 += 3;
            }
        }
    }
    return image;
}
