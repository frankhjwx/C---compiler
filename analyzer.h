#ifndef _ANALYZER_
#define _ANALYZER_

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

struct Type_{
	enum { UNDEFINED, BASIC, ARRAY, STRUCTURE } kind;
	union {
		int basic;  // 0 for INT, 1 for FLOAT
		struct { Type elem; int size; } array;
		FieldList structure;
	} u;
};

struct FieldList_ {
	char* name;
	Type type;
	int lineno;
	FieldList next;
};

struct symbol_table {
	char* name;
	int funcorVariable; // 0 = func, 1 = variable
	int visitedTag; // 0 = definition, 1 = statement
	int lineNumber;
	int if_redefined; // 0 = not, 1 = yes
	Type return_type; // function message
	int num_args; // the number of args for a function
	Type arg_type[128]; // the type of the args
	Type var_type; // the type of a variable
	struct symbol_table* next;
};

void printstruct(Type t);
void printtype(Type t,int i);
void printtable(struct symbol_table* pt,int i);
struct Operand_* lookup(char* s);
struct Type_* check_if_defined(char* s,int funcorVariable,int lineno);
struct symbol_table* func_table(char *s);
int check_before_translate();

#endif

