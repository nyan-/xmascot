#include<time.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>

#include "image.h"

Display *dpy;
Window root;
int scrn;
unsigned int depth;
GC gc;

void
sleep_us( long us )
{
	struct timeval t;
	t.tv_sec  = 0;
	t.tv_usec = us;
	select( 0,NULL,NULL,NULL,&t );
}

void
main(int ac,char **av){
  Pixmap bkup,work,pixmap;
  int width = 96;
  int height = 104;
  int x,y;
  int gx,gy,gw,gh,gf = 0;
  int i;
  int r;
  int w;
  int h;
  int p_w;
  int p_h;
  int repeat_flag = 1;
  char *fname = NULL;
  char *dpy_name = NULL;
  char *fg = NULL;
  char *bg = NULL;
  unsigned long fgp,bgp;
  int fl = 1;
  static char *usage = 
    "Usege : %s [options] file\n"
    "  options\n"
    "    -display  <disp> : display\n"	
    "    -geometry <geom> : geometry\n"
    "    -v               : verbose mode\n"
    "    -fg <color>      : foreground color\n"
    "    -bg <color>      : background color\n"
    "    -repeat <n>      : repeat\n"
    "    -nooverride      : on root window only\n";
  for(i=1;i<ac;i++){
    if( av[i][0] == '-' ){
      switch( av[i][1] ){
       case 'd':
	if( ++i < ac ){
	  dpy_name = av[i];
	}
	break;	
       case 'g':
	if( ++i < ac ){
	  gf = XParseGeometry( av[i],&gx,&gy,&gw,&gh );
	}
	break;
       case 'v':
	set_verbose(1);
	break;
       case 'f':
	if( ++i < ac ){
	  fg = av[i];
	}
	break;
       case 'b':
	if( ++i < ac ){
	  bg = av[i];
	}
	break;
       case 'r':
	if( ++i < ac ){
	  repeat_flag = atoi(av[i]);
	}
	break;
       case 'n':
	fl = 0;
	break;
       default:
	err_out( usage, av[0] );
	exit(1);
      }
    }else
      fname = av[i];
  }
  if( (dpy = XOpenDisplay( dpy_name )) == NULL ){
    err_out("Cant Open Display!\n");
    exit(1);
  }
  scrn  = DefaultScreen(dpy);
  root = RootWindow(dpy,scrn);
  depth	= DefaultDepth(dpy,scrn);
  w = DisplayWidth(dpy,scrn);
  h = DisplayHeight(dpy,scrn);

  gc = XCreateGC(dpy,root,0,NULL);
  if(fl)
    XSetSubwindowMode(dpy,gc,IncludeInferiors);
  if( fg != NULL ){
    XColor c,e;
    XAllocNamedColor(dpy,DefaultColormap(dpy,scrn),fg,&c,&e);
    fgp = c.pixel;
  }else
    fgp = BlackPixel(dpy,scrn);
  if( bg != NULL ){
    XColor c,e;
    XAllocNamedColor(dpy,DefaultColormap(dpy,scrn),bg,&c,&e);
    bgp = c.pixel;
  }else
    bgp = BlackPixel(dpy,scrn);
  XSetForeground(dpy,gc,bgp);

  srand((int)time(NULL));

  if(fname != NULL) {
	ImageData *base = image_new();
    image_load(base,fname,-1,-1);
    if( base->data == NULL ){
      err_out("Can't open %s\n",fname );
      exit(1);
    }
    image_set_col(base, dpy, root);
    p_w = base->width;
    p_h = base->height;
    pixmap = image_pixmap(base,NULL);
    image_free_data(base);
  }
  else {
    if((gf&(WidthValue|HeightValue)) == (WidthValue|HeightValue)) {
      p_w = gw;
      p_h = gh;
    } else {
      p_w = rand()%(w/2+16);
      p_h = rand()%(w/2+16);
    }
    pixmap = XCreatePixmap(dpy,root,p_w,p_h,depth);
    XFillRectangle(dpy,pixmap,gc,0,0,p_w,p_h);
  }
  XSetForeground(dpy,gc,fgp);
  do {
    width = p_w+2;
    height = p_h+2;
    if((gf&(XValue|YValue)) == (XValue|YValue)) {
      x = gx;
      y = gy;
    } else {
      x = rand()%(w-width);
      y = rand()%(h-height);
    }
    bkup = XCreatePixmap(dpy,root,width,height,depth);
    work = XCreatePixmap(dpy,root,width,height,depth);
    
    XCopyArea(dpy,root,bkup,gc,x,y,width,height,0,0);
    XCopyArea(dpy,bkup,work,gc,0,0,width,height,0,0);
    XDrawRectangle(dpy,work,gc,0,0,width-1,height-1);
    XCopyArea(dpy,work,root,gc,0,0,width,height,x,y);
    XSync(dpy,True);
    
    sleep(1);
    for(r=0;r<width;r+=width/64+1) {
      XCopyArea(dpy,pixmap,work,gc,0,0,p_w,p_h,1,1);
      XCopyArea(dpy,bkup,work,gc,0,0,width,height,r,0);
      XDrawRectangle(dpy,work,gc,0,0,width-1,height-1);
      XCopyArea(dpy,work,root,gc,0,0,width,height,x,y);
      XSync(dpy,True);
      sleep_us(10000);
    }
    XCopyArea(dpy,pixmap,work,gc,0,0,p_w,p_h,1,1);
    XDrawRectangle(dpy,work,gc,0,0,width-1,height-1);
    XCopyArea(dpy,work,root,gc,0,0,width,height,x,y);
    XSync(dpy,True);
    sleep(1);
    for(;r>=0;r-=width/64+1) {
      XCopyArea(dpy,pixmap,work,gc,0,0,p_w,p_h,1,1);
      XCopyArea(dpy,bkup,work,gc,0,0,width,height,r,0);
      XDrawRectangle(dpy,work,gc,0,0,width-1,height-1);
      XCopyArea(dpy,work,root,gc,0,0,width,height,x,y);
      XSync(dpy,True);
      sleep_us(10000);
    }
    XCopyArea(dpy,bkup,work,gc,0,0,width,height,0,0);
    XDrawRectangle(dpy,work,gc,0,0,width-1,height-1);
    XCopyArea(dpy,work,root,gc,0,0,width,height,x,y);
    XSync(dpy,True);
    sleep(1);
    XCopyArea(dpy,bkup,root,gc,0,0,width,height,x,y);
    XSync(dpy,True);
    XFreePixmap(dpy,bkup);
    XFreePixmap(dpy,work);
  } while(--repeat_flag > 0);
}
