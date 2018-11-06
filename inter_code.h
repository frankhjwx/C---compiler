#ifndef _INTER_CODE_
#define _INTER_CODE_

typedef struct Operand_* Operand;

struct Operand_ {
	enum { VARIABLE, TEMP, CONSTANT, RELOP, ADDRESS, POINTER, LABEL, FUNC } kind;
	union {
		int var_name;
		int value;
		char* relop;
		char* func_name;
	} u;
};

struct argl_ {
	Operand arg;
	struct argl_* next;
};

struct InterCode {
	enum { UNDEFINE, ASSIGN, PLUS, MINUS, STAR, DIV, RETURN, IF_GOTO, LABEL_CODE, LABEL_TRUE, LABEL_GOTO, READ, WRITE, CALLFUNC, FUNCTION, ARG, PARAM, DEC  } kind;
	union {
		struct { Operand right, left; } assign;
		struct { Operand result, op1, op2; } binop;
		struct { Operand l; } label;
		struct { Operand r; } ret;
		struct { Operand t1, t2, op, l; } if_goto;
		struct { Operand r; } func;
		struct { Operand result, f; } callfunc;
		struct { Operand w; } wr;
		struct { Operand op; } arg;
		struct { Operand p; } param;
		struct { Operand op1, op2; } dec;
	} u;
};

struct InterCodes {
	struct InterCode code;
	struct InterCodes *prev, *next;
};

struct Operand_* new_temp();
struct Operand_* new_label();
struct Operand_* new_func(char* s);
struct Operand_* opval(int x);
struct InterCodes* new_code();
struct InterCodes* BindCode(struct InterCodes* code1, struct InterCodes* code2);
struct InterCodes* new_arg(struct Operand_* arg);
struct InterCodes* translate_exp(struct Node* r, struct Operand_* place);
struct InterCodes* translate_stmt(struct Node* r);
struct InterCodes* translate_cond(struct Node* r, struct Operand_* label_true, struct Operand_* label_false);
struct InterCodes* translate_args(struct Node* r);
struct InterCodes* translate_program(struct Node* r);
struct InterCodes* translate_extdeflist(struct Node* r);
struct InterCodes* translate_extdef(struct Node* r);
struct InterCodes* translate_fundec(struct Node* r);
struct InterCodes* translate_varlist(struct Node* r);
struct InterCodes* translate_paramdec(struct Node* r);
struct InterCodes* translate_vardec(struct Node* r, int k);
struct InterCodes* translate_compst(struct Node* r);
struct InterCodes* translate_stmtlist(struct Node* r);
struct InterCodes* translate_deflist(struct Node* r);
struct InterCodes* translate_def(struct Node* r);
struct InterCodes* translate_declist(struct Node* r);
struct InterCodes* translate_dec(struct Node* r);

#endif
