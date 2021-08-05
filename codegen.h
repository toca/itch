#pragma once
#include "node.h"
#include "func.h"
#include "var.h"
#include "program.h"
#include <string>
#include <vector>

void codegen(Program* program);
void gen_body(Node* node, const wchar_t* func_name);
std::wstring gen_expr(Node* node);

std::wstring comparison(const std::wstring& op, Node* lhs, Node* rhs);
std::wstring calc(Node* node);
void discharge(std::vector<Var*> vars, const std::wstring& return_var = L"");
void extrication(std::vector<Var*> vars, const std::wstring& return_var, int depth);

std::wstring arg2str(const std::vector<Var*>& vars);
void println(const wchar_t* fmt, ...);
void print(const wchar_t* fmt, ...);

