#include <stdio.h>
#include <stdlib.h>
#include "animate.h"
#include "gif.h"

extern int verbose;

int
anim_gif_read_stream(Animate *a, FILE *fp)
{
	GifInfo gi;
	ImageData *img;
	AnimatePat *ap   = NULL;
	AnimatePat *loop = NULL;
	RealImage *rimage;

	gi.background_index  = -1;
	if (gif_query_header(&gi, fp) < 0)
		return -1;
	
	if (verbose)
		fprintf(stderr, "bg_index: %d\n", gi.background_index);
	
	for (;;) {
		img = image_new();
		gi.transparent_index = -1;
		gi.disposal_method   = 0;
		gi.user_input_flag   = 0;
		gi.delay_time        = 0;
		gi.loop_flag         = 0;
		
		if (gif_read_stream_one(&gi, img, fp) < 0) {
			image_delete(img);
			break;
		}

		if (img->trans_flag == TRANS_AUTO && gi.transparent_index != -1) {
			img->trans_flag = TRANS_INDEX;
			img->trans_index = gi.transparent_index;
		}

		rimage = realimage_new_image(img);
		image_delete(img);

		ap = animpat_new();
		ap->image    = rimage;
		ap->interval = gi.delay_time * 10; 
		ap->dst_x  = gi.LeftOfs;
		ap->dst_y  = gi.TopOfs;
		ap->clip_w = gi.Width;
		ap->clip_h = gi.Height;
		ap->mode   = AP_NORMAL;

		animate_add_pattern(a, ap);

		if (loop == NULL && gi.loop_flag) {
			loop = ap;
		}
	}

	if (loop) {
		ap->loop = loop;
		if (gi.loop_cnt == 0)
			ap->loop_flag = LOOP_FOREVER;
		else {
			ap->loop_flag = LOOP_COUNT;
			ap->loop_max  = gi.loop_cnt;
		}
	}

	animate_getsize(a);
	return 0;
}
