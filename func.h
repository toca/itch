#pragma once

#include <string>
#include <vector>
#include "node.h"
#include "type.h"
#include "var.h"

class Func
{
public:
	Func();
	void push_outer_var(Var* var);
	std::vector<Var*> outer_vars;

	std::wstring name;
	std::vector<Var*> args;
	// TODO remove & use sig->type
	Type* type;
	Type* sig;
	Node* body;
	Func* next;
	Token* token;
};