%{
#include <stdio.h>
#include <string.h>
#include "namelist.h"
#include "animate.h"

static void entry_label(char *name);
static void start_anim(char *name);
static void end_anim(char *name);
static void entry_animpat(char *name, char *time);
static void set_loop(char *name, char *count);
static void set_from(char *xpos, char *ypos);
static void set_size(char *width, char *height);
static void set_to(char *xpos, char *ypos);
static void set_mask(char *num);
%}

%union {
	char *string;
}
%token START END
%token FROM SIZE TO MASK LOOP
%token <string> NUMBER
%token <string> QSTRING
%token <string> ID
%token <string> LABEL

%type <string> file
%type <string> start

%%

animation: start animation_contents END  { end_anim($1); }
	;

start: ID START { $$ = $1; start_anim($1); };

animation_contents: animpats
    | animation_contents animpats
	;

label: LABEL animpat { entry_label($1); }
	;	

file:	ID		
	| QSTRING
	| NUMBER
	;

loop: LOOP ID NUMBER { set_loop($2, $3); }
	;

from: FROM NUMBER NUMBER { set_from($2,$3); }
	;

size: SIZE NUMBER NUMBER { set_size($2,$3); }
	;

to:   TO NUMBER NUMBER	 { set_to($2,$3); }
	;

mask: MASK NUMBER { set_mask($2); }
     ;

patoption: from
	| size
	| to
	| mask
    | loop 
	;

patoptions: 
	| patoptions patoption
	;

animpat: file NUMBER  patoptions { entry_animpat($1,$2); }
	;

animpats:  animpat | label ;

%%

static NameList label_list;
static NameList name_list;
static Animate        *next_anm;
static AnimatePat     *next_ap;
static AnimatePat     *prev_ap;

static void
init(void)
{
	name_list  = namelist_new();
}

static void
entry_label(char *name)
{
	NameListRec *p = namelist_new();
	p->name = name;
	p->data = prev_ap;
	namelist_add(label_list, p);
}

static void
start_anim(char *name)
{
	label_list = namelist_new();
	prev_ap  = NULL;
	next_ap  = animpat_new();
}

static void
end_anim(char *name)
{
	animpat_delete(NULL,next_ap);
	namelist_delete_list(label_list);
}

static void
entry_animpat(char *name, char *time)
{
	RealImage *rimage;
	if ((rimage = (RealImage*)namelist_find(name_list, name)) == NULL) {
		ImageData *image = image_new();
		image_load(image, name, -1, -1);
		if (image->data == NULL) {		/* ロードに失敗 */
			rimage = realimage_new_plane(name);
		} else {
			rimage = realimage_new_image(image);
		}
		image_delete(image);
	}
	{
		NameListRec *p = namelist_new();
		p->name = name; /* no need to free */
		p->data = (void*)rimage;
		namelist_add(name_list, p);
	}
	next_ap->image = rimage;
	next_ap->interval = atoi(time);
	
	if (next_ap->mode == AP_NONE)
		next_ap->mode = AP_NORMAL;

	free(time);
	animate_add_pattern(next_anm, next_ap);
	prev_ap = next_ap;
	next_ap = animpat_new();
#if DEBUG
	fprintf(stderr,"entry_animpat: %s [%p]\n", name, prev_ap );
#endif

}

static void
set_loop(char *name, char *count)
{
	AnimatePat *p;
	int cnt = atoi(count);

	if ((p = (AnimatePat*)namelist_find(label_list, name)) == NULL) {
		fprintf(stderr, "can't find label [%s]. ignored.", name);
		goto end;
	}

#ifdef DEBUG
	fprintf(stderr,"%s: %p\n",name,p);
#endif

	next_ap->loop = p;
	if (count <= 0) 
		next_ap->loop_flag = LOOP_FOREVER;
	else {
		next_ap->loop_flag = LOOP_COUNT;
		next_ap->loop_max = cnt;
	}
#ifdef DEBUG
	fprintf(stderr,"loop to [%p] mode [%d] cnt[%d]\n",
			prev_ap->loop, prev_ap->loop_flag, prev_ap->loop_max);
#endif
 end:
	free(name);
	free(count);
}

static void
set_from(char *xpos, char *ypos)
{
	int x = atoi(xpos);
	int y = atoi(ypos);
	free(xpos);
	free(ypos);
	next_ap->src_x = x;
	next_ap->src_y = y;
	if (next_ap->mode == AP_NONE)
		next_ap->mode = AP_DIFF;
}

static void
set_size(char *width, char *height)
{
	int w = atoi(width);
	int h = atoi(height);
	free(width);
	free(height);
	next_ap->clip_w = w;
	next_ap->clip_h = h;
	if (next_ap->mode == AP_NONE)
		next_ap->mode = AP_DIFF;
}

static void
set_to(char *xpos, char *ypos)
{
	int x = atoi(xpos);
	int y = atoi(ypos);
	free(xpos);
	free(ypos);
	next_ap->dst_x = x;
	next_ap->dst_y = y;
	if (next_ap->mode == AP_NONE)
		next_ap->mode = AP_DIFF;
}

static void
set_mask(char *num)
{
	int n = atoi(num);
	free(num);
	if (n > 0 && n < 5) {
		next_ap->mode = AP_MASKED;
		next_ap->mask_mode = n;
		next_ap->mask_cnt  = 0;
	}else
		fprintf(stderr, "invalid mask: %d\n", n);
}

int anim_lineno = 0;
static char *anim_filename = NULL;

extern FILE *animin;
extern char *animtext;

int
animate_parse(Animate *a, FILE *fp)
{
	int ret;
	animin = fp;
	next_anm = a;
	ret = animparse();
	if (!ret)
		animate_getsize(a);
	return ret;
}

FILE *
animate_fopen(const char *name)
{
	static int inited = 0;
	if (!inited)
		init();
	anim_filename = strdup(name);
	anim_lineno = 0;
	return fopen(name, "r");
}

void
animate_fclose(FILE *fp)
{
	if (anim_filename)
		free(anim_filename);
	anim_filename = NULL;
	anim_lineno = 0;
	fclose(fp);
}

int
animerror(char *msg)
{
	fprintf(stderr,"%s %d: %s at '%s'\n",
			anim_filename, anim_lineno, msg, animtext);
}

int animwrap(void)
{
	return 1;
}
