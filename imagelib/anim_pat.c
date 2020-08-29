#include <stdio.h>
#include <stdlib.h>
#include "animate.h"

AnimatePat *
animpat_new(void)
{
	AnimatePat *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("animpat_new");
		exit(1);
	}
	p->prev = p->next = p;	/* loop list */
	p->image = NULL;
	p->mode  = AP_NONE;
	p->src_x = p->src_y = 0;
	p->dst_x = p->dst_y = 0;
	p->clip_w = p->clip_h = 0;
	p->mask_mode = 0;
	p->loop_flag = LOOP_NONE;
	p->loop_max = 0;
	p->loop_cnt = 0;
	return p;
}

void
animpat_delete(Animate *a, AnimatePat *p)
{
	if (p) {
		if (p->image)
			realimage_delete(a, p->image);
		free(p);
	}
}

