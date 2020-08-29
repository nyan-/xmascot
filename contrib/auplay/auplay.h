/* auplay.h */

/*
  The volume option (-v #) needs '/dev/audio' controll include file,
 ('audioio.h') .
  If you don't have it, change this to '#undef USE_AUCNTL' .
*/
#define USE_AUCNTL

#ifdef USE_AUCNTL

/*
  Write a name of '/dev/audio' controll include file .
    Examples
      NetBSD        #include<sys/audioio.h>
      SunOS 4.1.x   #include<sun/audioio.h>
*/
#include <sys/audioio.h>

#endif
