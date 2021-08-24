#pragma once

enum class NodeKind
{
	ADD,			// +
	SUB,			// -
	MUL,			// *
	DIV,			// /
	MOD,			// %
	NEG,			// unary -
	NOT,            // !
	LOGIC_AND,      // &&
	LOGIC_OR,       // ||
	EQ,				// ==
	NE,				// !=
	LT,				// <
	LE,				// <=
	GT,				// >
	GE,				// >=
	ASSIGN,			// =
	ADD_ASSIGN,		// +=
	SUB_ASSIGN,		// -=
	MUL_ASSIGN,     // *=
	DIV_ASSIGN,     // /=
	MOD_ASSIGN,     // %=
	RETURN,			// "return"
	BLOCK,			// { ... }
	EXPR_STMT,		// Expression statement
	LVAR,			// Left  Variable
	RVAR,			// Right Variable
	NUM,			// Integer literal
	BOOL,			// Booolean literal
	STR,			// String literal
	BAT,			// Batch (command) literal
	COMMENT,		// Commment
	NIL,			// empty
	IF,				// if
	FOR,			// for
	FUNCTION,       // function
	CALL,			// function call
	INDIRECT_CALL,  // func object
	GOTO,			// goto
};
