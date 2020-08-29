#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/time.h>
#include<unistd.h>

#include "image.h"

#ifndef SEARCH_PATH
#define SEARCH_PATH "/usr/X11R6/include/pixmaps"
#endif

Display *dpy;
Window   root;
int      screen;
int		 depth;

char *search_path = SEARCH_PATH ":";

volatile int dw;
volatile int dh;
unsigned width  = 32;
unsigned height = 32;
int dx = -1;
int dy = 1;

void
sleep_us( long us )
{
	struct timeval t;
	t.tv_sec  = 0;
	t.tv_usec = us;
	select( 0,NULL,NULL,NULL,&t );
}

int
main(int argc,char *argv[])
{
	Window	win; 
	XSizeHints xshint;

	int x = 0;
	int y = 0;
	unsigned w = 100;
	unsigned h = 100;
	int gf = 0;
	int 	 i;
	char     *bg = NULL;
	char	 *fg = NULL;
	char	 *fname = NULL;
	unsigned long fgp,bgp;
	GC       gc;
	Pixmap   pixmap;
	Pixmap   pixmap2;
	int fl = 0;
	char *dpy_name = NULL;
	long     usec = 10000;

	static char *usage = 
		"Usege : %s [options] file\n"
		"  options\n"
		"    -display  <disp> : display\n"	
		"    -geometry <geom> : geometry\n"
		"    -v               : verbose mode\n"
 		"    -size WWxHH      : size\n"
		"    -fg <color>      : foreground color\n"
		"    -bg <color>      : background color\n"
		"    -root            : on root window\n"
		"    -dir <dir>       : direction ( [+-]x[+-]y )\n"
       	"    -wait <u-sec>    : waiting time (micro sec)\n";
	
	for(i=1;i<argc;i++){
		if( argv[i][0] == '-' ){
			switch( argv[i][1] ){
			case 'd':
				if( argv[i][2] == 'i'){
					if( argv[i][3] == 's' ){
						if( ++i < argc ){
							dpy_name = argv[i];
						}
					}else if( argv[i][3] == 'r' ){
						if( ++i < argc ){
							sscanf( argv[i],"%d%d",&dx,&dy );
						}
					}
				}	
				break;	
			case 'g':
				if( ++i < argc ){
					gf = XParseGeometry( argv[i],&x,&y,&w,&h );
				}
				break;
			case 'v':
				set_verbose(1);
				break;
			case 's':
				if( ++i < argc ){
					sscanf( argv[i],"%dx%d",&width,&height );
				}
				break;
			case 'f':
				if( ++i < argc ){
					fg = argv[i];
				}
				break;
			case 'b':
				if( ++i < argc ){
					bg = argv[i];
				}
				break;
			case 'r':
				fl = 1;
				break;
			case 'w':
				if( ++i < argc ){
					usec = atoi(argv[i]);
				}
				break;
			default:
				err_out( usage, argv[0] );
				exit(1);
			}
		}else
			fname = argv[i];
	}

	if ((dpy = XOpenDisplay( dpy_name )) == NULL ){
		err_out("Cant Open Display!\n");
		exit(1);
	}
	screen  = DefaultScreen(dpy);
	root    = DefaultRootWindow(dpy);
	{
		XWindowAttributes attr;
		XGetWindowAttributes(dpy, root, &attr);
		depth = attr.depth;
	}
	gc      = XCreateGC(dpy,root,0,NULL);

	set_search_path(search_path);

	if (fname != NULL) {
		Pixmap p;
		ImageData *base = image_new();
		image_load(base,fname,-1,-1);
		if( base->data == NULL ){
			err_out("Can't open %s\n",fname );
			exit(1);
		}
		image_set_col(base, dpy, root);
		width  = base->width;
		height = base->height;
		p = image_pixmap(base,NULL);
		image_free_data(base);
		pixmap = XCreatePixmap(dpy,root,width*2,height*2,depth);
		XCopyArea(dpy,p,pixmap,gc,0,0,width,height,0,0);
		XFreePixmap(dpy,p);
	}else{
		XColor c,e;
		if (fg != NULL) {
			XAllocNamedColor(dpy,DefaultColormap(dpy,screen),fg,&c,&e);
			fgp = c.pixel;
		} else
			fgp = WhitePixel(dpy,screen);
		if (bg != NULL) {
			XAllocNamedColor(dpy,DefaultColormap(dpy,screen),bg,&c,&e);
			bgp = c.pixel;
		} else
			bgp = BlackPixel(dpy,screen);

		pixmap  = XCreatePixmap(dpy,root,width*2,height*2,depth);
		XSetForeground(dpy,gc,bgp);
		XFillRectangle(dpy,pixmap,gc,0,0,width,height);
		XSetForeground(dpy,gc,fgp);
		XFillRectangle(dpy,pixmap,gc,0,0,width/2,height/2);
		XFillRectangle(dpy,pixmap,gc,width/2,height/2,width/2,height/2);
	}	

	pixmap2 = XCreatePixmap(dpy,root,width,height,depth);
	XCopyArea(dpy,pixmap,pixmap,gc,0,0,width,height,width,0);
	XCopyArea(dpy,pixmap,pixmap,gc,0,0,width,height,0,height);
	XCopyArea(dpy,pixmap,pixmap,gc,0,0,width,height,width,height);
	XFlush(dpy);

	if ((gf&(WidthValue|HeightValue)) == (WidthValue|HeightValue)) {
		xshint.width = w;
		xshint.height = h;
		xshint.flags = USSize;
	} else {
		w = 100;
		h = 100;
		xshint.flags = 0;
	}
	if((gf&(XValue|YValue)) == (XValue|YValue)) {
		xshint.x = x;
		xshint.y = y;
		xshint.flags |= USPosition;
	} else {
		x = 0;
		y = 0;
	}
	if( fl == 1 ){
		win = root;
	}else{
		win = XCreateSimpleWindow(dpy,root,x,y,w,h,0,
						  WhitePixel(dpy,screen),BlackPixel(dpy,screen));
		XStoreName(dpy,win,"XSlide");
		XSetWMNormalHints(dpy,win,&xshint);
		XMapWindow(dpy,win);
	}

	msg_out("width x height: %d x %d\n",width,height);

	while(1){
		sleep_us( usec );
		XCopyArea(dpy,pixmap,
				  pixmap2,gc,dw,dh,width,height,0,0);
		XSetWindowBackgroundPixmap(dpy,win,pixmap2);
		XClearWindow(dpy,win);
		XSync(dpy,True);
		dw = (dw-dx+width) % width;
		dh = (dh-dy+height) % height;
	}
}
