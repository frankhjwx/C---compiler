#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tree.h"
#include "global.h"
#include "inter_code.h"
#include "analyzer.h"


int tempnum, labelnum;
struct argl_* arg_list;

struct Operand_* new_temp(){
	struct Operand_* tmp;
	tempnum++;
	tmp = (struct Operand_*)malloc(sizeof(struct Operand_));
	tmp->kind = TEMP;
	tmp->u.var_name = tempnum;
	return tmp;
}

struct Operand_* new_label(){
	struct Operand_* tmp = (struct Operand_*)malloc(sizeof(struct Operand_));
	labelnum++;
	tmp->kind = LABEL;
	tmp->u.var_name = labelnum;
	return tmp;
}

struct Operand_* new_func(char* s) {
	struct Operand_* tmp = (struct Operand_*)malloc(sizeof(struct Operand_));
	tmp->kind = FUNC;
	tmp->u.func_name = s;
	return tmp;
}

struct Operand_* opval(int x){
	struct Operand_* tmp = (struct Operand_*)malloc(sizeof(struct Operand_));
	tmp->kind = CONSTANT;
	tmp->u.value = x;
	return tmp;
}

struct InterCodes* new_code(){
	struct InterCodes* tmp = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	return tmp;
}

struct InterCodes* bindCode(struct InterCodes* code1, struct InterCodes* code2) {
	struct InterCodes* code = code1;
	while (code->next != NULL) {
		code = code->next;
	}
	code2->prev = code;
	code->next = code2;
	return code1;
}

int num_args(struct argl_* arg_list) {
	int n = 0;
	struct argl_* pt = arg_list;
	while (pt!=NULL) {
		pt = pt->next;
		n++;
	}
	return n;
}

struct InterCodes* new_arg(struct Operand_* arg) {
	struct InterCodes* tmpcode = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	tmpcode->code.kind = ARG;
	tmpcode->code.u.arg.op = arg;
	return tmpcode;
}

struct InterCodes* translate_exp(struct Node* r,struct Operand_* place) {
	float vfloat;
	struct InterCodes *tmp, *code1, *code2, *code3, *code4;
	struct Operand_ *t1, *t2, *t3, *t4, *variable, *vint, *label1, *label2;
	int i;
//	for (i=0;i<r->arity;i++) {
//		printf("%s ",r->children[i]->name);
//	}
//	printf("\n");
	tmp = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code4 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	if (strcmp(r->children[0]->name,"INT") == 0) {     // Exp -> INT
		vint = opval(r->children[0]->val_int);
		tmp->code.kind = ASSIGN;
		tmp->code.u.assign.left = place;
		tmp->code.u.assign.right = vint;
		return tmp;
	}
	if ((r->arity == 1) && (strcmp(r->children[0]->name,"ID") == 0)) {
		variable = lookup(r->children[0]->val_id);
		tmp->code.kind = ASSIGN;
		tmp->code.u.assign.left = place;
		tmp->code.u.assign.right = variable;
		return tmp;
	}
    if ((r->arity == 3) && (strcmp(r->children[1]->name,"ASSIGNOP") == 0)) {
		if (strcmp(r->children[0]->children[0]->name,"ID") == 0) {
			variable = lookup(r->children[0]->children[0]->val_id);		
			t1 = new_temp();
			t2 = new_temp();
			code1 = translate_exp(r->children[2],t1);
		    code2->code.kind = ASSIGN;
			code2->code.u.assign.left = variable;
			code2->code.u.assign.right = t1;
	//		code3->code.kind = ASSIGN;
	//		code3->code.u.assign.left = place;
	//		code3->code.u.assign.right = variable;
			return bindCode(code1,code2);
		} else { // EXP -> EXP LB EXP RB
			variable = lookup(r->children[0]->children[0]->children[0]->val_id);
			t1 = new_temp();
			t2 = new_temp();
			t3 = new_temp();
			t4 = new_temp();
			struct Operand_ *t3pointer = new_temp();
			code1 = translate_exp(r->children[0]->children[2],t1);
			code2->code.kind = STAR;
			code2->code.u.binop.result = t2;
			code2->code.u.binop.op1 = t1;
			code2->code.u.binop.op2 = opval(4);
			code3->code.kind = PLUS;
			code3->code.u.binop.result = t3;
			code3->code.u.binop.op1 = variable;
			code3->code.u.binop.op1->kind = ADDRESS;
			code3->code.u.binop.op2 = t2;
			
			t3pointer->kind = POINTER;
			t3pointer->u.var_name = t3->u.var_name;

			code4 = bindCode(bindCode(code1,code2),code3);
			code1 = new_code(); code2 = new_code();
			code1 = translate_exp(r->children[2],t4);
			code2->code.kind = ASSIGN;
			code2->code.u.assign.left = t3pointer;
			code2->code.u.assign.left->kind = POINTER;
			code2->code.u.assign.right = t4;
			return bindCode(code4,bindCode(code1,code2));
		}
	}
	if ((r->arity == 3) && (strcmp(r->children[1]->name,"PLUS") == 0)) {  // Exp -> Exp PLUS Exp
		t1 = new_temp();
		t2 = new_temp();
		code1 = translate_exp(r->children[0],t1);
		code2 = translate_exp(r->children[2],t2);
		code3->code.kind = PLUS;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = t1;
		code3->code.u.binop.op2 = t2;
		return bindCode(bindCode(code1,code2),code3);
	}
	if ((r->arity == 3) && (strcmp(r->children[1]->name,"MINUS") == 0)) {  // Exp -> Exp MINUS Exp
		t1 = new_temp();
		t2 = new_temp();
		code1 = translate_exp(r->children[0],t1);
		code2 = translate_exp(r->children[2],t2);
		code3->code.kind = MINUS;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = t1;
		code3->code.u.binop.op2 = t2;
		return bindCode(bindCode(code1,code2),code3);
	}
	if ((r->arity == 3) && (strcmp(r->children[1]->name,"STAR") == 0)) {  // Exp -> Exp STAR Exp
		t1 = new_temp();
		t2 = new_temp();
		code1 = translate_exp(r->children[0],t1);
		code2 = translate_exp(r->children[2],t2);
		code3->code.kind = STAR;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = t1;
		code3->code.u.binop.op2 = t2;
		return bindCode(bindCode(code1,code2),code3);
	}
    if ((r->arity == 3) && (strcmp(r->children[1]->name,"DIV") == 0)) {  // Exp -> Exp DIV Exp
		t1 = new_temp();
		t2 = new_temp();
		code1 = translate_exp(r->children[0],t1);
		code2 = translate_exp(r->children[2],t2);
		code3->code.kind = DIV;
		code3->code.u.binop.result = place;
		code3->code.u.binop.op1 = t1;
		code3->code.u.binop.op2 = t2;
		return bindCode(bindCode(code1,code2),code3);
	}
	if ((r->arity == 2) && (strcmp(r->children[0]->name,"MINUS") == 0)) {  // Exp -> MINUS Exp
		t1 = new_temp();
		code1 = translate_exp(r->children[1],t1);
		code2->code.kind = MINUS;
		code2->code.u.binop.result = place;
		code2->code.u.binop.op1 = opval(0);
		code2->code.u.binop.op2 = t1;
		return bindCode(code1,code2);
	}
	if (((r->arity == 3) && (strcmp(r->children[1]->name,"RELOP") == 0)) ||
		((r->arity == 2) && (strcmp(r->children[0]->name,"NOT") == 0)) ||
		((r->arity == 3) && (strcmp(r->children[1]->name,"AND") == 0)) ||
		((r->arity == 3) && (strcmp(r->children[1]->name,"OR") == 0))) {
		label1 = new_label();
		label2 = new_label();
		code1->code.kind = ASSIGN;
		code1->code.u.assign.left = place;
		code1->code.u.assign.right = opval(0);
		code2 = translate_cond(r,label1,label2);
		code3->code.kind = LABEL_CODE;
		code3->code.u.label.l = label1;
		code4->code.kind = ASSIGN;
		code4->code.u.assign.left = place;
		code4->code.u.assign.right = opval(1);
		return bindCode(bindCode(bindCode(code1,code2),code3),code4);
	}
	if ((r->arity == 3) && (strcmp(r->children[0]->name,"ID") == 0)) { // Exp -> ID LP RP
		char* function = r->children[0]->val_id;
		if (strcmp(function,"read") == 0) {
			code1->code.kind = READ;
			code1->code.u.func.r = place;
			return code1;
		}
		code1->code.kind = CALLFUNC;
		code1->code.u.callfunc.result = place;
		code1->code.u.callfunc.f = new_func(function);
	}
	if ((r->arity == 4) && (strcmp(r->children[2]->name,"Args") == 0)) { // Exp -> ID LP Args RP
		char* function = r->children[0]->val_id;
//		struct argl_* arg_list = (struct argl_*)malloc(sizeof(struct argl_));
		arg_list = NULL;
		code1 = translate_args(r->children[2]);
		if (strcmp(function,"write") == 0) {
			code2->code.kind = WRITE;
			code2->code.u.wr.w = arg_list->arg;
			return bindCode(code1,code2);
		}
		struct argl_* pt = arg_list;
		for (i=1; i<=num_args(arg_list); i++) {
			code2 = bindCode(code2,new_arg(pt->arg));
			pt = pt->next;
		}
		code3->code.kind = CALLFUNC;
		code3->code.u.callfunc.result = place;
		code3->code.u.callfunc.f = new_func(function);
		return bindCode(bindCode(code1,code2),code3);
	}
	if ((r->arity == 3) && (strcmp(r->children[0]->name,"LP") == 0)) { // Exp -> LP Exp RP
		return translate_exp(r->children[1],place);
	}
	if ((r->arity == 4) && (strcmp(r->children[1]->name,"LB") == 0)) { // Exp -> Exp LB Exp RP
		variable = lookup(r->children[0]->children[0]->val_id);
		t1 = new_temp();
		t2 = new_temp();
		t3 = new_temp();
		code1 = translate_exp(r->children[2], t1);
		code2->code.kind = STAR;
		code2->code.u.binop.result = t2;
		code2->code.u.binop.op1 = t1;
		code2->code.u.binop.op2 = opval(4);
		code3->code.kind = PLUS;


		code3->code.u.binop.result = t3;
		code3->code.u.binop.op1 = variable;
		code3->code.u.binop.op1->kind = ADDRESS;
		code3->code.u.binop.op2 = t2;

		struct Operand_ *t3pointer = new_temp();
		t3pointer->kind = POINTER;
		t3pointer->u.var_name = t3->u.var_name;

		code4->code.kind = ASSIGN;
		code4->code.u.assign.left = place;
		code4->code.u.assign.right = t3pointer;
		code4->code.u.assign.right->kind = POINTER;
		return bindCode(bindCode(bindCode(code1,code2),code3),code4);
	}

	return new_code();
	
}

struct InterCodes* translate_stmt(struct Node* r) {
	struct InterCodes *tmp, *code1, *code2, *code3, *code4, *labelcode1, *labelcode2, *labelcode3, *labelgoto;
	struct Operand_ *t1, *t2, *label1, *label2, *label3;

	tmp = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code4 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelcode1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelcode2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelcode3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelgoto = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	if (r->arity == 2) {    // Stmt -> Exp SEMI
		return translate_exp(r->children[0], NULL);
	}
	if (r->arity == 1) {    // Stmt -> CompSt
		return translate_compst(r->children[0]);
	}
	if (r->arity == 3) {    // Stmt -> RETURN Exp SEMI
		t1 = new_temp();
		code1 = translate_exp(r->children[1], t1);
		code2->code.kind = RETURN;
		code2->code.u.ret.r = t1;
		return bindCode(code1,code2);
	}
	if ((r->arity == 5) && (strcmp(r->children[0]->name,"IF") == 0)) {  // Stmt -> IF LP Exp RP Stmt
		label1 = new_label();
		label2 = new_label();
		code1 = translate_cond(r->children[2],label1,label2);
		code2 = translate_stmt(r->children[4]);
		labelcode1->code.kind = LABEL_CODE;
		labelcode1->code.u.label.l = label1;
		labelcode2->code.kind = LABEL_CODE;
		labelcode2->code.u.label.l = label2;
		return bindCode(bindCode(bindCode(code1,labelcode1),code2),labelcode2);
    } 
	if (r->arity == 7) { // Stmt -> IF LP Exp RP Stmt ELSE Stmt
		label1 = new_label();
		label2 = new_label();
		label3 = new_label();
		code1 = translate_cond(r->children[2],label1,label2);
		code2 = translate_stmt(r->children[4]);
		code3 = translate_stmt(r->children[6]);
		labelcode1->code.kind = LABEL_CODE;
		labelcode1->code.u.label.l = label1;
		labelcode2->code.kind = LABEL_CODE;
		labelcode2->code.u.label.l = label2;
		labelcode3->code.kind = LABEL_CODE;
		labelcode3->code.u.label.l = label3;
		labelgoto->code.kind = LABEL_GOTO;
		labelgoto->code.u.label.l = label3;
		return bindCode(
				bindCode(
					bindCode(code1,labelcode1),
					bindCode(code2,bindCode(labelgoto,labelcode2))
				),
				bindCode(code3,labelcode3)
				);
	}
	if ((r->arity == 5) && strcmp(r->children[0]->name,"WHILE") == 0) {  // Stmt -> WHILE LP Exp RP Stmt
		label1 = new_label();
		label2 = new_label();
		label3 = new_label();
		code1 = translate_cond(r->children[2],label2,label3);
		code2 = translate_stmt(r->children[4]);
		labelcode1->code.kind = LABEL_CODE;
		labelcode1->code.u.label.l = label1;
		labelcode2->code.kind = LABEL_CODE;
		labelcode2->code.u.label.l = label2;
		labelcode3->code.kind = LABEL_CODE;
		labelcode3->code.u.label.l = label3;
		labelgoto->code.kind = LABEL_GOTO;
		labelgoto->code.u.label.l = label1;
		return bindCode(
				bindCode(
					bindCode(labelcode1,code1),
					bindCode(labelcode2,code2)
				),
				bindCode(labelgoto,labelcode3)
			);
	}
}

struct InterCodes* translate_cond(struct Node* r,struct Operand_* label_true, struct Operand_* label_false) {
	struct InterCodes *tmp, *code1, *code2, *code3, *labelcode1, *labelgoto;
	struct Operand_ *op, *t1, *t2, *label1;
	tmp = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code3 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelcode1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	labelgoto = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	if ((r->arity == 3) && (strcmp(r->children[1]->name,"RELOP") == 0)) {     // Exp -> Exp RELOP Exp
		t1 = new_temp();
		t2 = new_temp();
		code1 = translate_exp(r->children[0],t1);
		code2 = translate_exp(r->children[2],t2);
		op = (struct Operand_*)malloc(sizeof(struct Operand_));
		op->kind = RELOP;
		op->u.relop = r->children[1]->val_id;
		code3->code.kind = IF_GOTO;
		code3->code.u.if_goto.t1 = t1;
		code3->code.u.if_goto.t2 = t2;
		code3->code.u.if_goto.op = op;
		code3->code.u.if_goto.l = label_true;
		labelgoto->code.kind = LABEL_GOTO;
		labelgoto->code.u.label.l = label_false;
		return bindCode(bindCode(bindCode(code1,code2),code3),labelgoto);
	} else if ((r->arity == 2) && (strcmp(r->children[0]->name,"NOT") == 0)) {    // Exp -> NOT Exp
		return translate_cond(r->children[1],label_false,label_true);
	} else if ((r->arity == 3) && (strcmp(r->children[1]->name,"AND") == 0)) {    // Exp -> Exp AND Exp
		label1 = new_label();
		code1 = translate_cond(r->children[0],label1,label_false);
		code2 = translate_cond(r->children[2],label_true,label_false);
		labelcode1->code.kind = LABEL_CODE;
		labelcode1->code.u.label.l = label1;
		return bindCode(bindCode(code1,labelcode1),code2);
	} else if ((r->arity == 3) && (strcmp(r->children[1]->name,"OR") == 0)) {     // Exp -> Exp OR Exp
		label1 = new_label();
		code1 = translate_cond(r->children[0],label_true,label1);
		code2 = translate_cond(r->children[2],label_true,label_false);
		labelcode1->code.kind = LABEL_CODE;
		labelcode1->code.u.label.l = label1;
		return bindCode(bindCode(code1,labelcode1),code2);
	} else {
		t1 = new_temp();
		code1 = translate_exp(r,t1);
		code2->code.kind = IF_GOTO;
		code2->code.u.if_goto.t1 = t1;
		code2->code.u.if_goto.t2 = opval(0);
		code2->code.u.if_goto.op->kind = RELOP;
		code2->code.u.if_goto.op->u.relop = "!=";
		code2->code.u.if_goto.l = label_true;
		labelgoto->code.kind = LABEL_GOTO;
		labelgoto->code.u.label.l = label_false;
		return bindCode(bindCode(code1,code2),labelgoto);
	}
}

struct InterCodes* translate_args(struct Node* r) {
	struct Operand_* t1;
	struct InterCodes* code1, *code2;
	struct argl_* arglisttmp;
	int i;
	code1 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	code2 = (struct InterCodes*)malloc(sizeof(struct InterCodes));
	if (r->arity == 1) { // Args -> Exp
		t1 = new_temp();
		arglisttmp = arg_list;
		code1 = translate_exp(r->children[0],t1);
		arg_list = arglisttmp;
		struct argl_* newarg = (struct argl_*)malloc(sizeof(struct argl_));
		newarg->arg = t1;
		newarg->next = arg_list;
		arg_list = newarg;
		return code1;
	} else {             // Args -> Exp COMMA Args
		t1 = new_temp();
		arglisttmp = arg_list;
		code1 = translate_exp(r->children[0],t1);
		arg_list = arglisttmp;
		struct argl_* newarg = (struct argl_*)malloc(sizeof(struct argl_));
		newarg->arg = t1;
		newarg->next = arg_list;
		arg_list = newarg;
		code2 = translate_args(r->children[2]);
		return bindCode(code1,code2);
	}
}

struct InterCodes* translate_program(struct Node* r) {
	return translate_extdeflist(r->children[0]);
}

struct InterCodes* translate_extdeflist(struct Node* r) {
	if (strstr(r->children[0]->name,"ExtDef") != NULL) {  // ExtDefList -> ExtDef ExtDefList
		if (strstr(r->children[1]->name,"NUL")!=NULL)
			return translate_extdef(r->children[0]);
		else return bindCode(translate_extdef(r->children[0]),translate_extdeflist(r->children[1]));
	} else {
		return new_code();
	}
}

struct InterCodes* translate_extdef(struct Node* r) {
	if ((r->arity == 3) && (strstr(r->children[1]->name,"ExtDecList")!=NULL)) {  // ExtDef -> Specifier ExtDecList SEMI
		return new_code();
	} else if (r->arity == 2) { // ExtDef -> Specifier SEMI
		return new_code();
	} else {  // ExtDef-> Specifier FunDec CompSt
		return bindCode(translate_fundec(r->children[1]),translate_compst(r->children[2]));
	}
}

struct InterCodes* translate_fundec(struct Node* r) {
	struct InterCodes* code1 = new_code();
	code1->code.kind = FUNCTION;
	code1->code.u.func.r = new_func(r->children[0]->val_id);
	if (r->arity == 3) return code1; // FunDec -> ID LP RP
	return bindCode(code1,translate_varlist(r->children[2])); // FunDec -> ID LP VarList RP
}

struct InterCodes* translate_varlist(struct Node* r) {
	if (r->arity == 1) return translate_paramdec(r->children[0]);  // VarList -> ParamDec
	return bindCode(translate_paramdec(r->children[0]),translate_varlist(r->children[2])); // VarList -> ParamDec COMMA VarList
}

struct InterCodes* translate_paramdec(struct Node* r) {
	return translate_vardec(r->children[1],0);
}

struct InterCodes* translate_vardec(struct Node* r,int k) {
	struct InterCodes* code1 = new_code();
	struct Operand_* variable;
	if (k == 0) {
		if (r->arity == 1) {   // VarDec -> ID
			code1->code.kind = PARAM;
			variable = lookup(r->children[0]->val_id);
			code1->code.u.param.p = variable;
			return code1;
		} else {   // VarDec -> ID LB INT RB
			code1->code.kind = DEC;
			variable = lookup(r->children[0]->children[0]->val_id);
			code1->code.u.dec.op1 = variable;
			code1->code.u.dec.op2 = opval(r->children[2]->val_int * 4);
			return code1;
		}
	} else {
		if (r->arity == 4) {
			code1->code.kind = DEC;
			variable = lookup(r->children[0]->children[0]->val_id);
			code1->code.u.dec.op1 = variable;
			code1->code.u.dec.op2 = opval(r->children[2]->val_int * 4);
			return code1;
		} else return new_code();
	}
}


struct InterCodes* translate_compst(struct Node* r) { // CompSt -> LC DefList StmtList RC
	return bindCode(translate_deflist(r->children[1]),translate_stmtlist(r->children[2]));
}

struct InterCodes* translate_stmtlist(struct Node* r) {
	if (r->arity == 2) { // StmtList -> Stmt StmtList
		return bindCode(translate_stmt(r->children[0]),translate_stmtlist(r->children[1]));
	} else {
		return new_code();
	}
}

struct InterCodes* translate_deflist(struct Node* r) {
	if (r->arity == 2) { // DefList -> Def DefList
		return bindCode(translate_def(r->children[0]), translate_deflist(r->children[1]));
	} else {
		return new_code();
	}
}

struct InterCodes* translate_def(struct Node* r) { // Def -> Specifier DecList SEMI
	return translate_declist(r->children[1]);
}

struct InterCodes* translate_declist(struct Node* r) {
	if (r->arity == 1) { // DecList -> Dec
		return translate_dec(r->children[0]);
	} else { // DecList -> Dec COMMA DecList
		return bindCode(translate_dec(r->children[0]),translate_declist(r->children[2]));
	}
}


struct InterCodes* translate_dec(struct Node* r) {
	struct Operand_ *variable, *t1;
	struct InterCodes* code1, *code2;
	if (r->arity == 3) { // Dec -> VarDec ASSIGNOP Exp
		variable = lookup(r->children[0]->val_id);
		t1 = new_temp();
		code1 = translate_exp(r->children[2],t1);
		code2 = new_code();
		code2->code.kind = ASSIGN;
		code2->code.u.assign.left = variable;
		code2->code.u.assign.right = t1;
		return bindCode(code1,code2);
	} else {
		return translate_vardec(r->children[0],1);
	}
}

void printoperand(struct Operand_* op) {
	if (op->kind == VARIABLE) fprintf(fp2,"v%d",op->u.var_name);
	if (op->kind == TEMP) fprintf(fp2,"t%d",op->u.var_name);
	if (op->kind == CONSTANT) fprintf(fp2,"#%d",op->u.value);
	if (op->kind == RELOP) fprintf(fp2,"%s",op->u.relop);
	if (op->kind == LABEL) fprintf(fp2,"label%d",op->u.var_name);
	if (op->kind == FUNC) fprintf(fp2,"%s",op->u.func_name);
	if (op->kind == POINTER) fprintf(fp2,"*t%d",op->u.var_name);
	if (op->kind == ADDRESS) fprintf(fp2,"&v%d",op->u.var_name);
}

void printcodes(struct InterCodes* c){
	if (c == NULL) return;
	switch (c->code.kind) {
		case ASSIGN: {
				    //	 printf("ASSIGN");
						 printoperand(c->code.u.assign.left);
						 fprintf(fp2," := ");
						 printoperand(c->code.u.assign.right);
						 fprintf(fp2,"\n");
						 break;
					 }
		case PLUS: {
					   	printoperand(c->code.u.binop.result);
						fprintf(fp2," := ");
						printoperand(c->code.u.binop.op1);
						fprintf(fp2," + ");
						printoperand(c->code.u.binop.op2);
						fprintf(fp2,"\n");
						break;
				   }
		case IF_GOTO: {
						  fprintf(fp2,"IF ");
						  printoperand(c->code.u.if_goto.t1);
						  fprintf(fp2," ");
						  printoperand(c->code.u.if_goto.op);
						  fprintf(fp2," ");
						  printoperand(c->code.u.if_goto.t2);
						  fprintf(fp2," GOTO ");
						  printoperand(c->code.u.if_goto.l);
						  fprintf(fp2,"\n");
						  break;
					  }
		case READ: {
					   fprintf(fp2,"READ ");
					   printoperand(c->code.u.func.r);
					   fprintf(fp2,"\n");
					   break;
				   }
		case LABEL_GOTO: {
							 fprintf(fp2,"GOTO ");
							 printoperand(c->code.u.label.l);
							 fprintf(fp2,"\n");
							 break;
						 }
		case LABEL_CODE: {
							 fprintf(fp2,"LABEL ");
							 printoperand(c->code.u.label.l);
							 fprintf(fp2," :\n");
							 break;
						 }
		case WRITE: {
						fprintf(fp2,"WRITE ");
						printoperand(c->code.u.wr.w);
						fprintf(fp2,"\n");
						break;
					}
		case RETURN: {
						 fprintf(fp2,"RETURN ");
						 printoperand(c->code.u.ret.r);
						 fprintf(fp2,"\n");
						 break;
					 }
		case UNDEFINE: {
						 //  printf("you get sth wrong here.\n");
						   break;
					   }
		case MINUS: {
						printoperand(c->code.u.binop.result);
						fprintf(fp2," := ");
						printoperand(c->code.u.binop.op1);
						fprintf(fp2," - ");
						printoperand(c->code.u.binop.op2);
						fprintf(fp2,"\n");
						break;
					}
		case STAR: {
						printoperand(c->code.u.binop.result);
						fprintf(fp2," := ");
						printoperand(c->code.u.binop.op1);
						fprintf(fp2," * ");
						printoperand(c->code.u.binop.op2);
						fprintf(fp2,"\n");
						break;
					}
		case DIV: {
						printoperand(c->code.u.binop.result);
						fprintf(fp2," := ");
						printoperand(c->code.u.binop.op1);
						fprintf(fp2," / ");
						printoperand(c->code.u.binop.op2);
						fprintf(fp2,"\n");
						break;
					}
		case PARAM: {
						fprintf(fp2,"PARAM ");
						printoperand(c->code.u.param.p);
						fprintf(fp2,"\n");
						break;
					}
		case ARG: {
					  fprintf(fp2,"ARG ");
					  printoperand(c->code.u.arg.op);
					  fprintf(fp2,"\n");
					  break;
				  }
		case CALLFUNC: {
						   printoperand(c->code.u.callfunc.result);
						   fprintf(fp2," := CALL ");
						   printoperand(c->code.u.callfunc.f);
						   fprintf(fp2,"\n");
						   break;
					   }
		case FUNCTION: {
						   fprintf(fp2,"FUNCTION ");
						   printoperand(c->code.u.func.r);
						   fprintf(fp2," :\n");
						   break;
					   }
		case DEC: {
					  fprintf(fp2,"DEC ");
					  printoperand(c->code.u.dec.op1);
					  fprintf(fp2," ");
					  fprintf(fp2,"%d",c->code.u.dec.op2->u.value);
					  fprintf(fp2,"\n");
					  break;
				  }
		default: {
					 fprintf(fp2,"UNDEFINED\n");
				 }
	}
	printcodes(c->next);
}
