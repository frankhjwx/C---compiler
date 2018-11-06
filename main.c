#include <stdio.h>
#include <stdarg.h>
#include "tree.h"
#include "analyzer.h"
#include "global.h"
#include "inter_code.h"

struct Node* root;
struct symbol_table* S_table_r;
struct symbol_table* now;
struct InterCodes* codes;
int valnum;
int Type_num;
int tempnum;
int labelnum;
FILE* fp2;

void init(){
	if_error = 0;
	valnum = 0;
	Type_num = 0;
	tempnum = 0;
	labelnum = 0;
}

int main(int argc, char** argv) {
	if (argc <= 2) return 1;
	FILE* f = fopen(argv[1], "r");
	fp2 = fopen(argv[2], "w");
	if (!f)
	{
		perror(argv[1]);
		return 1;
	}
	init();
	yyrestart(f);
	yyparse();
	if (!if_error) {
		construct_table(root);
		add_read();
		add_write();
		check_redefined();
		sematic_analysis(root);
		if (check_before_translate() == 0) {
			codes = translate_program(root);
			printcodes(codes);
		}
//		printNode(root,0);
	
	}
//	printtable(S_table_r,0);
	return 0;
}
