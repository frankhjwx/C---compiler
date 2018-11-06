#ifndef _TREE_
#define _TREE_
#include "analyzer.h"

struct Node {
	int type_terminal; // 1 = is_terminal   0 = not_terminal
	char* name; // the name of the Node
	int lineno;  // the line_number of the Node
	union{
		char* val_id; // the value of the TYPE or ID
		int val_int; // the value of INT
		float val_float; // the value of FLOAT
	};
	int arity; // the num of sons
	Type type;
	int argnum;
	struct Node* children[10]; // Node pointing to sons
};

struct Node* createNode(int lineno, char* name, int arity, ...);
struct Node* createIDNode(int lineno, char* val_id);
void printNode(struct Node* r, int nLayer);
void add_read();
void add_write();
#endif
