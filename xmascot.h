/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#ifndef __XMASCOT
#define __XMASCOT

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Label.h>

#include "image.h"

#define	I2P(n)	((void *)(uintptr_t)(n))

#define CHAIN_SIZE 8

#define DT 100				/* シミュレーション刻幅 */

#ifdef USE_DOUBLE
#define ANGLE_PI (M_PI)
#else
/* isin.c */
void isin_init(void);
int isin(int deg);
int icos(int deg);
#define ANGLE_PI (M_PI*256)
#endif

#define ROLL_DEG  (ANGLE_PI/32)
#define RAD       (M_PI/ANGLE_PI)

#define ALARM_NUM 3							/* アラームの数     */
#define ALARM_ALLNUM (ALARM_NUM+4)			/* アラーム関連総数 */
#define XtRRgb	"Rgb"

/* マスコット構造体 */
typedef struct{
	Widget entry;	
	String title;		/* マスコット名称 */
	String fname;		/* ファイル名     */
	int    col0;		/* 透明色(pixel)  */
	int    rgb0;		/* 透明色(rgb)    */
	float  mmag;		/* 拡大率 */
#ifdef SOUND
	String start_snd;	/* 起動サウンド   */
	String click_snd;   /* クリックサウンド */
	String end_snd;		/* 終了サウンド   */
#ifdef BIFF
	String mail_snd;	/* メール着信サウンド */
#endif
#endif
#ifdef BIFF
	XtJustify biff_justify;	/* メール到着マーク位置 */
#endif
} Mascot;

/* マスコットメニュー構造体 */
typedef struct{
	String title;		/* メニュー名称     */
	int n_mascots;		/* マスコット総数   */
	Mascot *mascots;	/* マスコット構造体 */
} MascotMenu;

/* アラーム構造体 */
typedef struct{
	int sw;				/* アラームは有効か？ */
	int hour;			/* アラーム 時間      */
	int min;			/*          分        */
	String action;		/* アラームアクション */
	XtIntervalId id;	/* 割り込み ID        */
} Alarm;

/* XMascot 基本構造体 */
typedef struct{
	Boolean	verbose;		/* 詳細表示   				*/
	int 	grav;			/* 重力定数   				*/
	int 	chain_len;		/* 鎖の長さ   				*/
	float 	f_damping;		/* 減衰係数   				*/
	double	damping;
	float	magnify;		/* 拡大率					*/
	String 	search_path; 	/* サーチパス 				*/
	String	pin_pat;		/* ピンのパターンファイル   */
	int		pcol0;			/* ピンの透明色インデックス */
	int		prgb0;			/* ピンの透明色 RGB         */
	int		chain_num;		/* 鎖の数 					*/
	int		draw_timing;	/* 描画頻度 				*/
#ifdef USE_CHAINPAT
	String	chain_pat;		/* 紐(鎖)のパターンファイル     */
	int		ccol0;			/* 紐(鎖)の透明色インデックス   */
	int		crgb0;			/* 紐(鎖)の透明色 RGB           */
#endif
	int		menus_num;		/* メニューの総数 */
	int     menu_no;		/* 起動時メニュー番号   */
	int 	no;				/* 起動時マスコット番号 */
	String	def_act;		/* デフォルトのアクション設定 */
	int     th;				/* 初期角度  */
	Boolean random;			/* キャラクタのランダム変更 */
	int		change_time;	/* 切替までの時間 (分指定)  */
	Boolean all_menu;		/* 切替え全メニュー         */
#ifdef BIFF
	Boolean biff_mode;		/* biff は有効か？   */
	String  biff_action;	/* biff のアクション */
	int	    biff_update;	/* biff 更新間隔     */
	Boolean biff_once;  	/* 一回だけアクションを起こす */
	String  biff_cmd;		/* メイル到着チェックコマンド */
	String	biff_pat;		/* biff 到着マークパターンファイル           */
	String  biff_filter;	/* biff メール一覧表示用フィルタ             */
	int     biff_popdown;	/* biff の到着メール一覧の消えるまでの時間   */
	int		bcol0;			/* biff 到着マークパターン透明色インデックス */
	int		brgb0;			/* biff 到着マークパターン透明色 RGB         */
#ifdef YOUBIN
	char *server;			/* youbin server ホスト名 */
	int   youbin;			/* youbin モードかどうか  */
#endif
#endif
#ifdef SOUND
	String snd_cmd;			/* 音声再生用コマンド */
#endif
#ifdef SHADOW
	int		shadow;			/* 影の幅 */
#endif
	Cursor cursor_normal;
	Cursor cursor_click;
	Cursor cursor_drag;

/* XMascot データ群 */
	MascotMenu *mascot_menus;	/* マスコットメニュー構造体 */
	int      mascot_number;		/* 選択中のマスコット		 */
	int		 chain_disp_num;


} XMascotData, AppData, *AppDataPtr;

typedef enum {
	SOUND_START, SOUND_END, SOUND_CLICK, SOUND_MAIL
} SoundType;

/* menu.c */

void menu_add_line(Widget menu);
Widget menu_add_cascade(Widget menu,char *title,Widget cascade);
Widget menu_add_dialog(Widget menu,char *title,Widget dialog);
Widget menu_add_callback(Widget menu,char *title,XtCallbackProc callback);

Widget change_menu(Widget top,int n);
Widget preference_dialog(Widget top);
Widget alarm_dialog(Widget top);
Widget about_dialog(Widget top);
void set_new_string(String *s, String new);

/* resource.c */
void set_sim_param(void);
void get_resources(Widget top);
void usage(int *argc, char **argv);

/* etc.c */
#include "etc.h"

/* pattern.c */
void set_mas(XMascotData *adat);
void set_widget_pattern(Widget w,char *name,int c,int r);
void set_chain_pat(char *name, int c, int r);

/* timer.c */
void start_timer(void);
void stop_timer(void);
void restart_timer(void);

/* action.c */

void change_mascot(Widget w,XtPointer dat,XtPointer call);
void change_mascot_with_sound(Widget w,XtPointer dat,XtPointer call);

void RaiseAll(void);
void ExitApp(void);
void Quit(Widget w,XEvent *e,String *p,Cardinal *n);
void QuitMsg(Widget w,XEvent *e,String *p,Cardinal *n);
void MapWin(Widget w,XEvent *e,String *p,Cardinal *n);
void UnMapWin(Widget w,XEvent *e,String *p,Cardinal *n);
void ConfigWin(Widget w,XEvent *e,String *p,Cardinal *n);
void Press(Widget w,XEvent *e,String *p,Cardinal *n);
void ReleasePin(Widget w,XEvent *e,String *p,Cardinal *n);
void ReleaseMascot(Widget w,XEvent *e,String *p,Cardinal *n);
void MotionPin(Widget w,XEvent *e,String *p,Cardinal *n);
void MotionMascot(Widget w,XEvent *e,String *p,Cardinal *n);

void Sound(Widget w,XEvent *e,String *p,Cardinal *n);
void SoundStart(Widget w,XEvent *e,String *p,Cardinal *n);
void SoundClick(Widget w,XEvent *e,String *p,Cardinal *n);
void SoundEnd(Widget w,XEvent *e,String *p,Cardinal *n);
void SoundMail(Widget w,XEvent *e,String *p,Cardinal *n);

void ChangeMascot(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeMascotFile(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeMascotNext(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeMascotNextAll(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeMascotRandom(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeMascotRandomAll(Widget w,XEvent *e,String *p,Cardinal *n);

void StartMove(Widget w,XEvent *e,String *p,Cardinal *);
void System(Widget w,XEvent *e,String *p,Cardinal *n);
void ChangeParam(Widget w,XEvent *e,String *p,Cardinal *n);
void Bell(Widget w,XEvent *e,String *p,Cardinal *n);

/* sim.c */
void reset_pos(void);
void set_pos(void);
void sim(void);

#ifdef SOUND
/* sound.c */
void sound_play(char *name);
void sounds_play(char **names,int num);
void xmascot_sound(XMascotData *adat, SoundType num);
#endif

#ifdef BIFF
/* biff.c */

void set_biff(void);

void ShowBiffNotice(Widget w,XEvent *e,String *p,Cardinal *n);
void BiffEnter(Widget w,XEvent *e,String *p,Cardinal *n);

#endif

/* alarm.c */
void set_alarms(void);

/* file.c */
void get_rcfile(void);
void put_rcfile(void);
void action_parse(char *action);

typedef enum {
	NODAT, STRING, ID, SYMBOL, RESERVE
} TokenType;

void set_token(char *p);
int get_token2(void);

/* main.c */
void create_chains(int num);

#endif

