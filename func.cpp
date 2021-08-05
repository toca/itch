#include "func.h"

Func::Func()
{
}

void Func::push_outer_var(Var* var)
{
	auto find = std::find_if(this->outer_vars.begin(), this->outer_vars.end(), [var](auto each) {
		return each->name == var->name;
	});
	if (find == this->outer_vars.end())
	{
		this->outer_vars.push_back(var);
	}
}
