#pragma once

#include <string>
#include <vector>
#include "node_kind.h"
#include "token.h"
#include "type.h"
#include "var.h"


class Node {
public:
	Node();
	void push_outer_var(Var* var);

	NodeKind kind;					// kind
	Node* next;						// next-node
	Node* lhs;						// Left-hand side
	Node* rhs;						// Right-hand side
	Node* body;						// block body
	Node* condition;				// if, for condition
	Node* else_stmt;				// if - else statement
	Node* init;						// for( init; ...; ...)
	Node* increment;				// for( ...; increment: ...)
	Node* args;						// function args
	Token* token;					// for logging
	std::wstring name;				// variable name
	std::wstring dest;				// goto destination
	std::wstring begin;				// label for begin
	std::wstring end;				// label for end
	std::wstring cont;				// label for continue
	Type* type;						// int, string, bool, func
	int int_val;					// Used if kind == NUM
	bool bool_val;					// Used in kind == BOOL
	std::wstring str_val;			// Used in kind == STRING
	std::wstring func_name;         // Used in kind == FUNCTION
	std::vector<Var*> outer_vars;	// variables not local
	std::wstring return_var_name;   // for return
	int depth;						// for return
};



std::wstring kind2op(NodeKind kind);
