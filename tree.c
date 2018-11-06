#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "tree.h"
#include "analyzer.h"
#include "global.h"

struct Type_* nT[128];
struct Type_* current_func_Type;
int Type_num;

struct Node* createNode(int lineno, char* name, int arity, ...){
	va_list vl;
	int i;
	struct Node* T = (struct Node*)malloc(sizeof(struct Node));
	struct Type_* newtype = (struct Type_*)malloc(sizeof(struct Type_));
	T->type = newtype;
	T->arity = arity;
	T->name = name;
	T->lineno = lineno;
	va_start(vl,arity);
	for (i=0;i<9;i++) T->children[i] = NULL;
	
	if (strstr(name,"ID")!=NULL || strstr(name,"TYPE")!=NULL || strstr(name,"RELOP")!=NULL)
		T->val_id = val_id;	

	if (strstr(name,"INT")!=NULL) T->val_int = val_int;
	
	if (strstr(name,"FLOAT")!=NULL) T->val_float = val_float;

	for (i=0;i<arity;i++) {
		T->children[i] = va_arg(vl,struct Node*);
		if (T->children[i]->lineno < T->lineno) T->lineno = T->children[i]->lineno;
	}
	va_end(vl);
	return T;
}

struct Node* createIDNode(int lineno, char* val_id){
	struct Node* T = (struct Node*)malloc(sizeof(struct Node));
	T = createNode(lineno,"ID",0);
	T->val_id = val_id;
	return T;
}

void construct_table(struct Node* r) {
	int i;
//	printf("%s\n",r->name);

	if (strstr(r->name,"NUL")!=NULL) return;
	r->type->kind = UNDEFINED;
	for (i=0; i<r->arity; i++)
		construct_table(r->children[i]);
	
	if (strstr(r->name,"TYPE")!=NULL) {     // TYPE
		r->type->kind = BASIC;
		if (strstr(r->val_id,"int")!=NULL)
			r->type->u.basic = 0;
		else r->type->u.basic = 1;
	}

	if (strstr(r->name,"INT")!=NULL) {      // INT
		r->type->kind = BASIC;
		r->type->u.basic = 0;
	}

	if (strstr(r->name,"FLOAT")!=NULL) {    // FLOAT
		r->type->kind = BASIC;
		r->type->u.basic = 1;
	}

	if (strstr(r->name,"VarDec")!=NULL){    // VarDec
		if (r->arity == 1) {                // VarDec -> ID
			r->type = r->children[0]->type;
			r->val_id = r->children[0]->val_id;
		} else {                            // VarDec -> VarDec LB INT RB
			r->type->kind = ARRAY;
			r->val_id = r->children[0]->val_id;
			if (r->children[0]->type->kind != ARRAY) {
				struct Type_* newtype = (struct Type_*)malloc(sizeof(struct Type_));
				r->type->u.array.size = 1;
				r->type->u.array.elem = r->children[0]->type;
			} else {
				struct Type_* newtype = (struct Type_*)malloc(sizeof(struct Type_));
				r->type->u.array.size = r->children[0]->type->u.array.size + 1;
				r->type->u.array.elem = r->children[0]->type;
			}
		}
	}

	if (strstr(r->name,"Specifier")!=NULL && (strlen(r->name) == 9)){  // Specifier
		if (strstr(r->children[0]->name,"TYPE")!=NULL) { // Specifier -> TYPE
			r->type = r->children[0]->type;
		} else {                                         // Specifier -> StructSpecifier
			r->type = r->children[0]->type;
		}
	}

	if ((strstr(r->name,"Dec")!=NULL) && (strlen(r->name) == 3)) {       // Dec
		if (r->arity == 1) {                 // Dec -> VarDec
			r->type = r->children[0]->type;
			r->val_id = r->children[0]->val_id;
		} else {                             // Dec -> VarDec ASSIGNOP Exp
			r->type = r->children[0]->type;
			r->val_id = r->children[0]->val_id;
		}
	}

	if (strstr(r->name,"DecList")!=NULL) {    // DecList
		if (r->arity == 1) {                  // DecList -> VarDec
			r->type = r->children[0]->type;
			r->val_id = r->children[0]->val_id;
		} else {                              // DecList -> Dec COMMA DecList
			r->type = r->children[0]->type;
			r->val_id = r->children[0]->val_id;
		}
	}
	

	if ((strstr(r->name,"Def")!=NULL) && (strlen(r->name) == 3)) {           // Def
		r->val_id = r->children[1]->val_id;
		r->type = r->children[0]->type;
	}

	if ((strstr(r->name,"DefList")!=NULL) && (strlen(r->name)==7)) {                                 // DefList
		if (r->arity == 2) {
			r->val_id = r->children[0]->val_id;
			r->type = r->children[0]->type;
		}
	}

	if ((strstr(r->name,"CompSt")!=NULL)) {        // CompSt
		if (r->arity == 4) {
			struct Node* tr = (struct Node*)malloc(sizeof(struct Node));      // CompSt -> LC DefList StmtList RC
			r->type = r->children[1]->type;
			tr = r->children[1];
			do {
				if (tr->arity == 0) break;
				struct Node* tmpr = (struct Node*)malloc(sizeof(struct Node));
				tmpr = tr;
				tmpr = tmpr->children[0];
				tmpr = tmpr->children[1];
				do  {
					if (tmpr->type->kind == ARRAY) {
						tmpr->type->u.array.elem = tr->type;
					} else {
						tmpr->type = tr->type;
					}
					struct symbol_table* newtable = (struct symbol_table*)malloc(sizeof(struct symbol_table));
					newtable->name = tmpr->val_id;
					newtable->funcorVariable = 1;
					newtable->visitedTag = 0;
					newtable->lineNumber = tmpr->lineno;
					newtable->var_type = tmpr->type;
					if (S_table_r == NULL) {
						S_table_r = newtable;
						now = S_table_r;
					} else {
						now->next = newtable;
						now = now->next;
					}
					tmpr = tmpr->children[2];
				} while (tmpr!=NULL);
				tr = tr->children[1];
				if (tr == NULL) break;
			} while (strstr(tr->name,"NUL")==NULL);
	    }
	}
	
	if (strstr(r->name,"ParamDec")!=NULL) { // ParamDec
		if (r->children[1]->type->kind == ARRAY) {
			r->type = r->children[1]->type;
			r->type->u.array.elem = r->children[0]->type;
		} else {
			r->type = r->children[0]->type;
		}
		r->val_id = r->children[1]->val_id;
	}

	if (strstr(r->name,"VarList")!=NULL) { // VarList
		r->type = r->children[0]->type;
	}

	if (strstr(r->name,"FunDec")!=NULL) {  // FunDec
		r->val_id = r->children[0]->val_id;
	}

	if ((strstr(r->name,"ExtDef")!=NULL) && (strlen(r->name)==6)) {  // ExtDef
		r->val_id = r->children[1]->val_id;
		r->type = r->children[0]->type;
		if (strstr(r->children[1]->name,"FunDec")!=NULL) {  // ExtDef -> Specifier FunDec CompSt
			struct symbol_table* newtable = (struct symbol_table*)malloc(sizeof(struct symbol_table));
			newtable->name = r->val_id;
			newtable->funcorVariable = 0;
			newtable->visitedTag = 0;
			newtable->lineNumber = r->lineno;
			newtable->return_type = r->type;
			newtable->num_args = 0;

			struct Node* tmpr = (struct Node*)malloc(sizeof(struct Node));
			tmpr = r->children[1]; // Fundec
			if (tmpr->arity == 4) {  // Fundec -> ID LP VarList RP
				newtable->num_args++;
				tmpr = tmpr->children[2];
				while (tmpr->arity == 3) {
					newtable->arg_type[newtable->num_args] = tmpr->type;
					newtable->num_args++;
					tmpr = tmpr->children[2];
				}
				newtable->arg_type[newtable->num_args] = tmpr->type;
			} else {                 // Fundec -> ID LP RP
				newtable->num_args = 0;
			}

			if (S_table_r == NULL) {
				S_table_r = newtable;
				now = S_table_r;
			} else {
				now->next = newtable;
				now = now->next;
			}

			tmpr = r->children[1];
			if (tmpr->arity == 4) {
				tmpr = tmpr->children[2];
				while (tmpr!=NULL) {
					struct symbol_table* newtable2 = (struct symbol_table*)malloc(sizeof(struct symbol_table));
					newtable2->name = tmpr->children[0]->val_id;
					newtable2->funcorVariable = 1;
					newtable2->visitedTag = 0;
					newtable2->lineNumber = tmpr->lineno;
					newtable2->var_type = tmpr->type;

					if (S_table_r == NULL) {
						S_table_r = newtable;
						now = S_table_r;
					} else {
						now->next = newtable2;
						now = now->next;
					}
					tmpr = tmpr->children[2];
				}
			}
		} else {                                   
		}
	}

	if (strstr(r->name,"OptTag")!=NULL) {               // OptTag
		if (r->arity == 1) {                             // OptTag -> ID
			r->val_id = r->children[0]->val_id;
		}
	}

	if (strstr(r->name,"StructSpecifier")!=NULL) {      // StructSpecifier
		if (r->arity == 5) {                            // StructSpecifier -> STRUCT OptTag LC DefList RC
			r->val_id = r->children[1]->val_id;
			Type_num++;
			struct Type_* newtype = (struct Type_*)malloc(sizeof(struct Type_));
			newtype->kind = STRUCTURE;
			struct FieldList_* newfield = (struct FieldList_*)malloc(sizeof(struct FieldList_));
			newtype->u.structure = newfield;
			newtype->u.structure->name = r->val_id;
			newtype->u.structure->lineno = r->lineno;
			struct FieldList_* structurefieldhead = (struct FieldList_*)malloc(sizeof(struct FieldList_));
			struct FieldList_* structurefieldnow = (struct FieldList_*)malloc(sizeof(struct FieldList_));
			struct Node* tmpr = (struct Node*)malloc(sizeof(struct Node));
			structurefieldhead = NULL;
			
			tmpr = r;
			tmpr = tmpr->children[3];    // DefList
			do {
				struct Node* tmpr2 = (struct Node*)malloc(sizeof(struct Node));
				tmpr2 = tmpr->children[0]->children[1];   // DecList
				do {
					struct FieldList_* structurefield = (struct FieldList_*)malloc(sizeof(struct FieldList_));
					struct Type_* newtype2 = (struct Type_*)malloc(sizeof(struct Type_));
					structurefield->type = newtype2;
					structurefield->name = tmpr2->val_id; 
					structurefield->lineno = tmpr->lineno;
					if (tmpr2->type->kind == ARRAY) {
						structurefield->type = tmpr2->type;
						structurefield->type->u.array.elem = tmpr->type;
					} else {
						structurefield->type = tmpr->type;
					}
					if (structurefieldhead == NULL) {
						structurefieldhead = structurefield;
						structurefieldnow = structurefieldhead;
					} else {
						structurefieldnow->next = structurefield;
						structurefieldnow = structurefieldnow->next;
					}
					tmpr2 = tmpr2->children[2];
				} while (tmpr2!=NULL);

				tmpr = tmpr->children[1];
			} while (tmpr->arity == 2);
			
			newtype->u.structure->type = UNDEFINED;
			newtype->u.structure->next = structurefieldhead;
			nT[Type_num] = newtype;
			r->type = newtype;
		} else {                                        // StructSpecifier -> STRUCT Tag
			r->val_id = r->children[1]->children[0]->val_id;
			int k;
			k = -1;
			for (i=1;i<=Type_num;i++) {
				if (strcmp(nT[i]->u.structure->name,r->val_id) == 0) {
					k = i;
					break;
				}
			}
			if (k == -1) {
				printf("Error type 17 at Line %d: Undefined structure \"%s\".\n",r->lineno,r->val_id);
			} else {
				r->type = nT[k];
			}
		}
	}

	if (strstr(r->name,"Exp")!=NULL){      // Exp
		if (strstr(r->children[0]->name,"INT")!=NULL) {  // Exp -> INT
			r->type->kind = BASIC;
			r->type->u.basic = 0;
		} else if (strstr(r->children[0]->name,"FLOAT")!=NULL) { // Exp -> FLOAT
			r->type->kind = BASIC;
			r->type->u.basic = 1;
		} else if ((r->arity == 2) && (strstr(r->children[0]->name,"MINUS")!=NULL)) { //Exp -> MINUS Exp
			r->type = r->children[1]->type;
		}
	}

}

void add_read(){
	struct symbol_table* newtable;
	newtable = (struct symbol_table*)malloc(sizeof(struct symbol_table));
	newtable->name = "read";
	newtable->funcorVariable = 0;
	newtable->lineNumber = -1;
	newtable->num_args = 0;
	newtable->return_type = (struct Type_*)malloc(sizeof(struct Type_));
	newtable->return_type->kind = BASIC;
	newtable->return_type->u.basic = 0;
	now->next = newtable;
	now = now->next;
}

void add_write(){
	struct symbol_table* newtable;
	newtable = (struct symbol_table*)malloc(sizeof(struct symbol_table));
	newtable->name = "write";
	newtable->funcorVariable = 0;
	newtable->lineNumber = -1;
	newtable->num_args = 1;
	newtable->arg_type[1] = (struct Type_*)malloc(sizeof(struct Type_));
	newtable->return_type = (struct Type_*)malloc(sizeof(struct Type_));
	newtable->arg_type[1]->kind = BASIC;
	newtable->arg_type[1]->u.basic = 0;
	newtable->return_type->kind = BASIC;
	newtable->return_type->u.basic = 0;
	now->next = newtable;
	now = now->next;
}

void sematic_analysis(struct Node* r){
	int i;

	if (strstr(r->name,"ExtDef")!=NULL && strlen(r->name) == 6) { //ExtDef
		current_func_Type = r->type;
	}


	for (i=0; i<r->arity; i++)
		sematic_analysis(r->children[i]);
	
	
	if (strstr(r->name,"Exp")!=NULL) {              // Exp
		if (strstr(r->children[0]->name,"ID")!=NULL && r->arity == 1) {    // Exp -> ID
			struct Type_* tmp;
			tmp = check_if_defined(r->children[0]->val_id,1,r->lineno);
			if (tmp->kind != UNDEFINED) {
				r->type = tmp;
			}
		} else if ((r->arity == 3) && (strstr(r->children[0]->name,"LP")!=NULL)) { // Exp -> LP Exp RP
			r->type = r->children[1]->type;
		} else if (strstr(r->children[0]->name,"ID")!=NULL && r->arity == 3) { // Exp -> ID LP RP
			struct Type_* tmp;
			tmp = check_if_defined(r->children[0]->val_id,0,r->lineno);
			if (tmp->kind != UNDEFINED) {
				r->type = tmp;
			struct symbol_table* original_func;
			original_func = func_table(r->children[0]->val_id);
			if (original_func->num_args != 0) {
				printf("Error type 9 at Line %d: Function \"%s(",r->lineno,r->children[0]->val_id);
				int k;
				if (original_func->arg_type[1]->kind == BASIC) {  // getting dizzy here, the description is so ambiguous, how to print array or structure error here? anyway, i only complete int/float type here
					if (original_func->arg_type[1]->u.basic == 0)
						printf("int");
					else printf("float");
				}
				for (k=2; k<=original_func->num_args; k++) {
					if (original_func->arg_type[k]->kind == BASIC) {
						if (original_func->arg_type[k]->u.basic == 0)
							printf(", int");
						else printf(", float");
					}
				}
				printf(")\" is not applicable for arguments \"()\".\n");
			}
			}

		} else if (strstr(r->children[0]->name,"ID")!=NULL && r->arity == 4) { // Exp -> ID LP Args RP
			struct Type_* tmp;
			tmp = check_if_defined(r->children[0]->val_id,0,r->lineno);
			if (tmp->kind != UNDEFINED) {
				r->type = tmp;

			struct symbol_table* original_func;
			original_func = func_table(r->children[0]->val_id);

			struct Node* tmpr = r;
			tmpr = tmpr->children[2];
			int t, k;
			t = 0;
			k = 0;
			struct Type_* tmpt[128];
			while (tmpr!=NULL) {
				t++;
				tmpt[t] = tmpr->children[0]->type;
				tmpr = tmpr->children[2];
			}

			int if_illegal = 0;
			if (t != original_func->num_args) if_illegal = 1;
			if (if_illegal == 0)
				for (k = 1; k <= original_func->num_args;k++) {
					if (cmp_type(tmpt[k],original_func->arg_type[k]) == 1)
						if_illegal = 1;
				}

			if (if_illegal == 1) {
				printf("Error type 9 at Line %d: Function \"%s(",r->lineno,r->children[0]->val_id);
				if (original_func->num_args!=0) {
					if (original_func->arg_type[1]->kind == BASIC) {
						if (original_func->arg_type[1]->u.basic == 0)
							printf("int");
						else printf("float");
					}
					for (k=2;k<=original_func->num_args;k++) {
						if (original_func->arg_type[k]->kind == BASIC) {
							if (original_func->arg_type[k]->u.basic == 0)
								printf(", int");
							else printf(", float");
						}
					}
				}
				printf(")\" is not applicable for arguments \"(");
				if (tmpt[1]->kind == BASIC) {
					if (tmpt[1]->u.basic == 0)
						printf("int");
					else printf("float");
				}
				for (k=2;k<=t;k++) {
					if (tmpt[k]->kind == BASIC) {
						if (tmpt[k]->u.basic == 0)
							printf(", int");
						else printf(", float");
					}
				}
				printf(")\"\n");
			}
			}


		} else if ((r->arity >= 2) && (strstr(r->children[1]->name,"ASSIGNOP")!=NULL)) {   // Exp -> Exp ASSIGNOP Exp
			if (cmp_type(r->children[0]->type, r->children[2]->type) == 1) {
				if (r->children[0]->type->kind != UNDEFINED)
					printf("Error type 5 at line %d: Type mismatched for assignment.\n", r->lineno);
			}

			int if_legal;
			if_legal = 1; // illegal

			if (strstr(r->children[0]->children[0]->name,"ID")!=NULL) { //  ID on left side
				if_legal = 0;
			} 

			if ((r->children[0]->arity == 4) && strstr(r->children[0]->children[1]->name,"LB")!=NULL) { //  Exp LB Exp RB
				if_legal = 0;
				r->type = r->children[2]->type;
			}

			if ((r->children[0]->arity == 3) && strstr(r->children[0]->children[1]->name,"DOT")!=NULL) { //  Exp DOT ID
				if_legal = 0;
			}

			if (if_legal == 1) {
				printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",r->lineno);
			}
		} else if ((r->arity == 3) && ((strstr(r->children[1]->name,"PLUS")!=NULL) || (strstr(r->children[1]->name,"MINUS")!=NULL) || (strstr(r->children[1]->name,"STAR")!=NULL) || (strstr(r->children[1]->name,"DIV")!=NULL))) {
			if (cmp_type(r->children[0]->type,r->children[2]->type) == 1) {
				printf("Error type 7 at Line %d: Type mismatched for operands.\n", r->lineno);
			} else {
				r->type = r->children[0]->type;
			}
		} else if ((r->arity == 4) && ((strstr(r->children[1]->name,"LB")!=NULL))) { // Exp LB Exp RB
			if (r->children[2]->type->u.basic != 0) {
				printf("Error type 12 at Line %d: \"%f\" is not an integer.\n",r->lineno,r->children[2]->children[0]->val_float);
			} else {
				if (strstr(r->children[0]->children[0]->name,"ID")!=NULL) {
					if (check_if_defined(r->children[0]->children[0]->val_id,1,r->lineno)->kind != ARRAY) {
						printf("Error type 10 at Line %d: \"%s\" is not an array.\n",r->lineno,r->children[0]->children[0]->val_id);
					}
				}
			}
			r->type = r->children[0]->type->u.array.elem;
		} else if ((r->arity == 3) && (strstr(r->children[1]->name,"DOT")!=NULL)) { // Exp DOT ID
			if (r->children[0]->type->kind != STRUCTURE) {
				if (r->children[0]->type->kind != UNDEFINED)
					printf("Error type 13 at Line %d: Illegal use of \".\".\n",r->lineno);
			} else {
				struct FieldList_* tmpr;
				tmpr = r->children[0]->type->u.structure;
				int ifillegal = 1;
				do {
					tmpr = tmpr->next;
					if (strcmp(tmpr->name,r->children[2]->val_id) == 0)
						ifillegal = 0;
				} while (tmpr->next!=NULL);
				if (ifillegal == 1) {
					printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",r->lineno,r->children[2]->val_id);
				}

			}
		}
			
	}
	
	if (strstr(r->name,"Stmt")!=NULL && strlen(r->name) == 4) {  // Stmt
		if (strstr(r->children[0]->name,"RETURN")!=NULL) {  // Stmt -> RETURN Exp SEMI
			if (cmp_type(r->children[1]->type,current_func_Type) == 1) {
				printf("Error type 8 at Line %d: Type mismatched for return.\n", r->lineno);
			}
		}
	}

}

void printNode(struct Node* r, int nLayer){
	int i;
	if (strstr(r->name,"NUL")!=NULL) return;
	for (i=0;i<nLayer;i++) {
		printf("  ");
	}
	printf("%s ",r->name);	
	if (strstr(r->name,"ID")!=NULL || strstr(r->name,"TYPE")!=NULL) {
		printf(": %s",r->val_id);
	}

	if (strstr(r->name,"INT")!=NULL) {
		printf(": %d",r->val_int);	
	}
	
	if (strstr(r->name,"FLOAT")!=NULL) {
		printf(": %f",r->val_float);
	}

	if (r->arity != 0) 
		printf(" (%d)",r->lineno);

	if (r->type->kind == BASIC)
		printf("......BASIC");
	else if (r->type->kind == ARRAY)
		printf("......ARRAY");
	else if (r->type->kind == STRUCTURE)
		printf("......STRUCT");

	printf("\n");
	for (i=0;i<r->arity;i++) {
		printNode(r->children[i],nLayer+1);
	}
	return;
}
