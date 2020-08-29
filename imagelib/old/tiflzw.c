/*
   XMascot Ver 2.5   image-lib
   Copyright(c) 1996 Go Watanabe     go@cclub.tutcc.tut.ac.jp
                     Tsuyoshi IIda   iida@cclub.tutcc.tut.ac.jp
*/

/* TIF LZW の解凍 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "image.h"

/* ビット単位の入出力 */
typedef struct {
	int             R;
	long            Rdata;
	char            bitmask[9];
	int             num;
}               BIT_IO;

static void 
bit_init(BIT_IO * bit)
{
	bit->R = 0;
	bit->Rdata = 0;
	bit->num = 0;
}

static void 
bit_create(BIT_IO * bit)
{
	bit->bitmask[0] = 0x00;
	bit->bitmask[1] = 0x01;
	bit->bitmask[2] = 0x03;
	bit->bitmask[3] = 0x07;
	bit->bitmask[4] = 0x0f;
	bit->bitmask[5] = 0x1f;
	bit->bitmask[6] = 0x3f;
	bit->bitmask[7] = 0x7f;
	bit->bitmask[8] = 0xff;
	bit_init(bit);
}

static int 
SHIFT(int a, int n)
{
	return (((n) > 0) ? (a) << (n) : (a) >> -(n));
}

static long 
bit_get(BIT_IO * bit, int bits, FILE * fp)
{
	long            t, ret;
	if (bit->Rdata < 0)
		return -1;
	t = bits - bit->R;
	ret = SHIFT(bit->Rdata, t) & (0xffffffff UL >> (32 - bits));
	for (; t > 0; t -= 8) {
		if (bit->Rdata >= 0) {
			bit->Rdata = fgetc(fp);
			bit->num++;
		}
		ret |= SHIFT(bit->Rdata & 0xff, t - 8);
	}
	bit->R = abs(t);
	return ret;
}


/* ハッシュ */

typedef struct {
	int             prev;
	char            data;
	int             next;
	int             len;
}               HASHTAB;

typedef struct {
	HASHTAB        *table;
	char           *databuf;
	int            *htab;
	int             tabnum;
}               HASH;

static int 
hash_hf(char *data, int len)
{
	int             i, h = 0;
	for (i = 0; i < len; i++)
		h = (h + ((128 + i + data[i]) & 0xff) * 253) % 257;
	return h;
}

static int 
hash_getdata(HASH * hash, char *buf, int index)
{
	int             l;
	if (index == -1)
		return 0;
	l = hash_getdata(hash, buf, hash->table[index].prev);
	buf[l] = hash->table[index].data;
	return l + 1;
}

static void 
hash_addsub(HASH * hash, int index, int prev, int data)
{
	int             l, h, p;
	l = hash_getdata(hash, hash->databuf, prev);
	hash->databuf[l] = data;
	l++;
	hash->table[index].prev = prev;
	hash->table[index].data = data;
	hash->table[index].next = -1;
	hash->table[index].len = l;
	h = hash_hf(hash->databuf, l);
	if (hash->htab[h] == -1)
		hash->htab[h] = index;
	else {
		for (p = hash->htab[h]; hash->table[p].next != -1; p = hash->table[p].next);
		hash->table[p].next = index;
	}
}

static int 
hash_add(HASH * hash, int prev, int data)
{
	if (hash->tabnum < 4096) {
		hash_addsub(hash, hash->tabnum, prev, data);
		hash->tabnum++;
	}
	return hash->tabnum - 1;
}

static void 
hash_init(HASH * hash)
{
	int             i;
	for (i = 0; i < 4096; i++) {
		hash->table[i].prev = -1;
		hash->table[i].next = -1;
		hash->table[i].len  = 0;
	}
	for (i = 0; i < 257; i++)
		hash->htab[i] = -1;
	for (i = 0; i < 256; i++)
		hash_addsub(hash, i, -1, i);
	hash->tabnum = 258;
}

static void 
hash_create(HASH * hash)
{
	hash->table   = (HASHTAB *)XtCalloc(sizeof(HASHTAB), 4096);
	hash->databuf = (char *)XtCalloc(sizeof(char), 4096);
	hash->htab    = (int *)XtCalloc(sizeof(int), 257);
	hash_init(hash);
}

static void 
hash_destroy(HASH * hash)
{
	XtFree((char *) hash->table);
	XtFree((char *) hash->databuf);
	XtFree((char *) hash->htab);
}

static int 
hash_getfirst(HASH * hash, int index)
{
	for (; hash->table[index].prev != -1; index = hash->table[index].prev);
	return hash->table[index].data;
}

static int 
hash_getmaxindex(HASH * hash)
{
	return hash->tabnum - 1;
}

/* LZW 展開 */

#define CLEAR           256
#define ENDCODE         257

#define OUTTABLE(idx) \
  { int i, len = hash_getdata(&h,databuf, (idx)); \
    for (i=0; i<len; i++)   *data++ = databuf[i]; }

int 
tiflzwdecode(u_char * data, FILE * fp)
{
	int             bitlen, nextcode, code;
	char            databuf[4096];
	HASH            h;
	BIT_IO          bit;
	bit_create(&bit);
	hash_create(&h);
	bitlen = 9;
	while ((nextcode = bit_get(&bit, bitlen, fp)) != ENDCODE && !feof(fp)) {
		if (nextcode == CLEAR) {
			hash_init(&h);
			bitlen = 9;
			if ((code = bit_get(&bit, bitlen, fp)) == ENDCODE)
				break;
			OUTTABLE(code);
		} else {
			int             idx;
			if (nextcode <= hash_getmaxindex(&h)) {
				OUTTABLE(nextcode);
				idx = hash_add(&h, code, hash_getfirst(&h, nextcode));
			} else {
				OUTTABLE(code);
				*data++ = hash_getfirst(&h, code);
				idx = hash_add(&h, code, hash_getfirst(&h, code));
			}
			if (idx == ((1 << bitlen) - 2))
				if (bitlen < 12)
					bitlen++;
			code = nextcode;
		}
	}
	hash_destroy(&h);
	return bit.num;
}
