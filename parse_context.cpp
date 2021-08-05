#include "parse_context.h"
#include "error.h"
#include "format.h"

#include <algorithm>

ParseContext::ParseContext(const std::wstring& filename, const std::wstring& source)
	: filename(filename)
	, source(source)
	, scope_depth(0)
	, vars(std::vector<std::vector<Var*>>{})
	, funcs(std::vector<Func*>{})
	, outside_vars(std::vector<std::vector<Var*>>{})
{
	// Global (Top-Level) variable scope
	vars.push_back(std::vector<Var*>{});
	outside_vars.push_back(std::vector<Var*>{});
}



std::wstring ParseContext::get_continue_label()
{
	for (int i = int(this->cont_labels.size()) - 1; 0 <= i; i--) 
	{
		if (!this->cont_labels[i].empty())
		{
			return this->cont_labels[i];
		}
	}
	return L"";
}


std::wstring ParseContext::get_end_label()
{
	for (int i = int(this->end_labels.size()) - 1; 0 <= i; i--)
	{
		if (!this->end_labels[i].empty())
		{
			return this->end_labels[i];
		}
	}
	return L"";
}


void ParseContext::enter_scope()
{
	this->enter_scope(L"", L"");
}



void ParseContext::enter_scope(const std::wstring& continue_label, const std::wstring& end_label)
{
	this->scope_depth++;
	this->vars.push_back(std::vector<Var*>{});
	this->outside_vars.push_back(std::vector<Var*>{});
	this->cont_labels.push_back(continue_label);
	this->end_labels.push_back(end_label);
}



void ParseContext::leave_scope()
{
	if (scope_depth == 0)
	{
		throw std::logic_error(__FUNCTION__ ": unexpected leave_scope");
	}
	this->scope_depth--;
	if (this->in_global())
	{
		this->return_var.clear();
	}
	auto temp_outer_vars = this->outside_vars.back();
	this->outside_vars.pop_back();
	this->vars.pop_back();
	for (auto var : temp_outer_vars)
	{
		// if not local variable then discharge
		if (std::find(this->vars.back().begin(), this->vars.back().end(), var) == this->vars.back().end())
		{
			this->push_outer_var(var);
		}
	}
	// labels
	this->cont_labels.pop_back();
	this->end_labels.pop_back();
}


void ParseContext::define_var(Var* var)
{
	if (this->in_global())
	{
		if (this->find_func_def(var->name))
		{
			throw parse_error(
				this->filename.c_str(),
				this->source.c_str(),
				var->token->pos,
				format(L"variable: '%s' redefined.", var->name.c_str())
			);
		}
	}
	if (this->find_local_var(var->name))
	{
		throw parse_error(
			this->filename.c_str(),
			this->source.c_str(),
			var->token->pos,
			format(L"variable: '%s' redefined.", var->name.c_str())
		);
	}

	this->vars.back().push_back(var);
}


Var* ParseContext::find_var(const std::wstring& ident)
{
	for (int i = int(this->vars.size() - 1); 0 <= i; i--)
	{
		for (auto& each : this->vars[i])
		{
			if (tolower(each->name) == tolower(ident))
			{
				if (i == this->vars.size() - 1 && !this->in_global())
				{
					each->is_local = true;
				}
				else
				{
					each->is_local = false;
				}
				return each;
			}
		}
	}
	return nullptr;
}


Var* ParseContext::find_local_var(const std::wstring& ident)
{
	for (auto& each : this->vars.back())
	{
		if (tolower(each->name) == tolower(ident))
		{
			return each;
		}
	}
	return nullptr;
}


Var* ParseContext::find_global_var(const std::wstring& ident)
{
	for (auto& each : this->vars.front())
	{
		if (tolower(each->name) == tolower(ident))
		{
			return each;
		}
	}
	return nullptr;
}


void ParseContext::define_func(Func* func)
{
	if (this->find_global_var(func->name)) // expect local = global
	{
		throw parse_error(
			this->filename.c_str(),
			this->source.c_str(),
			func->token->pos,
			format(L"function: '%s' redefined.", func->name.c_str())
		);
	}

	if (this->find_func_def(func->name))
	{
		throw parse_error(
			this->filename.c_str(),
			this->source.c_str(),
			func->token->pos,
			format(L"function: '%s' redefined.", func->name.c_str())
		);
	}
	this->funcs.push_back(func);
}


Func* ParseContext::find_func_def(const std::wstring& ident)
{
	for (auto each : this->funcs)
	{
		if (tolower(each->name) == tolower(ident))
		{
			return each;
		}
	}
	return nullptr;
}


bool ParseContext::in_global()
{
	return this->scope_depth == 0;
}

int ParseContext::depth()
{
	// var n: int = 0; //	global ... N/A
	// func main(): int {	function ... 1
	// ...					............ 1
	// if (cond) {          if block ... 2
	// ...                  ............ 2
	// }                    leave block. 1
	//                      ............ 1
	return this->scope_depth - 1;
}

void ParseContext::push_outer_var(Var* var)
{
	auto cur = this->outside_vars.back();
	auto found = std::find(cur.begin(), cur.end(), var);
	if (found == cur.end())
	{
		this->outside_vars.back().push_back(var);
	}
}

std::vector<Var*> ParseContext::get_outer_vars()
{
	return this->outside_vars.back();
}

void ParseContext::return_var_name(const std::wstring& name)
{
	this->return_var = name;
}

std::wstring ParseContext::return_var_name()
{
	return this->return_var;
}

