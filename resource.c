/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi Iida   iida@cclub.tutcc.tut.ac.jp
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include "xmascot.h"

void            get_mascots_resources(Widget top, int n);

extern double   disp_dpm;	/* dot per mm */

#ifdef USE_DOUBLE
extern double   th;		/* 初期角度               */
#else
extern int      th;
#endif

#ifdef BIFF
char           *biff_filter[12];
extern char     token_buf[];
#endif

AppData         adat;		/* リソース格納用構造体   */

/* マスコット基本リソース群 */
static XtResource resources[] = {
	{
		"verbose", "Verbose",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, verbose),
		XtRImmediate, (XtPointer) FALSE
	},
	{
		"gravity", "Gravity",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, grav),
		XtRImmediate, (XtPointer) 163	/* on the MOON */
	},
	{
		"chainLen", "ChainLen",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, chain_len),
		XtRImmediate, (XtPointer) 70
	},
	{
		"dampCoeff", "DampCoeff",
		XtRFloat, sizeof(float),
		XtOffsetOf(AppData, f_damping),
		XtRString, (XtPointer) "0.01"
	},
	{
		"degree", "Degree",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, th),
		XtRImmediate, (XtPointer) 30
	},
	{
		"magnifyBase", "MagnifyBase",
		XtRFloat, sizeof(float),
		XtOffsetOf(AppData, magnify),
		XtRString, (XtPointer) "1.0"
	},
	{
		"action", "Action",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, def_act),
#ifdef SOUND
		XtRString, (XtPointer) "sound(pipipipi.au) start_move"
#else
		XtRString, (XtPointer) "bell(100) start_move"
#endif
	},
	{
		"search", "Search",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, search_path),
		XtRString, (XtPointer) ""
	},
	{
		"pinPattern", "Pattern",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, pin_pat),
		XtRString, (XtPointer) "pin.gif"
	},
	{
		"pcol0", "Col0",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, pcol0),
		XtRImmediate, (XtPointer)-1
	},
	{
		"prgb0", "Rgb0",
		XtRRgb, sizeof(int),
		XtOffsetOf(AppData, prgb0),
		XtRString, (XtPointer) "auto"
	},
	{
		"chainNum", "ChainNum",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, chain_num),
		XtRImmediate, (XtPointer) 6
	},
	{
		"random", "Random",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, random),
		XtRImmediate, (XtPointer) FALSE
	},
	{
		"changeTime", "Interval",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, change_time),
		XtRImmediate, (XtPointer) 0
	},
	{
		"allMenu", "AllMenu",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, all_menu),
		XtRImmediate, (XtPointer) FALSE
	},

#ifdef USE_CHAINPAT
	{
		"chainPattern", "Pattern",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, chain_pat),
		XtRString, (XtPointer) "chain.gif"
	},
	{
		"ccol0", "Col0",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, ccol0),
		XtRImmediate, (XtPointer) - 1
	},
	{
		"crgb0", "Rgb0",
		XtRRgb, sizeof(int),
		XtOffsetOf(AppData, crgb0),
		XtRString, (XtPointer) "auto"
	},
#endif
	{
		"menusNum", "MenusNum",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, menus_num),
		XtRImmediate, (XtPointer) 1
	},
	{
		"menuNo", "MenuNo",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, menu_no),
		XtRImmediate, (XtPointer) 0
	},
	{
		"mascotNo", "MascotNo",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, no),
		XtRImmediate, (XtPointer) 0
	},
#ifdef BIFF
	{
		"biff", "Biff",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, biff_mode),
		XtRString, "true"
	},
	{
		"biffOnce", "BiffOnce",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, biff_once),
		XtRString, "true"
	},
	{
		"update", XtCInterval,
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, biff_update),
		XtRString, "30"
	},
	{
		"biffCmd", "BiffCmd",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, biff_cmd),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"biffAction", "BiffAction",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, biff_action),
		XtRString, (XtPointer) "bell(100) start_move"
	},
	{
		"biffPattern", "Pattern",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, biff_pat),
		XtRString, (XtPointer) "mail_r.gif"
	},
#ifdef BIFF_LIST
	{
		"biffFilter", "BiffFilter",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, biff_filter),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"biffPopdown", XtCInterval,
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, biff_popdown),
		XtRString, (XtPointer) "10"
	},
#endif
#ifdef YOUBIN
	{
		"youbin", "Youbin",
		XtRBoolean, sizeof(Boolean),
		XtOffsetOf(AppData, youbin),
		XtRString, (XtPointer) "true"
	},
	{
		"server", "Server",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, server),
		XtRString, (XtPointer) "localhost"
	},
#endif				/* YOUBIN */
	{
		"bcol0", "Col0",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, bcol0),
		XtRImmediate, (XtPointer) - 1
	},
	{
		"brgb0", "Rgb0",
		XtRRgb, sizeof(int),
		XtOffsetOf(AppData, brgb0),
		XtRString, (XtPointer) "auto"
	},
#endif
#ifdef SOUND
	{
		"soundCommand", "SoundCommand",
		XtRString, sizeof(String),
		XtOffsetOf(AppData, snd_cmd),
		XtRImmediate, (XtPointer) NULL
	},
#endif
#ifdef SHADOW
	{
		"shadow", "Shadow",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, shadow),
		XtRImmediate, (XtPointer) 4
	},
#endif
	{
		"drawTiming", "DrawTiming",
		XtRInt, sizeof(int),
		XtOffsetOf(AppData, draw_timing),
		XtRImmediate, (XtPointer) 1
	},
	/* cursor font resource */
	{
		"cursor_normal", XtCCursor,
		XtRCursor, sizeof(Cursor),
		XtOffsetOf(AppData, cursor_normal),
		XtRString, (XtPointer) "FONT cursor 3",
	},
	{
		"cursor_click", XtCCursor,
		XtRCursor, sizeof(Cursor),
		XtOffsetOf(AppData, cursor_click),
		XtRString, (XtPointer) "FONT cursor 4",
	},
	{
		"cursor_drag", XtCCursor,
		XtRCursor, sizeof(Cursor),
		XtOffsetOf(AppData, cursor_drag),
		XtRString, (XtPointer) "FONT cursor 5",
	},
};

/* リソース取得 */
void 
get_resources(Widget top)
{
	int             i;
	char           *search_path;	/* サーチパス			 */

	/* 基本リソース */
	XtGetApplicationResources(top, &adat, resources,
							  XtNumber(resources), NULL, 0);
	set_verbose(adat.verbose);

	/* 各種パラメータの計算 */

	adat.chain_len *= disp_dpm;
	adat.damping = adat.f_damping;
	th = ANGLE_PI * adat.th / 180;

	/* メニュー関連 */
	msg_out("menu num:%d\n", adat.menus_num);
	adat.mascot_menus = 
		(MascotMenu *)XtMalloc(sizeof(MascotMenu) * (adat.menus_num));

	for (i = 0; i < adat.menus_num; i++)
		get_mascots_resources(top, i);

	adat.menu_no = (adat.random && adat.all_menu) 
		? rand() % adat.menus_num : adat.menu_no;
	{
		MascotMenu *m = &adat.mascot_menus[adat.menu_no];
		adat.mascot_number = (adat.random) ? rand() % m->n_mascots : adat.no;
	}

	/* サーチパス */
	search_path = XtMalloc(strlen(adat.search_path) 
						   + strlen(":" XMASDIR ":") + 1);
	strcat(strcpy(search_path, adat.search_path), ":" XMASDIR ":");
	set_search_path(search_path);

	msg_out("search_path:[%s]\n", search_path);

#ifdef BIFF
	/* execpv のためのパラメータ生成 */
	biff_filter[0] = NULL;
	if (adat.biff_filter != NULL) {
		int             t, i;
		char           *p;
		set_token(adat.biff_filter);
		if ((t = get_token2()) != NODAT) {
			biff_filter[0] = XtNewString(token_buf);
			if ((p = strrchr(biff_filter[0], '/')) != NULL)
				p++;
			else
				p = biff_filter[0];
			biff_filter[1] = p;
			for (i = 2; i < 12;) {
				if ((t = get_token2()) == NODAT) {
					biff_filter[i] = (char *) NULL;
					break;
				} else
					biff_filter[i++] = XtNewString(token_buf);
			}
		}
	}
#endif
}

void 
usage(int *argc, char **argv)
{
	static char    *usages =
	"\n  file: Mascot pattern filename [*.gif,*.bmp,*.mag,*.tif,*.pnm]\n"
	"  options:\n"
	"    -verbose        : Verbose mode\n"
	"    -gravity n      : Gravity(cm/s^2)\n"
	"    -chainlength n  : Chain length(mm)\n"
	"    -damping n      : Damping coefficient\n"
	"    -degree n       : Starting degree\n"
	"    -menuno n       : Starting menu Number\n"
	"    -no n           : Starting mascot Number\n"
	"    -magnify f      : Mascot magnify\n"
	"    -random         : Choise mascot random\n"
	"    -pinpat fname   : Pin pattern file [*.gif,*.bmp,*.mag,*.tif,*.pnm]\n"
	"    -random         : Select mascot random.\n"
	"    -changetime min : Change mascot periodic.\n"
	"    -allmenu        : Change mascot on all menu.\n"
	"    -chainnum n     : Chain number.\n"
	"    -drawtiming n   : draw pattern once per n * DT \n"
#ifdef USE_CHAINPAT
	"    -chainpat fname : Chain pattern file [*.gif,*.bmp,*.mag,*.tif,*.ppm]\n"
#endif
	"    -searchpath s   : Mascot"
#ifdef SOUND
	"/Sound"
#endif
	" search path\n"
#ifdef SOUND
	"    -soundcmd s     : Sound play command\n"
	"    -soundstart s   : Start sound filename\n"
	"    -soundclick s   : Click sound filename\n"
	"    -soundend   s   : End   sound filename\n"
#ifdef BIFF
	"    -soundmail  s   : Mail arrived sound filename\n"
#endif				/* BIFF */
#endif				/* SOUND */
#ifdef BIFF
	"    -nobiff         : Biff mode off\n"
	"    -update sec     : How often to check for mail\n"
	"    -noonce         : Do action when mail arrived always\n"
	"    -biffcmd cmd    : Command for checking mail arrived\n"
	"    -biffpat fname  : Biff pattern file [*.gif,*.bmp,*.mag,*.tif,*.pnm]\n"
	"    -biffpos just   : Justify of BIFF pattern (left,center,right)\n"
#ifdef BIFF_LIST
	"    -nobifflists    : Don't show reached mail lists\n"
	"    -biffgeometry   : Reached Mail list's Geometry\n"
	"    -bifffilter     : Filter command for Mail lists\n"
	"    -biffpopdown s  : seconds till popdown arrived mail lists\n"
#endif
#ifdef YOUBIN
	"    -noyoubin         : Youbin mode off\n"
	"    -server hostname  : Youbin server name\n"
#endif
#endif
#ifdef SHADOW
	"    -shadow n       : Set shadow length\n"
	"    -noshadow       : No shadow\n"
#endif
	"\n";
	int             i, f = 0;
	for (i = 1; i < *argc; i++) {
		if (argv[i][0] == '-') {
			if (!f++)
				err_out("%s: option not understood:", argv[0]);
			err_out(" %s", argv[i]);
		}
	}
	if (f) {
		err_out("\nusage: %s [options] [file]\n%s", argv[0], usages);
		exit(1);
	}
}

/* マスコットメニュー用リソース */
static XtResource mascot_menu_res[] = {
	{
		"title", "Title",
		XtRString, sizeof(String),
		XtOffsetOf(MascotMenu, title),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"numsOfMenu", "NumsOfMenu",
		XtRInt, sizeof(int),
		XtOffsetOf(MascotMenu, n_mascots),
		XtRImmediate, (XtPointer) 0
	},
};

/* マスコット用リソース */
static XtResource mascot_res[] = {
	{
		"title", "Title",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, title),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"filename", "Pattern",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, fname),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"col0", "Col0",
		XtRInt, sizeof(int),
		XtOffsetOf(Mascot, col0),
		XtRImmediate, (XtPointer) - 1
	},
	{
		"rgb0", "Rgb0",
		XtRRgb, sizeof(int),
		XtOffsetOf(Mascot, rgb0),
		XtRString, (XtPointer) "auto"
	},
	{
		"magnify", "Magnify",
		XtRFloat, sizeof(float),
		XtOffsetOf(Mascot, mmag),
		XtRString, (XtPointer) "1.0"
	},
#ifdef SOUND
	{
		"startSnd", "StartSnd",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, start_snd),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"clickSnd", "ClickSnd",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, click_snd),
		XtRImmediate, (XtPointer) NULL
	},
	{
		"endSnd", "EndSnd",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, end_snd),
		XtRImmediate, (XtPointer) NULL
	},
#ifdef BIFF
	{
		"mailSnd", "MailSnd",
		XtRString, sizeof(String),
		XtOffsetOf(Mascot, mail_snd),
		XtRImmediate, (XtPointer) NULL
	},
#endif				/* BIFF */
#endif				/* SOUND */
#ifdef BIFF
	{
		"biffPos", "BiffPos",
		XtRJustify, sizeof(XtJustify),
		XtOffsetOf(Mascot, biff_justify),
		XtRImmediate, (XtPointer) XtJustifyRight
	},
#endif
};

/* 各メニューのマスコットのリソース取得 */
void 
get_mascots_resources(Widget top, int n)
{
	int             i;
	char            name[20];
	MascotMenu     *menu;
	Mascot         *mas;
	Widget          w;

	sprintf(name, "menu%d", n);
	menu = &adat.mascot_menus[n];
	XtGetSubresources(top, menu, name, "Menu",
					  mascot_menu_res, XtNumber(mascot_menu_res), NULL, 0);
	menu->mascots = (Mascot *) XtMalloc(sizeof(Mascot) *
						(menu->n_mascots + ((n == 0) ? 1 : 0)));

	w = XtCreateWidget(name, objectClass, top, NULL, 0);
	msg_out("menu %d: %d mascots %s\n", n, menu->n_mascots, menu->title);
	for (i = 0; i < menu->n_mascots; i++) {
		mas = &(menu->mascots[i]);
		sprintf(name, "masDat%d", i);
		XtGetSubresources(w, mas, name, "MasDat",
				  mascot_res, XtNumber(mascot_res), NULL, 0);
		if (mas->title != NULL) {
			msg_out("mascot %d ", i);
			if (mas->rgb0 == -1)
				msg_out("trgb:[auto] ");
			else
				msg_out("trgb:[%06x] ", mas->rgb0);
			if (mas->col0 == -1)
				msg_out("tcol:[auto] ");
			else
				msg_out("tcol:[%d] ", mas->col0);
			msg_out("%s\n", mas->title);
		}
	}

	if (n == 0) {
		/* コマンドラインで指定したエントリ用 */
		mas = &(menu->mascots[i]);
		XtGetSubresources(w, mas, "masDatX", "MasDat",
				  mascot_res, XtNumber(mascot_res), NULL, 0);
		if (mas->title != NULL) {
			msg_out("mascot %d ", i);
			if (mas->rgb0 == -1)
				msg_out("trgb:[auto] ");
			else
				msg_out("trgb:[%06x] ", mas->rgb0);
			if (mas->col0 == -1)
				msg_out("tcol:[auto] ");
			else
				msg_out("tcol:[%d] ", mas->col0);
			msg_out("%s\n", mas->title);
		}
	}
	XtDestroyWidget(w);
}
