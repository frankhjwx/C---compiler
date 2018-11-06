%{
#include <stdio.h>
#include "lex.yy.c"
#include "tree.h"
#include "analyzer.h"
#include "global.h"
#define NO yylloc.first_line

extern char* val_id;
%}

%union{
	struct Node* Tnode;
}
%token <Tnode> INT FLOAT ID
%token <Tnode> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV
%token <Tnode> AND OR DOT NOT TYPE
%token <Tnode> LP RP LB RB LC RC
%token <Tnode> STRUCT RETURN IF ELSE WHILE

%type <Tnode> Program ExtDefList ExtDef ExtDecList
%type <Tnode> Specifier StructSpecifier OptTag Tag
%type <Tnode> VarDec FunDec VarList ParamDec
%type <Tnode> CompSt StmtList Stmt
%type <Tnode> DefList Def DecList Dec
%type <Tnode> Exp Args


%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%right ASSIGNOP
%left OR AND
%left RELOP
%left PLUS MINUS
%left DIV STAR
%right NOT
%left LP RP LB RB DOT


%locations

%%
/* High-level Definitions */
Program : ExtDefList { $$ = createNode(@$.first_line,"Program",1,$1); root = $$;}
  ;
ExtDefList : ExtDef ExtDefList { $$ = createNode(@$.first_line,"ExtDefList",2,$1,$2); }
  | { $$ = createNode(@$.first_line,"NUL",0); }
  ;
ExtDef : Specifier ExtDecList SEMI { $3 = createNode(@3.first_line,"SEMI",0); $$ = createNode(@$.first_line,"ExtDef",3,$1,$2,$3); }
  | Specifier SEMI { $2 = createNode(@2.first_line,"SEMI",0); $$ = createNode(@$.first_line,"ExtDef",2,$1,$2); }
  | Specifier FunDec CompSt { $$ = createNode(@$.first_line,"ExtDef",3,$1,$2,$3); }
  | Specifier FunDec error { printf("Error type B at Line %d: Incomplete definition of function \"%s\".\n",yylineno,$2->children[0]->val_id); }
  ;
ExtDecList : VarDec { $$ = createNode(@$.first_line,"ExtDecList",1,$1); }
  | VarDec COMMA ExtDecList { $2 = createNode(@$.first_line,"COMMA",0); $$ = createNode(@$.first_line,"ExtDecList",3,$1,$2,$3); }
  ;

/* Specifiers */
Specifier : TYPE { val_id = $1->val_id; $1 = createNode(@$.first_line,"TYPE",0); $$ = createNode(@$.first_line,"Specifier",1,$1); }
  | StructSpecifier { $$ = createNode(@$.first_line,"Specifier",1,$1); }
  ;
StructSpecifier : STRUCT OptTag LC DefList RC { $1 = createNode(@1.first_line,"STRUCT",0); $3 = createNode(@3.first_line,"LC",0); $5 = createNode(@5.first_line,"RC",0); $$ = createNode(@$.first_line,"StructSpecifier",5,$1,$2,$3,$4,$5); }
  | STRUCT Tag { $1 = createNode(@1.first_line,"STRUCT",0); $$ = createNode(@$.first_line,"StructSpecifier",2,$1,$2); }
  ;
OptTag : { $$ = createNode(@$.first_line,"NUL",0); }
  | ID { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $$ = createNode(@$.first_line,"OptTag",1,$1); }
  ;
Tag : ID { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $$ = createNode(@$.first_line,"Tag",1,$1); }
  ;

/* Declarators */
VarDec : ID { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $$ = createNode(@$.first_line,"VarDec",1,$1); }
  | VarDec LB INT RB { $2 = createNode(@2.first_line,"LB",0); $3 = createNode(@3.first_line,"INT",0); $4 = createNode(@4.first_line,"RB",0); $$ = createNode(@$.first_line,"VarDec",4,$1,$2,$3,$4); }
  ;
FunDec : ID LP VarList RP { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $2 = createNode(@2.first_line,"LP",0); $4 = createNode(@4.first_line,"RP",0); $$ = createNode(@$.first_line,"FunDec",4,$1,$2,$3,$4); } 
  | ID LP RP { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $2 = createNode(@2.first_line,"LP",0); $3 = createNode(@3.first_line,"RP",0); $$ = createNode(@$.first_line,"FunDec",3,$1,$2,$3); }
  ;
VarList : ParamDec COMMA VarList { $2 = createNode(@2.first_line,"COMMA",0); $$ = createNode(@$.first_line,"VarList",3,$1,$2,$3); }
  | ParamDec { $$ = createNode(@$.first_line,"VarList",1,$1); }
  ;
ParamDec : Specifier VarDec { $$ = createNode(@$.first_line,"ParamDec",2,$1,$2); }
  ;

/* Statements */
CompSt : LC DefList StmtList RC { $1 = createNode(@1.first_line,"LC",0); $4 = createNode(@4.first_line,"RC",0); $$ = createNode(@$.first_line,"CompSt",4,$1,$2,$3,$4); }
  ;
StmtList : { $$ = createNode(@$.first_line,"StmtList",0); $$->type_terminal = 2; }
  | Stmt StmtList { $$ = createNode(@$.first_line,"StmtList",2,$1,$2); }
  ;
Stmt : Exp SEMI { $2 = createNode(@2.first_line,"SEMI",0); $$ = createNode(@$.first_line,"Stmt",2,$1,$2); }
  | CompSt { $$ = createNode(@$.first_line,"Stmt",1,$1); }
  | RETURN Exp SEMI { $1 = createNode(@1.first_line,"RETURN",0); $3 = createNode(@3.first_line,"SEMI",0); $$ = createNode(@$.first_line,"Stmt",3,$1,$2,$3); }
  | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $1 = createNode(@1.first_line,"IF",0); $2 = createNode(@2.first_line,"LP",0); $4 = createNode(@4.first_line,"RP",0); $$ = createNode(@$.first_line,"Stmt",5,$1,$2,$3,$4,$5); }
  | IF LP Exp RP Stmt ELSE Stmt { $1 = createNode(@1.first_line,"IF",0); $2 = createNode(@2.first_line,"LP",0); $4 = createNode(@4.first_line,"RP",0); $6 = createNode(@6.first_line,"ELSE",0); $$ = createNode(@$.first_line,"Stmt",7,$1,$2,$3,$4,$5,$6,$7); }
  | WHILE LP Exp RP Stmt { $1 = createNode(@1.first_line,"WHILE",0); $2 = createNode(@2.first_line,"LP",0); $4 =createNode(@4.first_line,"RP",0); $$ = createNode(@$.first_line,"Stmt",5,$1,$2,$3,$4,$5); }
  | Exp error { printf("Error type B at Line %d: Missing \";\".\n", yylineno); }
  | error SEMI { printf("Error type B at line %d: Syntax error!\n", yylineno); }
  ;

/* Local Definitions */
DefList : { $$ = createNode(@$.first_line,"DefList",0); $$->type_terminal = 2; }
  | Def DefList { $$ = createNode(@$.first_line,"DefList",2,$1,$2); $$->name = "DefList"; }
  ;
Def : Specifier DecList SEMI { $3 = createNode(@3.first_line,"SEMI",0); $$ = createNode(@$.first_line,"Def",3,$1,$2,$3); }
  ;
DecList : Dec { $$ = createNode(@$.first_line,"DecList",1,$1); }
  | Dec COMMA DecList { $2 = createNode(@2.first_line,"COMMA",0); $$ = createNode(@$.first_line,"DecList",3,$1,$2,$3); }
  ;
Dec : VarDec { $$ = createNode(@$.first_line,"Dec",1,$1); }
  | VarDec ASSIGNOP Exp { $2 = createNode(@2.first_line,"ASSIGNOP",0); $$ = createNode(@$.first_line,"Dec",3,$1,$2,$3); }
  ;

/* Expressions */
Exp : Exp ASSIGNOP Exp { $2 = createNode(@2.first_line,"ASSIGNOP",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp AND Exp { $2 = createNode(@2.first_line,"AND",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp OR Exp { $2 = createNode(@2.first_line,"OR",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp RELOP Exp { val_id = $2->val_id; $2 = createNode(@2.first_line,"RELOP",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp PLUS Exp { $2 = createNode(@2.first_line,"PLUS",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp MINUS Exp { $2 = createNode(@2.first_line,"MINUS",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp STAR Exp { $2 = createNode(@2.first_line,"STAR",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp DIV Exp { $2 = createNode(@2.first_line,"DIV",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | LP Exp RP { $1 = createNode(@1.first_line,"LP",0); $3 = createNode(@3.first_line,"RP",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | MINUS Exp { $1 = createNode(@1.first_line,"MINUS",0); $$ = createNode(@$.first_line,"Exp",2,$1,$2); }
  | NOT Exp { $1 = createNode(@1.first_line,"NOT",0); $$ = createNode(@$.first_line,"Exp",2,$1,$2); }
  | ID LP Args RP { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $2 = createNode(@2.first_line,"LP",0); $4 = createNode(@4.first_line,"RP",0); $$ = createNode(@$.first_line,"Exp",4,$1,$2,$3,$4); }
  | ID LP RP { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $2 = createNode(@2.first_line,"LP",0); $3 = createNode(@3.first_line,"RP",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | Exp LB Exp RB { $2 = createNode(@2.first_line,"LB",0); $4 = createNode(@4.first_line,"RB",0); $$ = createNode(@$.first_line,"Exp",4,$1,$2,$3,$4); }
  | Exp DOT ID { val_id = $3->val_id; $2 = createNode(@2.first_line,"DOT",0); $3 = createNode(@3.first_line,"ID",0); $$ = createNode(@$.first_line,"Exp",3,$1,$2,$3); }
  | ID { val_id = $1->val_id; $1 = createNode(@1.first_line,"ID",0); $$ = createNode(@$.first_line,"Exp",1,$1); }
  | INT { $1 = createNode(@1.first_line,"INT",0); $$ = createNode(@$.first_line,"Exp",1,$1); }
  | FLOAT { $1 = createNode(@1.first_line,"FLOAT",0); $$ = createNode(@$.first_line,"Exp",1,$1); }
  | Exp LB error RB{ printf("Error type B at Line %d: Missing \"]\".\n", yylineno); }
  ;
Args : Exp COMMA Args { $2 = createNode(@2.first_line,"COMMA",0); $$ = createNode(@$.first_line,"Args",3,$1,$2,$3); }
  | Exp { $$ = createNode(@$.first_line,"Args",1,$1); }
  ;

%%
yyerror(char* msg) {
//	fprintf(stderr, "Error type B at line %d: %s\n", yylineno, msg);
	if_error = 1;
}
