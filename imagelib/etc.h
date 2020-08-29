/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/
#ifndef __ETC_UTILS_H
#define __ETC_UTILS_H

/* etc.c */
void set_verbose( int v );
void msg_out(const char *fmt,...);
void err_out(const char *fmt,...);
void set_search_path( char *s );
char *search(char *fname);

#endif
