#ifndef _GLOBAL_
#define _GLOBAL_

extern struct Type_* nT[128];    // Types defined by struct
extern int Type_num;           // num of new types
extern struct Node* root;
extern struct symbol_table* S_table_r;
extern struct symbol_table* now;
extern int val_int;
extern float val_float;
extern char* val_id;
extern int valnum;
extern int if_error;
extern int yylineno;
extern int tempnum;
extern int labelnum;
extern FILE* fp2;

#endif
