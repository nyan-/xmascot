/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "xmascot.h"

/* 設定ファイル */
#define RCFILE ".xmascotrc"

extern Widget   top;

#define LINEBUF 1024

const char     *reserve1[] = {
	"dummy_no_data",
	"inter_timer",
	"houry_alarm",
	"half_alarm",
};

const char     *reserve2[] = {
	"timer_action",
	"inter_action",
	"houry_action",
	"half_action",
};

static char    *token_p;
static char     buf[LINEBUF + 1];

void 
set_token(char *p)
{
	token_p = p;
}

char            token_buf[LINEBUF + 1];
int             token_dat;

int 
get_token(void)
{
	char            ch;

	while ((ch = *token_p++) == ' ' || ch == '\t' || ch == '\n');

	if (ch == '#' || ch == '\0')
		return NODAT;

	if (ch == '"') {
		char           *p = token_buf;
		for (;;) {
			ch = *token_p++;
			if (ch == '"')
				break;
			else if (ch == '\\') {
				ch = *token_p++;
			}
			if (ch == '\n' || '\0')
				return NODAT;
			*p++ = ch;
		}
		*p = '\0';
		return STRING;
	} else if (isalnum(ch) || ch == '_' || ch == '.') {
		int             i;
		char           *p = token_buf;
		*p++ = ch;
		while (isalnum((ch = *token_p)) || ch == '_' || ch == '.') {
			*p++ = ch;
			token_p++;
		}
		*p = '\0';
		for (i = 0; i < ALARM_NUM; i++) {
			if (!strncmp("alarm", token_buf, 5) && token_buf[5] - '0' == i) {
				token_dat = i;
				return RESERVE;
			}
			if (!strncmp("action", token_buf, 6) && token_buf[6] - '0' == i) {
				token_dat = ALARM_ALLNUM + i;
				return RESERVE;
			}
		}
		for (i = 0; i < 4; i++) {
			if (!strcmp(reserve1[i], token_buf)) {
				token_dat = ALARM_NUM + i;
				return RESERVE;
			}
		}
		for (i = 0; i < 4; i++) {
			if (!strcmp(reserve2[i], token_buf)) {
				token_dat = ALARM_ALLNUM + ALARM_NUM + i;
				return RESERVE;
			}
		}
#ifdef BIFF
		if (!strcmp("biff_action", token_buf)) {
			token_dat = ALARM_ALLNUM + ALARM_NUM + 4;
			return RESERVE;
		}
#endif
		return ID;
	} else {
		token_dat = ch;
		return SYMBOL;
	}
}

int 
get_token2(void)
{
	char            ch;

	while ((ch = *token_p++) == ' ' || ch == '\t' || ch == '\n');

	if (ch == '\0')
		return NODAT;

	if (ch == '"') {
		char           *p = token_buf;
		for (;;) {
			ch = *token_p++;
			if (ch == '"')
				break;
			else if (ch == '\\') {
				ch = *token_p++;
			}
			if (ch == '\n' || '\0')
				return NODAT;
			*p++ = ch;
		}
		*p = '\0';
		return STRING;
	} else {
		char           *p = token_buf;
		*p++ = ch;
		while (!((ch = *token_p) == ' ' || ch == '\t' || ch == '\n' || ch == '\0')) {
			*p++ = ch;
			token_p++;
		}
		*p = '\0';
		return ID;
	}
}

extern Alarm    alarm_dat[];
extern AppData  adat;
#ifdef BIFF
extern String   biff_action;
#endif

void 
get_rcfile(void)
{
	FILE           *fp;
	char           *p, *path;
	int             i, num, hour;

	for (i = 0; i < ALARM_ALLNUM; i++) {
		alarm_dat[i].sw =
			alarm_dat[i].hour =
			alarm_dat[i].min = 0;
		alarm_dat[i].action = XtNewString(adat.def_act);
		alarm_dat[i].id = -1;
	}
#ifdef BIFF
	biff_action = XtNewString(adat.biff_action);
#endif
	p = getenv("HOME");
	path = XtMalloc(strlen(p) + strlen(RCFILE) + 1 + 1);
	strcpy(path, p);
	strcat(path, "/");
	strcat(path, RCFILE);
	if ((fp = fopen(path, "r")) != NULL) {
		while (fgets(buf, LINEBUF, fp) != NULL) {
			set_token(buf);
			if (get_token() == RESERVE) {
#ifdef BIFF
				if (token_dat == ALARM_ALLNUM + ALARM_NUM + 4) {
					if (get_token() == STRING)
						set_new_string(&biff_action, token_buf);
				} else
#endif
				if (token_dat < ALARM_ALLNUM) {
					num = token_dat;
					if (get_token() == ID) {
						alarm_dat[num].sw = (strcmp(token_buf, "on")) ? 0 : 1;
					}
					if (get_token() == ID) {
						hour = atoi(token_buf);
						if (get_token() == SYMBOL && token_dat == ':')
							if (get_token() == ID) {
								alarm_dat[num].hour = hour;
								alarm_dat[num].min = atoi(token_buf);
							}
					}
				} else {
					num = token_dat - ALARM_ALLNUM;
					if (get_token() == STRING)
						set_new_string(&alarm_dat[num].action, token_buf);
				}
			}
		}
		fclose(fp);
	}
	XtFree(path);
	msg_out("Read .xmascotrc DONE.\n");
}

static void 
put_rc_string(FILE * fp, String str)
{
	char           *p = str;

	fputc('\"', fp);
	while (*p != '\0') {
		if (*p == '\"') {
			fputc('\\', fp);
			fputc('\"', fp);
		} else
			fputc(*p, fp);
		p++;
	}
	fprintf(fp, "\"\n");
}

void 
put_rcfile(void)
{
	FILE           *fp;
	char           *p, *path;
	int             i, n;

	p = getenv("HOME");
	path = XtMalloc(strlen(p) + strlen(RCFILE) + 1 + 1);
	strcpy(path, p);
	strcat(path, "/");
	strcat(path, RCFILE);
	if ((fp = fopen(path, "w")) != NULL) {
		for (i = 0; i < ALARM_NUM; i++) {
			fprintf(fp, "alarm%d %s %02d:%02d\n", i, (alarm_dat[i].sw) ? "on" : "off",
				alarm_dat[i].hour, alarm_dat[i].min);
			if (alarm_dat[i].action != NULL) {
				fprintf(fp, "action%d ", i);
				put_rc_string(fp, alarm_dat[i].action);
			}
		}
		for (i = 0; i < 4; i++) {
			n = ALARM_NUM + i;
			if (i == 1) {	/* interval */
				fprintf(fp, "%s %s %02d:%02d\n", reserve1[i],
					(alarm_dat[n].sw) ? "on" : "off",
					alarm_dat[n].hour, alarm_dat[n].min);
			} else if (i != 0) {
				fprintf(fp, "%s %s 00:00\n", reserve1[i],
					(alarm_dat[n].sw) ? "on" : "off");
			}
			if (alarm_dat[n].action != NULL) {
				fprintf(fp, "%s ", reserve2[i]);
				put_rc_string(fp, alarm_dat[n].action);
			}
		}
#ifdef BIFF
		/* for biff */
		if (biff_action != NULL) {
			fprintf(fp, "biff_action ");
			put_rc_string(fp, biff_action);
		}
#endif
		fclose(fp);
	} else
		err_out("warning: Can't write rcfile.");
	XtFree(path);
}

/* アクションの駆動 */
void 
action_parse(char *action)
{
	String          ptr[10];
	int             num, t;
	char           *act = NULL;

	if (action == NULL)
		return;
	set_token(action);
	t = get_token();

	for (;;) {
		if (t == NODAT)
			break;
		else if (t == ID) {
			num = 0;
			if (act != NULL)
				XtFree(act);
			act = XtNewString(token_buf);
			t = get_token();
			if (t == SYMBOL) {
				if (token_dat == '(') {
					t = get_token();
					for (; num < 10;) {
						if (t == NODAT)
							break;
						if (t == STRING || t == ID || t == RESERVE) {
							ptr[num++] = XtNewString(token_buf);
							t = get_token();
						} else if (t == SYMBOL && token_dat == ')') {
							t = get_token();
							break;
						} else
							t = get_token();
					}
					XtCallActionProc(top, act, NULL, ptr, num);
				} else
					XtCallActionProc(top, act, NULL, NULL, 0);
			} else
				XtCallActionProc(top, act, NULL, NULL, 0);
		} else
			t = get_token();
	}
	if (act != NULL)
		XtFree(act);
}
