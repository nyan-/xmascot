#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>






#ifdef SHAPE
	if (m) {
		if (shadow_len) {
			for(i=h+shadow_len-1;i>=0;i--)
				for(j=w+shadow_len-1;j>=0;j--)
					if (i<h && j<w && data[i*w+j] != trans_index) {
						XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
						XPutPixel(img_mask,j,i,1);
						if ((i+j)& 1)
							XPutPixel(img_mask,j+shadow_len,i+shadow_len,1);
					} else {
						XPutPixel(img_data,j,i,black);
						XPutPixel(img_mask,j,i,0);
					}
		} else {
			for (i=0;i<h;i++)
				for (j=0;j<w;j++)
					if (data[i*w+j] != trans_index) {
						XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
						XPutPixel(img_mask,j,i,1);
					}else{
						XPutPixel(img_mask,j,i,0);
					}
		}
	}else
#endif /* SHAPE */
	{
		for(i=0;i<h;i++)
			for(j=0;j<w;j++)
				XPutPixel(img_data,j,i,pixel_value[data[i*w+j]]);
	}

