#include "node.h"
#include <stdexcept>
std::wstring kind2op(NodeKind kind)
{
    switch (kind)
    {
    case NodeKind::NOT:
        return L"!";
    case NodeKind::ADD:
        return L"+";
    case NodeKind::NEG:
    case NodeKind::SUB:
        return L"-";
    case NodeKind::MUL:
        return L"*";
    case NodeKind::DIV:
        return L"/";
    case NodeKind::MOD:
        return L"%";
    case NodeKind::EQ:
        return L"==";
    case NodeKind::NE:
        return L"!=";
    case NodeKind::GT:
        return L">";
    case NodeKind::GE:
        return L">=";
    case NodeKind::LT:
        return L"<";
    case NodeKind::LE:
        return L"<=";
    case NodeKind::ASSIGN:
        return L"=";
    case NodeKind::ADD_ASSIGN:
        return L"+=";
    case NodeKind::SUB_ASSIGN:
        return L"-=";
    case NodeKind::MUL_ASSIGN:
        return L"*=";
    case NodeKind::DIV_ASSIGN:
        return L"/=";
    case NodeKind::MOD_ASSIGN:
        return L"%=";
    case NodeKind::LOGIC_AND:
        return L"&&";
    case NodeKind::LOGIC_OR:
        return L"||";
    default:
        throw std::logic_error("unknown nodekind specified");
    }
}

Node::Node()
{
}

void Node::push_outer_var(Var* var)
{
    auto find = std::find_if(this->outer_vars.begin(), this->outer_vars.end(), [var](auto each) {
        return each->name == var->name;
    });
    if (find == this->outer_vars.end())
    {
        this->outer_vars.push_back(var);
    }
}
