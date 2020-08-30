/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xmu/Converters.h>
#include <X11/Xlocale.h>

#include "CascadeMenu.h"
#include "xmascot.h"

/* ウィンドウ基本属性 */
double          disp_dpm;
Atom            wm_protocols[2];

XtAppContext    app;

/* ウィジェット */
Widget          top, mascot;
Widget         *chain = NULL;
#ifdef BIFF
Widget          biff;
#endif

/* リソース */
extern AppData  adat;		/* リソースデータ構造体     */

/* 変更メニュー用の選択マーク */
Pixmap          select_mark;
#include "xbmfile/select.xbm"

/* アイコン */
Pixmap          icon;
#include "xbmfile/icon.xbm"

/* コマンドラインオプション */
static XrmOptionDescRec options[] = {
	{"-verbose", ".verbose", XrmoptionNoArg, "True"},
	{"-gravity", ".gravity", XrmoptionSepArg, NULL},
	{"-chainlength", ".chainLen", XrmoptionSepArg, NULL},
	{"-damping", ".dampCoeff", XrmoptionSepArg, NULL},
	{"-degree", ".degree", XrmoptionSepArg, NULL},
	{"-menuno", ".menuNo", XrmoptionSepArg, NULL},
	{"-no", ".mascotNo", XrmoptionSepArg, NULL},
	{"-magnify", ".magnifyBase", XrmoptionSepArg, NULL},
	{"-pinpat", ".pinPattern", XrmoptionSepArg, NULL},
	{"-random", ".random", XrmoptionNoArg, "True"},
	{"-changetime", ".changeTime", XrmoptionSepArg, NULL},
	{"-allmenu", ".allMenu", XrmoptionNoArg, "True"},
	{"-chainnum", ".chainNum", XrmoptionSepArg, NULL},
#ifdef USE_CHAINPAT
	{"-chainpat", ".chainPattern", XrmoptionSepArg, NULL},
#endif
	{"-searchpath", ".search", XrmoptionSepArg, NULL},
#ifdef SOUND
	{"-soundcmd", ".soundCommand", XrmoptionSepArg, NULL},
	{"-soundstart", ".masDatX.startSnd", XrmoptionSepArg, NULL},
	{"-soundclick", ".masDatX.clickSnd", XrmoptionSepArg, NULL},
	{"-soundend", ".masDatX.endSnd", XrmoptionSepArg, NULL},
#ifdef BIFF
	{"-soundmail", ".masDatX.mailSnd", XrmoptionSepArg, NULL},
#endif				/* BIFF */
#endif				/* SOUND */
#ifdef BIFF
	{"-nobiff", ".biff", XrmoptionNoArg, "False"},
	{"-update", ".update", XrmoptionSepArg, NULL},
	{"-noonce", ".biffOnce", XrmoptionNoArg, "False"},
	{"-biffcmd", ".biffCmd", XrmoptionSepArg, NULL},
	{"-biffpat", ".biffPattern", XrmoptionSepArg, NULL},
	{"-biffpos", ".MasDat.biffPos", XrmoptionSepArg, NULL},
#ifdef BIFF_LIST
	{"-biffgeometry", ".biffList.geometry", XrmoptionSepArg, NULL},
	{"-bifffilter", ".biffFilter", XrmoptionSepArg, NULL},
	{"-popdowntime", ".biffPopdown", XrmoptionSepArg, NULL},
	{"-nobifflists", ".biffPopdown", XrmoptionNoArg, "0"},
#endif
#ifdef YOUBIN
	{"-noyoubin", ".youbin", XrmoptionNoArg, "False"},
	{"-server", ".server", XrmoptionSepArg, NULL},
#endif
#endif
#ifdef SHADOW
	{"-shadow", ".shadow", XrmoptionSepArg, NULL},
	{"-noshadow", ".shadow", XrmoptionNoArg, "0"},
#endif
	{"-drawtiming", ".drawTiming", XrmoptionSepArg, NULL},
};

/* ピンのデフォルトアクション */
String          trans =
"<Message>WM_PROTOCOLS: quitmsg()\n"
"<MapNotify>: map()\n"
"<UnmapNotify>: unmap()\n"
"<ConfigureNotify>: config()\n"
"<Btn1Down>: press()\n"
"<Btn1Up>: release_pin()\n"
"<Btn1Motion>: motion_pin()\n"
"<Btn3Down>: XawPositionSimpleMenu(popup) XtMenuPopup(popup)";

/* マスコットのデフォルトアクション */
String          trans_mas =
"<Btn1Down>: press()\n"
"<Btn1Up>: release_mascot()\n"
"<Btn1Motion>: motion_mascot()\n"
"<Btn3Down>: snd_click()";

#ifdef BIFF
#ifdef BIFF_LIST
String          trans_biff = "<Btn1Down>: showbiff()";
#endif
#endif

/* アクションリスト */
static XtActionsRec actions[] = {
	{"map", MapWin},
	{"unmap", UnMapWin},
	{"config", ConfigWin},
	{"press", Press},
	{"release_pin", ReleasePin},
	{"release_mascot", ReleaseMascot},
	{"motion_pin", MotionPin},
	{"motion_mascot", MotionMascot},

	{"quit", Quit},
	{"quitmsg", QuitMsg},
	{"change", ChangeMascot},
	{"chg_file", ChangeMascotFile},
	{"chg_next", ChangeMascotNext},
	{"chg_next_all", ChangeMascotNextAll},
	{"chg_random", ChangeMascotRandom},
	{"chg_random_all", ChangeMascotRandomAll},
	{"start_move", StartMove},

	{"sound", Sound},
	{"snd_start", SoundStart},
	{"snd_click", SoundClick},
	{"snd_end", SoundEnd},
	{"snd_mail", SoundMail},

	{"system", System},
	{"chg_param", ChangeParam},
	{"bell", Bell},
#ifdef BIFF
#ifdef BIFF_LIST
	{"showbiff", ShowBiffNotice},
	{"biffnopdown", BiffEnter},
#endif
#endif
};

/* RGB文字列 -> long のコンバータ */
static Boolean 
XtRgbCvt(Display * dpy, XrmValue * args, Cardinal * num,
	 XrmValue * from, XrmValue * to, XtPointer * converter_data)
{
	char            str[20];
	static int     rgb;
	strcpy(str, (String) from->addr);

	if (!strcmp(str, "auto") || !strcmp(str, "AUTO")) {
		to->size = sizeof(rgb);
		rgb = -1;
		to->addr = (XtPointer)&rgb;
		return True;
	}
	if (strlen(str) == 6) {
		if (isxdigit(str[0]) &&
		    isxdigit(str[1]) &&
		    isxdigit(str[2]) &&
		    isxdigit(str[3]) &&
		    isxdigit(str[4]) &&
		    isxdigit(str[5])) {
			sscanf(str, "%x", &rgb);
			to->size = sizeof(int);
			to->addr = (XtPointer) & rgb;
			return True;
		}
	}
	XtDisplayStringConversionWarning(dpy, from->addr, "XtRRgb");
	to->addr = NULL;
	to->size = 0;
	return False;
}

/* 鎖の生成 (2回以上呼び出し可能) */
void
create_chains(int num)
{
	int             i;
	XTextProperty   winname;
	char           *window_name = "xmascot_chain";
	XStringListToTextProperty(&window_name, 1, &winname);

	if (chain) {
		for (i = 0; i < adat.chain_num; i++)
			XtDestroyWidget(chain[i]);
		XtFree((char *) chain);
	}
	chain = (Widget *) XtMalloc(sizeof(Widget) * num);
	for (i = 0; i < num; i++) {
		chain[i] = XtCreateWidget("chains",
								  overrideShellWidgetClass, top, NULL, 0);
		XtSetMappedWhenManaged(chain[i], False);
		XtVaSetValues(chain[i], 
					  XtNwidth, (Dimension)CHAIN_SIZE,
					  XtNheight,(Dimension)CHAIN_SIZE,
					  XtNborderWidth, (Dimension)1, NULL);
		XtRealizeWidget(chain[i]);
	}
	adat.chain_num = num;
}

/* メニュー関連 */

#if defined(BIFF) && defined(BIFF_LIST)
/* メール一覧 */
static Widget   ml_menu;
extern int      mbox_flag;
static void
MailLists(Widget w, XtPointer dat, XtPointer call)
{
	ShowBiffNotice(NULL, NULL, NULL, NULL);
}
/* メインメニューの初期設定 */
static void
set_main_menu(Widget w, XtPointer dat, XtPointer call)
{
	if (mbox_flag)
		XtVaSetValues(ml_menu, XtNsensitive, True, NULL);
	else
		XtVaSetValues(ml_menu, XtNsensitive, False, NULL);
}
#endif

/* 終了 */
void
ExitDialog(Widget w, XtPointer dat, XtPointer call)
{
	ExitApp();
}


/* X 関連の初期化 */
void
xinit(int *argc, char **argv)
{
	int             i;
	Widget          popup;

	/* アプリの初期化 */
#ifdef USE_SESSION
	top = XtVaOpenApplication(&app, "XMascot", options, XtNumber(options),
			  argc, argv, NULL, sessionShellWidgetClass, NULL);
#else
	top = XtVaAppInitialize(&app, "XMascot", options, XtNumber(options),
				argc, argv, NULL, NULL);
#endif

	/* リソースコンバータの登録 */
	XtAppAddConverter(app, XtRString, XtRJustify, XmuCvtStringToJustify,
			  (XtConvertArgList) NULL, 0);
	XtAppSetTypeConverter(app, XtRString, XtRRgb, XtRgbCvt,
			      (XtConvertArgList) NULL, 0, XtCacheNone,
			      (XtDestructor) NULL);

	/* 各種パラメータ変数 */
	disp_dpm = HeightOfScreen(XtScreen(top))
		/ HeightMMOfScreen(XtScreen(top));

	get_resources(top);

	/* 引数にマスコットのファイル名を指定した場合 */
	if (*argc != 1) {
		MascotMenu *m = &adat.mascot_menus[0];
		adat.mascot_number = m->n_mascots;
		m->mascots[m->n_mascots].fname = 
			m->mascots[m->n_mascots].title = argv[1];
	}
	XawSimpleMenuAddGlobalActions(app);
	XtAppAddActions(app, actions, XtNumber(actions));
	XtOverrideTranslations(top, XtParseTranslationTable(trans));
	XtVaSetValues(top,
				  XtNwidth, (Dimension)1,
				  XtNheight,(Dimension)1,
				  XtNborderWidth, (Dimension)0, NULL);
	XtSetMappedWhenManaged(top, False);
	XtRealizeWidget(top);
	XSelectInput(XtDisplay(top), XtWindow(top),
				 XtBuildEventMask(top) | PointerMotionHintMask);

	/* 選択マークのビットマップ生成 */
	select_mark = XCreateBitmapFromData(XtDisplay(top), XtWindow(top),
										(char *) select_bits,
										select_width, select_height);
	/* アイコンのビットマップ生成 */
	icon = XCreateBitmapFromData(XtDisplay(top), XtWindow(top),
								 (char *) icon_bits,
								 icon_width, icon_height);
	XtVaSetValues(top, XtNiconPixmap, icon, NULL);

	/* 終了用プロパティ */
	wm_protocols[0] = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW", False);
	wm_protocols[1] = XInternAtom(XtDisplay(top), "WM_SAVEYOURSELF", False);
	XSetWMProtocols(XtDisplay(top), XtWindow(top), wm_protocols, 2);

	/* メインメニュー */
	popup = XtVaCreatePopupShell("popup", cascadeMenuWidgetClass, top, NULL);

	menu_add_line(popup);
	for (i = 0; i < adat.menus_num; i++)
		menu_add_cascade(popup, adat.mascot_menus[i].title, 
						 change_menu(top, i));
	menu_add_line(popup);
	menu_add_dialog(popup, "preference", preference_dialog(top));
	menu_add_dialog(popup, "alarm", alarm_dialog(top));
#if defined(BIFF) && defined(BIFF_LIST)
	ml_menu = menu_add_callback(popup, "list", MailLists);
	XtAddCallback(popup, XtNpopupCallback, set_main_menu, NULL);
#endif
	menu_add_line(popup);
	menu_add_dialog(popup, "about", about_dialog(top));
	menu_add_callback(popup, "exit", ExitDialog);
	
	/* mascot Window */
	mascot = XtCreateWidget("mascot_base",
							overrideShellWidgetClass, top, NULL, 0);
	XtOverrideTranslations(mascot, XtParseTranslationTable(trans_mas));
	XtSetMappedWhenManaged(mascot, False);
	XtVaSetValues(mascot, 
				  XtNwidth, (Dimension)1,
				  XtNheight,(Dimension)1,
				  XtNborderWidth, (Dimension)0, NULL);
	XtRealizeWidget(mascot);
	XSelectInput(XtDisplay(mascot), XtWindow(mascot),
				 XtBuildEventMask(mascot) | PointerMotionHintMask);

	/* chains Window */
	create_chains(adat.chain_num);

#ifdef BIFF
	/* biff Window */
	biff = XtCreateWidget("biff", overrideShellWidgetClass, top, NULL, 0);
	XtVaSetValues(biff, XtNsaveUnder, True, XtNoverrideRedirect, True, NULL);
#ifdef BIFF_LIST
	XtOverrideTranslations(biff, XtParseTranslationTable(trans_biff));
#endif
	XtSetMappedWhenManaged(biff, False);
	XtVaSetValues(biff,
				  XtNwidth, (Dimension)1,
				  XtNheight,(Dimension)1,
				  XtNborderWidth, (Dimension)0, NULL);
	XtRealizeWidget(biff);
#endif

	/* shell widget はカーソルを持ってない.. */
	XDefineCursor(XtDisplay(top), XtWindow(top), adat.cursor_normal);
	XDefineCursor(XtDisplay(mascot), XtWindow(mascot), adat.cursor_normal);
}

extern volatile int time_fl;	/* タイマ用フラグ     */

/* 子プロセスの処理 */
static void
ChildTerm(int dummy)
{
	int s;
	wait(&s);
}

/* マスコット定期切替え用ハンドラ (通常) */
static void
ChangeHand1(XtPointer cl, XtIntervalId * id)
{
	if (adat.all_menu)
		ChangeMascotNextAll(NULL, NULL, NULL, NULL);
	else
		ChangeMascotNext(NULL, NULL, NULL, NULL);
	XtAppAddTimeOut(app, adat.change_time * 60000, ChangeHand1, NULL);
}

/* マスコット定期切替え用ハンドラ (ランダム) */
static void
ChangeHand2(XtPointer cl, XtIntervalId * id)
{
	if (adat.all_menu)
		ChangeMascotRandomAll(NULL, NULL, NULL, NULL);
	else
		ChangeMascotRandom(NULL, NULL, NULL, NULL);
	XtAppAddTimeOut(app, adat.change_time * 60000, ChangeHand2, NULL);
}


#ifdef DUMMY_SETLOCALE
char *
setlocale(int cat, const char *locale)
{
	static char name[100] = "C";	/* XXX */
	if (locale) {
		if (!*locale) {
			char *s;
			if ((s = getenv("LC_CTYPE")) || (s = getenv("LANG")))
				strncpy(name, s, sizeof(name)-1);
			else
				strcpy(name, "C");
		} else {
			strncpy(name, locale, sizeof(name)-1);
		}
	}
	return name;
}
#endif

/* メイン */
int
main(int argc, char *argv[])
{
	signal(SIGCHLD, ChildTerm);
	srand((unsigned) getpid());	/* 乱数系初期化 */

#ifdef I18N
	/* Athene Widget I18N client */
	XtSetLanguageProc(NULL, NULL, NULL);
#endif
	xinit(&argc, argv);	/* widget/window 生成 */
	usage(&argc, argv);	/* オプションのチェック */
	get_rcfile();		/* 設定ファイルを読み込む */

#ifndef USE_DOUBLE
	isin_init();		/* isin,icos 初期化 */
#endif

	/* pin  のパターン設定 */
	set_widget_pattern(top, adat.pin_pat, adat.pcol0, adat.prgb0);
#ifdef BIFF
	/* biff のパターン設定 */
	set_widget_pattern(biff, adat.biff_pat, adat.bcol0, adat.brgb0);
#endif

#ifdef USE_CHAINPAT
	/* chain のパターン設定 */
	set_chain_pat(adat.chain_pat, adat.ccol0, adat.crgb0);
#endif

	/* マスコットのパターン設定 */
	set_mas(&adat);
	XtMapWidget(top);	/* マッピング   */
	set_alarms();		/* アラーム設定 */
#ifdef BIFF
	set_biff();		/* biff 設定    */
#endif

	if (adat.change_time) {
		if (adat.random)
			XtAppAddTimeOut(app, adat.change_time * 60000, ChangeHand1, NULL);
		else
			XtAppAddTimeOut(app, adat.change_time * 60000, ChangeHand2, NULL);
	}

	/* イベントループ */
	while (1) {
		if (XtAppPending(app) || time_fl <= 0)
			XtAppProcessEvent(app, XtIMAll);
		else
			sim();
	}
}
