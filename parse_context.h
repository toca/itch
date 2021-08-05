#pragma once
#include "func.h"
#include <string>
#include <vector>
class ParseContext
{
public:
	ParseContext(const std::wstring& filename, const std::wstring& source);

	void enter_scope();
	void enter_scope(const std::wstring& continue_label, const std::wstring& end_label);
	void leave_scope();

	//void push_label(const std::wstring& continue_label, const std::wstring& end_label);
	//void pop_label();

	std::wstring get_continue_label();
	std::wstring get_end_label();


	void define_var(Var* var);
	Var* find_var(const std::wstring& ident);
	void define_func(Func* func);
	Func* find_func_def(const std::wstring& ident);

	bool in_global();
	int depth();

	void push_outer_var(Var* var);
	std::vector<Var*> get_outer_vars();

	void return_var_name(const std::wstring& name);
	std::wstring return_var_name();

private:
	Var* find_local_var(const std::wstring& ident);
	Var* find_global_var(const std::wstring& ident);

	const std::wstring& source;
	const std::wstring filename;

	std::vector<std::wstring> cont_labels;
	std::vector<std::wstring> end_labels;

	std::vector< std::vector<Var*> > vars;
	std::vector<std::vector<Var*> > outside_vars;
	std::vector<Func*> funcs;
	int scope_depth;
	std::wstring return_var;
};

