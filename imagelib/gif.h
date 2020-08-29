#ifndef __GIF_H_
#define __GIF_H_

#define EXTENSION     0x21
#define IMAGESEP      0x2c
#define TRAILER       0x3b
#define INTERLACEMASK 0x40
#define COLORMAPMASK  0x80

typedef struct GifInfo {
	unsigned GlobalWidth, GlobalHeight;
	unsigned LeftOfs,TopOfs,Width,Height;
	float normal_aspect;
	int bit_mask;   			/* AND mask for data size */
	int background_index;

/* animated GIF 89a */
	int transparent_index;
	int disposal_method;
	int user_input_flag;
	int delay_time;

/* netscape extension */
	int loop_flag;
	int loop_cnt;
} GifInfo;

int gif_query_header(GifInfo *gi, FILE *fp);
int gif_read_stream_one(GifInfo *gi, ImageData *img, FILE *fp);

int gif_read_stream(ImageData *img, FILE *fp);

#endif
