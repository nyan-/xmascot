/*
   XMascot Ver 2.6
   Copyright(c) 1996,1997 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                          Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

/* 整数の sin/cos 関数 */

#ifndef USE_DOUBLE

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "xmascot.h"

int            *sin_tbl;

/* テーブル初期化 */
void 
isin_init(void)
{
	int             i;
	sin_tbl = (int *) XtMalloc((ANGLE_PI / 2 + 1) * sizeof(int));
	for (i = 0; i <= ANGLE_PI / 2; i++)
		sin_tbl[i] = sin(i * RAD) * 256;
}

int 
isin(int rad)
{
	if (rad < 0)
		return -isin(-rad);
	else if (rad > ANGLE_PI)
		return -isin(ANGLE_PI * 2 - rad);
	else if (rad > ANGLE_PI / 2)
		return sin_tbl[(int) (ANGLE_PI - rad)];
	else
		return sin_tbl[rad];
}

int 
icos(int rad)
{
	return isin(ANGLE_PI / 2 - rad);
}


#endif
