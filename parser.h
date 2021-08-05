#pragma once

#include <string>
#include "node.h"
#include "token.h"
#include "tokens.h"
#include "func.h"
#include "program.h"
#include "parse_context.h"


class Parser
{
public:
	Parser(Tokens& tokens, const std::wstring& filename, const std::wstring& source);
	Program* parse();

private:
	Node* pre_proc();
	void post_proc(Func* func);

	Func* function(bool anonymous = false);
	Func* func_def(bool anonymous = false);
	Var* param();
	void solve_scope(Func* func);
	std::vector<Var*> solve_vars(Node* body);
	void collect_func_call(Node* node, std::vector<Node*>* result);

	Node* statement();
	Node* compound_statement();
	Node* expr();
	Node* logical_or();
	Node* logical_and();
	Node* declaration();
	Node* expr_statement();
	Node* assign_statement();
	Node* assign();

	Node* equality();
	Node* relational();
	Node* add();
	Node* mul();
	Node* unary();
	Node* postfix();
	Node* primary();
	Node* func_call();
	Node* lvar();

	Type* type_spec();
	Type* type();
	Type* func_sig();


	void validate_operation(Node* node, NodeKind kind);
	void validate_operation(Node* lhs, Node* rhs, NodeKind kind);
	

	// TODO move to node
	Node* new_node(NodeKind kind, Token* token);
	Node* new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* token);
	Node* new_bool_binary(NodeKind kind, Node* lhs, Node* rhs, Token* token);
	Node* new_unary(NodeKind kind, Node* lhs, Token* token);
	Node* new_lvar_node(Token* token);
	Node* new_rvar_node(Token* token);
	Node* new_num_node(int num, Token* token);
	Node* new_bool_node(Token* token);
	Node* new_string_node(Token* token);
	Node* new_bat_node(Token* token);
	Node* new_comment_node(Token* token);
	Node* new_const_lvar(Token* token);

	void add_func(Func* func);
	Func* find_func(const std::wstring& name);
	bool is_assign(Token* token);
	bool is_variable(Token* token);
	std::wstring unique_label(const std::wstring& prefix);


	Tokens& tokens;
	const std::wstring filename;
	const std::wstring& source;
	ParseContext context;
	Func* func_cursor;

	bool collect_return_types(Node* node, std::vector<Type*>* types);
};
