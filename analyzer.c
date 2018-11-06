#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "analyzer.h"
#include "global.h"
#include "inter_code.h"

void printtype(Type t,int i){
	int j;
	for (j=1;j<=i;j++) printf("  ");
	if (t->kind == BASIC) {
		printf("  BASIC\n");
		for (j=1;j<=i;j++) printf("  ");
		if (t->u.basic == 0) printf("  INT\n");
		else printf("  FLOAT\n");
	} else if (t->kind == ARRAY) {
		printf("  ARRAY\n");
		printtype(t->u.array.elem,i+1);
		for (j=1;j<=i;j++) printf("  ");
		printf("  size = %d\n",t->u.array.size);
	} else if (t->kind == STRUCTURE) {
		printf("  STRUCTURE\n");
		printf("  %s\n",t->u.structure->name);
	} else {
		printf("  UNDEFINED\n");
	}
}

void printstruct(Type t){
	printf("STRUCTURE\n");
	printf("  name:%s",t->u.structure->name);
	printf("  defined as:\n");
	struct FieldList_* tmpf= (struct FieldList_*)malloc(sizeof(struct FieldList_));
	tmpf = t->u.structure->next;
	while (tmpf!=NULL) {
		if (tmpf->type->kind == UNDEFINED) break;
		printf("   id: %s\n ",tmpf->name);
		printf("   type: ");
		printtype(tmpf->type,3);
		tmpf = tmpf->next;
	}
}

void printtable(struct symbol_table* pt,int i) {
	if (pt == NULL) return;
	i = i+1;
	int j;
	printf("No.%d:\n",i);
	printf("  %s\n",pt->name);
	if (pt->funcorVariable == 0){
		printf("  FUNC\n");
		printf("  RETURN TYPE:\n");
		printtype(pt->return_type,0);
		printf("  ARGS: %d\n",pt->num_args);
		for (j = 1;j<=pt->num_args; j++){
			if (pt->arg_type[j]!=NULL) printtype(pt->arg_type[j],0);
			else printf("  ERROR\n");
		}

	} else {
		printf("  VAR\n");
		printtype(pt->var_type,0);
	}
	printtable(pt->next,i);
}

struct Operand_* lookup(char* s){
	struct symbol_table* pt = S_table_r;
	struct Operand_* tmp = (struct Operand_*)malloc(sizeof(struct Operand_));
	int no = 0;
	while (pt!=NULL) {
		no++;
		if (strcmp(pt->name,s) == 0) {
			tmp->kind = VARIABLE;
			tmp->u.var_name = no;
			return tmp;
		}
		pt = pt->next;
	}
	return NULL;
}

struct Type_* check_if_defined(char* s,int funcorVariable, int lineno){
	struct symbol_table* pt = S_table_r;

	struct Type_* tmp = (struct Type_*)malloc(sizeof(struct Type_));
	tmp->kind = UNDEFINED;
	while (pt!=NULL) {
		if (strcmp(pt->name,s) == 0) {
			if (pt->funcorVariable == funcorVariable) {
				if (pt->funcorVariable == 1) return pt->var_type;
				else return pt->return_type;
			} else {
				if (pt->funcorVariable == 1) {
					printf("Error type 11 at Line %d: \"%s\" is not a function.\n",lineno,s);					
				} else {
					printf("Error type 11 at Line %d: \"%s\" is not a variable.\n",lineno,s);
				}
				return tmp;
			}
		}
		pt = pt->next;
	}
	printf("Error type %d at Line %d: Undefined ",2-funcorVariable,lineno);
	if (funcorVariable == 0) printf("function");
	else printf("variable");
	printf(" \"%s\".\n",s);
	return tmp;
}

void check_redefined(){
	struct symbol_table* pt1 = S_table_r;
	struct symbol_table* pt2;
	while (pt1!=NULL){
		pt1->if_redefined = 0;
		pt1 = pt1->next;
	}
	pt1 = S_table_r;
	while (pt1!=NULL) {
		if (pt1->if_redefined != 1) {
			pt2 = pt1->next;
			while (pt2!=NULL) {
				if (strcmp(pt1->name,pt2->name) == 0 && pt1->funcorVariable == pt2->funcorVariable) {
					printf("Error type %d at Line %d: Redefined", 4 - pt2->funcorVariable, pt2->lineNumber);
					if (pt2->funcorVariable == 0) printf(" function ");
					else printf(" variable ");
					printf("\"%s\".\n",pt2->name);
					pt2->if_redefined = 1;
				}
				pt2 = pt2->next;
			}
		}
		pt1 = pt1->next;
	}

	int i,j;
	for (i=1;i<=Type_num;i++) {
		struct FieldList_* nf1;
		struct FieldList_* nf2;
		nf1 = nT[i]->u.structure;
		nf1 = nf1->next;
		while (nf1!=NULL) {
			nf2 = nf1->next;
			while (nf2!=NULL) {
				if (strcmp(nf1->name,nf2->name) == 0) {
					printf("Error type 15 at Line %d: Redefined field \"%s\".\n",nf2->lineno,nf1->name);
				}
				nf2 = nf2->next;
			}
			nf1 = nf1->next;
		}
	}

	for (i=1;i<=Type_num-1;i++) {
		for (j=i+1;j<=Type_num;j++) {
			if (strcmp(nT[i]->u.structure->name,nT[j]->u.structure->name) == 0) {
				printf("Error type 16 at Line %d: Duplicated name \"%s\".\n",nT[j]->u.structure->lineno,nT[i]->u.structure->name);
			}
		}
	}

}

int cmp_type(struct Type_* ta,struct Type_* tb) { // 0 = equal, 1 = different
	if (ta->kind != tb->kind) return 1;
	if (ta->kind == BASIC) {
		if (ta->u.basic == tb->u.basic) return 0;
		else return 1;
	} else if (ta->kind == ARRAY) {
		if (ta->u.array.size == tb->u.array.size && cmp_type(ta->u.array.elem,tb->u.array.elem) == 0)
			return 0;
		else return 1;
	} else if (ta->kind == STRUCTURE) {
		struct FieldList_* f1 = ta->u.structure->next;
		struct FieldList_* f2 = tb->u.structure->next;
		while (f1!=NULL) {
			if (f2 == NULL) return 1;
			if (cmp_type(f1->type,f2->type) == 1) return 1;
			f1 = f1->next;
			f2 = f2->next;
		}
		if (f2!=NULL) return 1;
	}
	return 0;
}

int check_before_translate(){
	struct symbol_table* pt = S_table_r;
	while (pt!=NULL) {
		if (pt->funcorVariable == 1) { //var
			if (pt->var_type->kind == STRUCTURE) {
				printf("Cannot translate: Code contains variables or parameters of structure type.\n");
				return 1;
			}
			if (pt->var_type->kind == ARRAY && pt->var_type->u.array.size>1) {
				printf("Cannot translate: Code contains variables of multi-dimensional array type.\n");
				return 1;
			}
		} else {
			int i;
			for (i=1;i<=pt->num_args;i++)
				if (pt->arg_type[i]->kind == ARRAY) {
					printf("Cannot translate: Code contains parameters of array type.\n");
					return 1;
				}
		}
		pt = pt->next;
	}
	return 0;
}


struct symbol_table* func_table(char *s) {
	struct symbol_table* pt = S_table_r;
	while (pt!=NULL) {
		if (strcmp(pt->name,s) == 0)
			return pt;
		pt = pt->next;
	}
	return pt;
}
