%{
#include <string.h>

#define yytext animtext
#define yyleng animleng
#define yylval animlval

#include "anim_y.h"

extern int anim_lineno;
%}
ws			[ \t]+
qstring		\"[^\"\n]*[\"\n]
number		[0-9]*
id			[_a-zA-Z][_a-zA-Z0-9]*
label		[_a-zA-Z][_a-zA-Z0-9]*\:
nl			\n
%%
{ws} ;
{qstring} { 
	yylval.string = strdup(yytext+1);
	if (yylval.string[yyleng-2] != '"')
		fprintf(stderr,"warn: unterminated stding");
	else
		yylval.string[yyleng-2] = '\0';
	return QSTRING;
}
from		{return FROM;}
size		{return SIZE;}
to			{return TO;}
mask		{return MASK;}
loop		{return LOOP;}
{number}	{yylval.string = strdup(yytext);return NUMBER;}
{id}		{yylval.string = strdup(yytext);return ID;}
{label}		{
	yylval.string = strdup(yytext);
	yylval.string[yyleng-1] = '\0';
	return LABEL;
}		 
{nl}		{anim_lineno++;}
\{			{return START;}
\}			{return END;}
.			{return yytext[0];}
%%
