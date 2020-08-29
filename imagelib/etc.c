/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "etc.h"

int verbose = 0;

void set_verbose(int v) {
	verbose = v;
}

/* メッセージの出力 -verbose で抑制解除 */
void msg_out(const char *fmt,...)
{
	va_list ap;
	if (verbose) {
		va_start(ap,fmt);
		vfprintf(stderr,fmt,ap);
		va_end(ap);
	}
}

/* メッセージ出力 */
void err_out(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vfprintf(stderr,fmt,ap);
	va_end(ap);
}	

/*
   サーチパスからファイルを探す。
   サーチパスの最後は ':' で終らなければならない。 
   見つからない時は NULL を返す
*/
static char *search_path = ":";
void set_search_path(char* s_p) {
	search_path = s_p;
}

char *search(char *fname)
{
	int l,fl;
	char *p,*q;
	static char *path = NULL;
	struct stat sb;
	
	if (fname == NULL) return NULL;
	msg_out("searching[%s]...",fname);
	/* カレントを探す */

	if (stat(fname,&sb)==0) {
		msg_out("[%s]\n",fname);
		return fname;
	}
	
	fl = strlen(fname);

	p = search_path;
	while ((q = strchr(p,':')) != NULL) {
		if ((l = q-p) > 0) {
			if (path != NULL)
				free(path);
			if ((path = malloc(l + fl + 2)) == NULL)
				return NULL;
			sprintf(path, "%.*s/%s", l, p, fname);
			if (stat(path,&sb)==0) {
				msg_out("[%s]\n",path);
				return path;
			}
		}
		p = q + 1;
	}
	msg_out("Not Found!\n");
	return NULL;
}
