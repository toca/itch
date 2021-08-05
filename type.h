#pragma once
#include <string>
#include <vector>
#include "node_kind.h"

class Type
{
private:
	enum class type
	{
		UNKNOWN,
		VOID,
		INT,
		STR,
		BOOL,
		FUNC,
	};
	Type(Type::type t);
	Type::type t;

	
public:
	static Type* new_unknown();
	static Type* new_void();
	static Type* new_integer();
	static Type* new_string();
	static Type* new_boolean();
	static Type* new_function();

	bool is_void();
	bool is_integer();
	bool is_string();
	bool is_boolean();
	bool is_function();

	std::wstring to_string();
	static Type* to_type(const std::wstring& str);

	bool validate(NodeKind kind);
	bool validate(NodeKind kind, Type* rhs);


	Type* func_return;
	std::vector<Type*> func_args;

	bool operator==(const Type& rhs);
	bool operator!=(const Type& rhs);
};
