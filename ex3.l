%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>	
#include "ex3.tab.h"
#include "global.h"
#include "tree.h"


int yycolumn = 1;
char* val_id;
int val_int;
float val_float;
int if_error;
#define YY_USER_ACTION \
  yylloc.first_line = yylloc.last_line = yylineno; \
  yylloc.first_column = yycolumn; \
  yylloc.last_column = yycolumn + yyleng - 1; \
  yycolumn += yyleng;
%}

%option yylineno

OCT_INT 0[0-7]+
ILLE_OCT_INT 0[0-9]+
HEX_INT 0[xX][0-9a-fA-F]+
ILLE_HEX_INT 0x[0-9a-zA-Z]+
INT 0|[1-9][0-9]*
FLOAT [0-9]*[\.]?[0-9]*[eE][+-]?[0-9]+|[0-9]+\.[0-9]+
ILLE_FLOAT [0-9]*[\.]?[0-9]*[eE]
TYPE int|float
ID [a-zA-Z_][a-zA-Z0-9_]*
SEMI [;]
COMMA \,
ASSIGNOP [=]
RELOP \>=|\<=|==|\!=|\>|\<
PLUS \+
MINUS -
STAR \*
DIV \/
AND &&
OR \|\|
DOT \.
NOT \!
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF "if"
ELSE "else"
WHILE while
SPACE [ ]
TAB \t
NL \n
LINECOMMENT "//"
BLOCKCOMMENTL "/*"
BLOCKCOMMENTR "*/"
%%
{PLUS} { return PLUS; }
{MINUS} { return MINUS; }
{STAR} { return STAR; }
{DIV} { return DIV; }
{OCT_INT} {
	sscanf(yytext + 1, "%o", &val_int);
    return INT;
}
{HEX_INT} {
	sscanf(yytext + 2, "%x", &val_int);
	return INT;
}
{INT} {
	val_int = atoi(yytext);
	return INT;
}
{ILLE_OCT_INT} {
	printf("Error type A at line %d: Illegal octal number \'%s\'.\n", yylineno, yytext);
	if_error = 1;
	return INT;
}
{ILLE_HEX_INT} {
	printf("Error type A at line %d: Illegal hexadecimal number \'%s\'.\n", yylineno, yytext);
	if_error = 1;
	return INT;
}
{SEMI} { return SEMI; }
{COMMA} { return COMMA; }
{ASSIGNOP} { return ASSIGNOP; }
{RELOP} {
	char* s = (char*)malloc(strlen(yytext) + 1);
	strcpy(s, yytext);
	struct Node* newT = (struct Node*)malloc(sizeof(struct Node));
	yylval.Tnode = newT;
	yylval.Tnode->val_id = s;
	return RELOP; }
{DOT} { return DOT;}
{LP} { return LP; }
{RP} { return RP; }
{LB} { return LB; }
{RB} { return RB; }
{LC} { return LC; }
{RC} { return RC; }
{TYPE} {
	char* s = (char*)malloc(strlen(yytext) + 1);
	strcpy(s, yytext);
	struct Node* newT = (struct Node*)malloc(sizeof(struct Node));
	yylval.Tnode = newT;
	yylval.Tnode->val_id = s;
	return TYPE; 
}
{AND} { return AND; }
{OR} { return OR; }
{NOT} { return NOT; }
{FLOAT} {
	sscanf(yytext, "%f", &val_float);
	return FLOAT;
}
{ILLE_FLOAT} {
	printf("Error type A at line %d: Illegal float number \'%s\'.\n", yylineno, yytext);
	if_error = 1;
}
{STRUCT} { return STRUCT; }
{RETURN} { return RETURN; }
{IF} { return IF; }
{ELSE} { return ELSE; }
{WHILE} { return WHILE; }
{ID} { 
	char* s = (char*)malloc(strlen(yytext) + 1);  
	strcpy(s, yytext);
	struct Node* newT = (struct Node*)malloc(sizeof(struct Node));
	yylval.Tnode = newT;
	yylval.Tnode->val_id = s;
	return ID; }
{SPACE} {}
{TAB} {}
{NL} { yycolumn = 1; } 
. {
	printf("Error type A at line %d: Mysterious character \'%s\'.\n", yylineno, yytext);
	if_error = 1;
}
{LINECOMMENT} {
	char c = input();
	while (c != '\n') c = input();
}
{BLOCKCOMMENTL} {
	char c1,c2;
	c1 = input();
	do {
		c2 = c1;
		c1 = input();
	} while ((c1 != EOF) && (!(c1 == '/' && c2 == '*')));
	if (c1 == EOF) {
		printf("Error type A at line %d: Missing Block Comment '*/'.\n", yylineno);
		if_error = 1;
	}
}
{BLOCKCOMMENTR} {
	printf("Error type B at line %d: Syntax error.\n", yylineno);
	if_error = 1;
}
%%
