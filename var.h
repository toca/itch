#pragma once
#include "token.h"
#include "type.h"
#include <string>
#include <vector>

enum class VarKind 
{
	CONST, // immutable
	VAR,   // mutable
};
struct Var
{
	std::wstring name;
	Type* type;
	VarKind kind;
	Token* token;
	bool is_local;
};

