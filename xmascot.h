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

#define DT 100				/* ���ߥ�졼�������� */

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

#define ALARM_NUM 3							/* ���顼��ο�     */
#define ALARM_ALLNUM (ALARM_NUM+4)			/* ���顼���Ϣ��� */
#define XtRRgb	"Rgb"

/* �ޥ����åȹ�¤�� */
typedef struct{
	Widget entry;	
	String title;		/* �ޥ����å�̾�� */
	String fname;		/* �ե�����̾     */
	int    col0;		/* Ʃ����(pixel)  */
	int    rgb0;		/* Ʃ����(rgb)    */
	float  mmag;		/* ����Ψ */
#ifdef SOUND
	String start_snd;	/* ��ư�������   */
	String click_snd;   /* ����å�������� */
	String end_snd;		/* ��λ�������   */
#ifdef BIFF
	String mail_snd;	/* �᡼���忮������� */
#endif
#endif
#ifdef BIFF
	XtJustify biff_justify;	/* �᡼������ޡ������� */
#endif
} Mascot;

/* �ޥ����åȥ�˥塼��¤�� */
typedef struct{
	String title;		/* ��˥塼̾��     */
	int n_mascots;		/* �ޥ����å����   */
	Mascot *mascots;	/* �ޥ����åȹ�¤�� */
} MascotMenu;

/* ���顼�๽¤�� */
typedef struct{
	int sw;				/* ���顼���ͭ������ */
	int hour;			/* ���顼�� ����      */
	int min;			/*          ʬ        */
	String action;		/* ���顼�ॢ������� */
	XtIntervalId id;	/* ������ ID        */
} Alarm;

/* XMascot ���ܹ�¤�� */
typedef struct{
	Boolean	verbose;		/* �ܺ�ɽ��   				*/
	int 	grav;			/* �������   				*/
	int 	chain_len;		/* ����Ĺ��   				*/
	float 	f_damping;		/* ���그��   				*/
	double	damping;
	float	magnify;		/* ����Ψ					*/
	String 	search_path; 	/* �������ѥ� 				*/
	String	pin_pat;		/* �ԥ�Υѥ�����ե�����   */
	int		pcol0;			/* �ԥ��Ʃ��������ǥå��� */
	int		prgb0;			/* �ԥ��Ʃ���� RGB         */
	int		chain_num;		/* ���ο� 					*/
	int		draw_timing;	/* �������� 				*/
#ifdef USE_CHAINPAT
	String	chain_pat;		/* ɳ(��)�Υѥ�����ե�����     */
	int		ccol0;			/* ɳ(��)��Ʃ��������ǥå���   */
	int		crgb0;			/* ɳ(��)��Ʃ���� RGB           */
#endif
	int		menus_num;		/* ��˥塼����� */
	int     menu_no;		/* ��ư����˥塼�ֹ�   */
	int 	no;				/* ��ư���ޥ����å��ֹ� */
	String	def_act;		/* �ǥե���ȤΥ������������ */
	int     th;				/* �������  */
	Boolean random;			/* ����饯���Υ������ѹ� */
	int		change_time;	/* ���ؤޤǤλ��� (ʬ����)  */
	Boolean all_menu;		/* ���ؤ�����˥塼         */
#ifdef BIFF
	Boolean biff_mode;		/* biff ��ͭ������   */
	String  biff_action;	/* biff �Υ�������� */
	int	    biff_update;	/* biff �����ֳ�     */
	Boolean biff_once;  	/* ���������������򵯤��� */
	String  biff_cmd;		/* �ᥤ����������å����ޥ�� */
	String	biff_pat;		/* biff ����ޡ����ѥ�����ե�����           */
	String  biff_filter;	/* biff �᡼�����ɽ���ѥե��륿             */
	int     biff_popdown;	/* biff ������᡼������ξä���ޤǤλ���   */
	int		bcol0;			/* biff ����ޡ����ѥ�����Ʃ��������ǥå��� */
	int		brgb0;			/* biff ����ޡ����ѥ�����Ʃ���� RGB         */
#ifdef YOUBIN
	char *server;			/* youbin server �ۥ���̾ */
	int   youbin;			/* youbin �⡼�ɤ��ɤ���  */
#endif
#endif
#ifdef SOUND
	String snd_cmd;			/* ���������ѥ��ޥ�� */
#endif
#ifdef SHADOW
	int		shadow;			/* �Ƥ��� */
#endif
	Cursor cursor_normal;
	Cursor cursor_click;
	Cursor cursor_drag;

/* XMascot �ǡ����� */
	MascotMenu *mascot_menus;	/* �ޥ����åȥ�˥塼��¤�� */
	int      mascot_number;		/* ������Υޥ����å�		 */
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

