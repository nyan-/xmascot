/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

#ifdef SOUND

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "xmascot.h"

extern AppData  adat;

/* 音声再生 (1つだけ) */
void 
sound_play(char *name)
{
	pid_t           pid;
	char            cmdline[1024];
	name = search(name);
	if (adat.snd_cmd != NULL && name != NULL) {
		sprintf(cmdline, adat.snd_cmd, name);
		if ((pid = fork()) == 0) {
			system(cmdline);
			exit(0);
		}
	}
}

/* 連続再生 */
void 
sounds_play(char **names, int num)
{
	int             i;
	pid_t           p;
	char            cmdline[1024];
	char           *name;

	if (adat.snd_cmd != NULL && num > 0) {
		if ((p = fork()) == 0) {
			for (i = 0; i < num; i++) {
				if ((name = search(names[i])) != NULL) {
					sprintf(cmdline, adat.snd_cmd, name);
					system(cmdline);
				}
			}
			exit(0);
		}
	}
}

void xmascot_sound(XMascotData *adat, SoundType num)
{
	switch(num) {
	case SOUND_START:
		sound_play(adat->mascot_menus[adat->menu_no].
				   mascots[adat->mascot_number].start_snd);
		break;
	case SOUND_END:
		sound_play(adat->mascot_menus[adat->menu_no].
				   mascots[adat->mascot_number].end_snd);
		break;
	case SOUND_CLICK:
		sound_play(adat->mascot_menus[adat->menu_no].
				   mascots[adat->mascot_number].click_snd);
		break;
#ifdef BIFF
	case SOUND_MAIL:
		sound_play(adat->mascot_menus[adat->menu_no].
				   mascots[adat->mascot_number].mail_snd);
		break;
#endif
	}
}


#endif
