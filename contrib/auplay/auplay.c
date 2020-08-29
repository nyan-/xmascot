#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/ioctl.h>

#include"auplay.h"

#define AUDIO_DEV "/dev/audio"

static int audio_fd = -1;
static char *title;

#ifdef USE_AUCNTL

static int vol;

void set_gain(gain)
  int gain;
{
    audio_info_t auinfo;
    AUDIO_INITINFO(&auinfo);
    auinfo.play.gain = gain;
    ioctl(audio_fd, AUDIO_SETINFO, &auinfo);
}

#endif

int open_audio()
{
    if(audio_fd < 0)
    {
        if((audio_fd = open(AUDIO_DEV, O_WRONLY)) < 0)
        {
            return -1;
        }
    }
    return audio_fd;
}

void close_audio()
{
    if(audio_fd >= 0)
    {
        close(audio_fd);
        audio_fd = -1;
    }
}

void opts(ac,av)
  int ac;
  char **av;
{
#define NEXTARG { if(ac > 1) { ac--;av++; } else { break; } }
  while(1) {
    NEXTARG;
    if((*av)[0] == '-' ) {
      if((*av)[1] == 'h' && (*av)[2] == '\0') {
	fprintf(stderr,"Usage: %s [options ...] audiofiles ...\n",title);
	fprintf(stderr,"  options:\n");
	fprintf(stderr,"    -h        : print this message\n");
#ifdef USE_AUCNTL
	fprintf(stderr,"    -v #      : volume (%d - %d)\n",AUDIO_MIN_GAIN,AUDIO_MAX_GAIN);
#endif
	exit(0);
      }
#ifdef USE_AUCNTL
      else if((*av)[1] == 'v' && (*av)[2] == '\0') {
	*av = NULL;
	NEXTARG;
	vol = atoi(*av);
	*av = NULL;
	if(vol < AUDIO_MIN_GAIN) {
	  vol = AUDIO_MIN_GAIN;
	}
	else if(vol > AUDIO_MAX_GAIN) {
	  vol = AUDIO_MAX_GAIN;
	}
	set_gain(vol);
      }
#endif
      else {
	fprintf(stderr,"Bad option '%s'\n",*av);
	*av = NULL;
      }
    }
  }
#undef NEXTARG
}

void playfile(fp)
  int fp;
{
  int size;
  char buf[1024];
  if(audio_fd >= 0 && fp >= 0) {
    while((size = read(fp,buf,1024)) > 0) {
      write(audio_fd,buf,size);
    }
  }
}

int main(ac,av)
  int ac;
  char **av;
{
  int inflag = 0;
  int fp;
  int fd;
  int try_c;
  title = av[0];
  for(try_c = 1; try_c < 60; try_c++) {
    fd = open_audio();
    if(fd >= 0) {
      opts(ac,av);
      ac--;av++;
      while(ac > 0) {
        if(*av != NULL) {
	  inflag = 1;
	  if((fp = open(*av,O_RDONLY)) >= 0) {
	    playfile(fp);
	    close(fp);
	  }
        }
        ac--;av++;
      }
      if(inflag == 0) {
        playfile(0);
      }
      close_audio();
      break;
    }
    else {
      sleep(1);
    }
  }
  return 0;
}
