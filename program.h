#pragma once

#include "func.h"
#include "node.h"

struct Program
{
	Func* func;
	Node* global_var;
};
