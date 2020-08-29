/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#ifdef BIFF

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifndef XAW3D
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#else
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/AsciiText.h>
#endif

#include "xmascot.h"

#define BUFFSIZE 1024+1

#ifdef BIFF_LIST
static void     biff_list_init(void);
static void     biff_list_set(char *froms);
#endif

extern XtAppContext app;
extern Widget   	top, biff;
extern AppData  	adat;

extern char    *biff_filter[];

static char    *mbox_name;
String          biff_action;

extern int      map_fl;		/* 表示中か？ */
int             mbox_flag = 0;	/* すでにうけつけてるか？ */

#ifdef BIFF_LIST

static int      biff_popdown_flag;	/* 自動 popdown 用フラグ */
static Widget   p_bifflist = NULL, text;

char           *froms = NULL;	/* メール一覧表示用 */

/* 文字列の追加確保 */
static char*
strbuf(char *p)
{
	static char    *start = NULL;
	static char    *next;
	static int      len, len2, l;

	if (p == NULL) {
		len = len2 = 4096;
		start = next = (char *) XtMalloc(len + 1);
		start[0] = '\0';
	} else if (start != NULL) {
		l = strlen(p);
		while (len - l < 0) {
			char           *p;
			len2 += 256;
			len += 256;
			p = (char *) XtRealloc(start, len2 + 1);
			start = p;
			next = p + len2 - len;
		}
		strcpy(next, p);
		len -= l;
		next += l;
	}
	return start;
}

/* 外部メールチェックコマンド呼びだし */
static int 
extern_biff(void)
{
	FILE           *fp;
	char            buf[BUFFSIZE];
	int             fd[2], check;
	pid_t           pid;
	int             wait_status;
	void            (*sig) ();
	char           *p;

	sig = signal(SIGCHLD, SIG_IGN);
	if (pipe(fd)) {
		perror("pipe");
		exit(1);
	}
	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	/* 子プロセス */
	if (pid == 0) {
		close(1);
		dup(fd[1]);
		close(fd[1]);

		if ((p = strrchr(adat.biff_cmd, '/')) == NULL)
			p = adat.biff_cmd;
		else
			p++;

		execlp(adat.biff_cmd, p, (char *) NULL);
		perror("extern_biff:execlp");
		exit(1);
	}
	/* 親プロセス */
	close(fd[1]);
	if ((froms = strbuf(NULL)) != NULL) {
		if ((fp = fdopen(fd[0], "r")) != NULL) {
			while (fgets(buf, BUFFSIZE - 1, fp) != NULL)
				froms = strbuf(buf);
			fclose(fp);
		} else
			perror("fdopen");
	}
	if (wait(&wait_status) < 0) {
		perror("wait");
		check = -1;
	} else {
		if (WIFEXITED(wait_status))
			check = WEXITSTATUS(wait_status);
		else
			check = -1;
	}
	signal(SIGCHLD, sig);

	return check;
}

/* メールボックスから From: Subject: を取得 */
static void 
get_from(void)
{
	FILE           *fp;
	char            buf[BUFFSIZE];
	char           *p;

	char           *f, *s;

	if ((froms = strbuf(NULL)) != NULL) {
		if ((fp = fopen(mbox_name, "r")) != NULL) {
			do {
				if (fgets(buf, BUFFSIZE - 1, fp) == NULL)
					break;
				if (strncmp(buf, "From ", 5) == 0) {
					f = s = NULL;
					for (;;) {
						if ((p = fgets(buf, BUFFSIZE - 1, fp)) == NULL
						    || buf[0] == '\n' || buf[0] == '\0')
							break;
						if (strncmp(buf, "From:", 5) == 0)
							f = XtNewString(buf);
						else if (strncmp(buf, "Subject:", 8) == 0)
							s = XtNewString(buf);
					}
					/*
					 * From: Subject:
					 * の順に表示させる
					 */
					if (f) {
						froms = strbuf(f);
						XtFree(f);
					}
					if (s) {
						froms = strbuf(s);
						XtFree(s);
					}
				}
				while (p != NULL && buf[0] != '\n' && buf[0] != '\0')
					p = fgets(buf, BUFFSIZE - 1, fp);
			} while (1);
			fclose(fp);
		}
	}
}

/* フィルタコマンド呼びだし */
static char    *
code_conv(char *data)
{
	FILE           *fp;
	char            buf[BUFFSIZE];
	int             fd1[2], fd2[2];
	pid_t           pid;
	char           *p;

	pipe(fd1);
	pipe(fd2);

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	/* 子プロセス */
	if (pid == 0) {
		close(fd1[1]);
		close(fd2[0]);
		close(0);
		dup(fd1[0]);
		close(fd1[0]);
		close(1);
		dup(fd2[1]);
		close(fd2[1]);
		execvp(biff_filter[0], &biff_filter[1]);
		perror("execvp");
		exit(1);
	}
	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	/* 親その１出力 */
	if (pid == 0) {
		close(fd2[1]);
		close(fd2[0]);
		close(fd1[0]);

		if ((fp = fdopen(fd1[1], "w")) != NULL) {
			fputs(data, fp);
			fclose(fp);
		}
		exit(1);
	}
	/* 親その２入力 */
	close(fd1[1]);
	close(fd1[0]);
	close(fd2[1]);

	if ((p = strbuf(NULL)) != NULL) {
		if ((fp = fdopen(fd2[0], "r")) != NULL) {
			while (fgets(buf, BUFFSIZE - 1, fp) != NULL)
				p = strbuf(buf);
			fclose(fp);
		}
	}
	return p;
}

#endif				/* BIFF_LIST */

/* 外部メールチェックコマンド呼びだしその2 */
static int 
extern_biff2(void)
{
	int             check;
	pid_t           pid;
	int             wait_status;
	void            (*sig) ();
	char           *p;
	sig = signal(SIGCHLD, SIG_IGN);
	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	/* 子プロセス */
	if (pid == 0) {
		if ((p = strrchr(adat.biff_cmd, '/')) == NULL)
			p = adat.biff_cmd;
		else
			p++;
		execlp(adat.biff_cmd, p, (char *) NULL);
		perror("extern_biff:execlp");
		exit(1);
	}
	/* 親プロセス */
	if (wait(&wait_status) < 0) {
		perror("wait");
		check = -1;
	} else {
		if (WIFEXITED(wait_status))
			check = WEXITSTATUS(wait_status);
		else
			check = -1;
	}
	signal(SIGCHLD, sig);
	return check;
}

/* メールは来たかな？ */
void 
check_mbox(int mbox_mode)
{
	/* 空の場合 */
	if (mbox_mode == 0) {
		if (mbox_flag && map_fl) {
			XtUnmapWidget(biff);
#ifdef BIFF_LIST
			if (adat.biff_popdown != 0)
				XtPopdown(p_bifflist);
#endif
			mbox_flag = 0;
		}
	} else {
		/*
		 * メールが追加した時初、初回または常にアクションをするのな
		 * らアクション
		 */
		if ((!mbox_flag || !adat.biff_once) && mbox_mode == 1) {
#ifdef SOUND
			/* メール着信サウンド */
			xmascot_sound(&adat, SOUND_MAIL);
#endif
			action_parse(biff_action);
		}
		/*
		 * アイコン化してない && 初回なら biff
		 * マークの表示
		 */
		if (map_fl && !mbox_flag) {
			mbox_flag = 1;
			reset_pos();
			set_pos();
			XtMapWidget(biff);
			RaiseAll();
		}
#ifdef BIFF_LIST
		/* 一覧を表示する場合 */
		if (adat.biff_popdown != 0 && mbox_mode == 1) {
			/* 表示すべき内容がある場合それを表示 */
			if (froms != NULL) {
				/* フィルタが設定されている */
				if (biff_filter[0] != NULL) {
					char           *p;
					p = code_conv(froms);
					biff_list_set(p);
					XtFree(p);
				} else
					biff_list_set(froms);
			} else
				XtPopdown(p_bifflist);
		}
#endif
		mbox_flag = 1;
	}
	return;
}

/* mailbox のチェック */
static void 
check_mailbox(XtPointer cl, XtIntervalId * id)
{
	/* 以前の mbox のサイズ */
	static int      mbox_last_size = 0;
	int             mbox_size = 0;
	int             mbox_mode;
	struct stat     st;
	mbox_size = (stat(mbox_name, &st) == 0) ? st.st_size : 0;
#ifdef BIFF_LIST
	if (mbox_size != 0)
		get_from();
	else
		froms = NULL;
#endif
	if (mbox_size == 0)
		mbox_mode = 0;
	else if (mbox_size > mbox_last_size)
		mbox_mode = 1;
	else
		mbox_mode = 2;
	check_mbox(mbox_mode);
#ifdef BIFF_LIST
	if (froms != NULL)
		XtFree(froms);
#endif
	mbox_last_size = mbox_size;
	XtAppAddTimeOut(app, adat.biff_update * 1000, check_mailbox, NULL);
}

/* 外部コマンド呼びだしの場合 */
static void 
check_extern(XtPointer cl, XtIntervalId * id)
{
	int             check, mbox = 0;
#ifdef BIFF_LIST
	froms = NULL;
	if (adat.biff_popdown != 0)
		check = extern_biff();
	else
#endif
		check = extern_biff2();
	/* エラーまたは返り値が 1 なら空 */
	if (check < 0 || check == 1)
		mbox = 0;
	/* 返り値が 0 なら新規メール */
	else if (check == 0)
		mbox = 1;
	/* そうでなければ 変わらず */
	else
		mbox = 2;
	check_mbox(mbox);
#ifdef BIFF_LIST
	if (froms != NULL)
		XtFree(froms);
#endif
	XtAppAddTimeOut(app, adat.biff_update * 1000, check_extern, NULL);
}

#ifdef YOUBIN
/* youbin の場合 */
static void 
check_youbin(XtPointer cl, int *fdp, XtInputId * id)
{
	/* 以前の mbox のサイズ */
	static int      mbox_last_size = 0;
	int             len, fl = 0, mbox_size;
	int             mbox_mode;
	long            date;
	int             fd = *fdp;
	char            buf[BUFFSIZE];
	char           *p, *q;
	char           *f = NULL, *s = NULL;

	if ((len = read(fd, buf, BUFFSIZE)) <= 0) {
		err_out("youbin died!!\n");
		XtRemoveInput(*id);
	}
	buf[len] = '\0';
	mbox_size = (int) strtol(buf, &p, 10);
	date = strtol(p, &q, 10);

	msg_out("youbin: %d %d\n", mbox_size, date);

	if (mbox_size == 0)
		mbox_mode = 0;
	else if (mbox_size > mbox_last_size)
		mbox_mode = 1;
	else
		mbox_mode = 2;

#ifdef BIFF_LIST
	if (mbox_mode == 1) {
		p = strtok(q + 1, "\n");
		while (p != NULL) {
			if (!strncmp("From:", p, 5)) {
				f = p;
				fl++;
			} else if (!strncmp("Subject:", p, 8)) {
				s = p;
				fl++;
			}
			if (fl > 1 || *p == '\0')
				break;
			p = strtok(NULL, "\n");
		}
		if (f) {
			if (!froms)
				froms = strbuf(NULL);
			froms = strbuf(f);
			froms = strbuf("\n");
		}
		if (s) {
			if (!froms)
				froms = strbuf(NULL);
			froms = strbuf(s);
			froms = strbuf("\n");
		}
	} else {
		if (froms != NULL)
			XtFree(froms);
		froms = NULL;
	}
#endif
	check_mbox(mbox_mode);
	mbox_last_size = mbox_size;
}

/* 子プロセスで youbin を呼び出す */
void 
youbin_init(void)
{
	FILE           *fp;

	int             fd[2];
	pid_t           pid;

	if (pipe(fd)) {
		perror("pipe");
		exit(1);
	}
	if ((pid = fork()) < 0) {
		perror("fork");
		exit(1);
	}
	if (pid == 0) {
		close(1);
		dup(fd[1]);
		close(fd[1]);
		execlp("youbin", "youbin", "-b", "-s", adat.server, (char *) NULL);
		perror("youbin_init:execlp");
		exit(1);
	}
	close(fd[1]);

	if ((fp = fdopen(fd[0], "r")) == NULL) {
		perror("youbin_init:fdopen");
		exit(1);
	}
	XtAppAddInput(app, fileno(fp), (XtPointer) XtInputReadMask,
		      check_youbin, (XtPointer) NULL);
}
#endif				/* YOUBIN */

/* BIFF の設定 */
void 
set_biff(void)
{
	if (adat.biff_mode) {
#ifdef BIFF_LIST
		if (adat.biff_popdown != 0)
			biff_list_init();
#endif
#ifdef YOUBIN
		if (adat.youbin) {
			msg_out("youbin server:%s\n", adat.server);
			youbin_init();
		} else
#endif
		if (adat.biff_cmd == NULL) {
			char           *username = getlogin();
			if (!username) {
				struct passwd  *pw = getpwuid(getuid());
				if (!pw) {
					err_out("set_biff: Can't find your username.\n");
					return;
				}
				username = pw->pw_name;
			}
			mbox_name = XtMalloc(strlen(MAILBOXDIR) + strlen(username) + 2);
			strcpy(mbox_name, MAILBOXDIR);
			strcat(mbox_name, "/");
			strcat(mbox_name, username);
			msg_out("biff mailbox: %s\n", mbox_name);
			check_mailbox(NULL, NULL);
		} else {
			msg_out("biff command: %s\n", adat.biff_cmd);
			check_extern(NULL, NULL);
		}
	}
}

#ifdef BIFF_LIST

XtIntervalId    popdown_id = -1;

static void 
BiffPopdownHand(XtPointer cl, XtIntervalId * id)
{
	if (biff_popdown_flag)
		XtPopdown(p_bifflist);
	popdown_id = -1;
}

static void 
biff_list_set(char *froms)
{
#if 0
	XtVaSetValues(text, XtNstring, XtNewString(froms), NULL);
#endif
	XtVaSetValues(text, XtNstring, froms, NULL);
	XtPopup(p_bifflist, XtGrabNone);
	if (adat.biff_popdown > 0) {
		biff_popdown_flag = 1;
		if (popdown_id != -1)
			XtRemoveTimeOut(popdown_id);
		popdown_id = XtAppAddTimeOut(app, adat.biff_popdown * 1000,
					     BiffPopdownHand, NULL);
	}
}

static void 
popdown(Widget w, XtPointer dat, XtPointer a)
{
	XtPopdown(dat);
}

static String   trans =
"<EnterNotify>: biffnopdown()";

static String   trans_text =
"<MapNotify>: end-of-file()";

/* biff 一覧表示の初期化 */
static void 
biff_list_init(void)
{
	Widget          form, cmd, label;

	p_bifflist = XtVaCreatePopupShell("biffList",
				       topLevelShellWidgetClass, top, NULL);
	XtOverrideTranslations(p_bifflist, XtParseTranslationTable(trans));

	form = XtVaCreateManagedWidget("base", formWidgetClass, p_bifflist, NULL);
	label = XtVaCreateManagedWidget("title", labelWidgetClass, form, NULL);
	text = XtVaCreateManagedWidget("text", asciiTextWidgetClass, form,
				       XtNdisplayCaret, False,
				       XtNeditType, XawtextRead,
				       XtNtype, XawAsciiString,
				       NULL);
	XtOverrideTranslations(text, XtParseTranslationTable(trans_text));

	cmd = XtVaCreateManagedWidget("ok", commandWidgetClass, form, NULL);
	XtAddCallback(cmd, XtNcallback, popdown, p_bifflist);
}

/* アクション biff のメール一覧を表示させる */
void 
ShowBiffNotice(Widget w, XEvent * e, String * p, Cardinal * n)
{
	if (adat.biff_popdown != 0 && mbox_flag)
		XtPopup(p_bifflist, XtGrabNone);
}

/* アクション popdown 防止 */
void 
BiffEnter(Widget w, XEvent * e, String * p, Cardinal * n)
{
	biff_popdown_flag = 0;
}
#endif				/* BIFF_LIST */

#endif
