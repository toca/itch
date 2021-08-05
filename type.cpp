#include "type.h"
#include "error.h"
#include <stdexcept>


Type::Type(Type::type t)
    :t(t)
{}

Type* Type::new_unknown()
{
    return new Type(type::UNKNOWN);
}

Type* Type::new_void()
{
    return new Type(type::VOID);
}

Type* Type::new_integer()
{
    return new Type(type::INT);
}

Type* Type::new_string()
{
    return new Type(type::STR);
}

Type* Type::new_boolean()
{
    return new Type(type::BOOL);
}

Type* Type::new_function()
{
    return new Type(type::FUNC);
}

bool Type::is_void()
{
    return this->t == type::VOID;
}

bool Type::is_integer()
{
    return this->t == type::INT;
}

bool Type::is_string()
{
    return this->t == type::STR;
}

bool Type::is_boolean()
{
    return this->t == type::BOOL;
}

bool Type::is_function()
{
    return this->t == type::FUNC;
}

std::wstring Type::to_string()
{
    switch (this->t)
    {
    case type::INT: return L"int";
    case type::STR: return L"string";
    case type::BOOL: return L"bool";
    case type::VOID: return L"void";
    case type::FUNC: return L"func";
    case type::UNKNOWN: return L"unknown";
    default: throw std::logic_error(__FUNCTION__ ": unknown type was specified");
    }
}

Type* Type::to_type(const std::wstring& str)
{
    if (str == L"int")
    {
        return new_integer();
    }
    else if (str == L"string")
    {
        return new_string();
    }
    else if (str == L"bool")
    {
        return new_boolean();
    }
    else if (str == L"void")
    {
        return new_void();
    }
    else if (str == L"func")
    {
        return new_function();
    }
    else
    {
        throw std::logic_error(__FUNCTION__ ": unknown type-string was specified");
    }
}


bool Type::validate(NodeKind kind)
{
    bool is_valid = false;
    switch (this->t)
    {
    case type::INT:
        if (kind == NodeKind::NEG) is_valid = true;
        break;
    case type::STR:
        break;
    case type::BOOL:
        if (kind == NodeKind::NOT) is_valid = true;
        break;
    case type::VOID:
        break;
    case type::UNKNOWN:
        throw std::logic_error(__FUNCTION__ ": unknown type specified");
    default:
        throw std::logic_error(__FUNCTION__ ": undefined type specified");
    }
    return is_valid;
}


bool Type::validate(NodeKind kind, Type* rhs)
{
    if (this->t != rhs->t)
    {
        return false;
    }

    bool is_valid = false;

    switch (this->t)
    {
    case type::INT:
        if (kind == NodeKind::ADD || kind == NodeKind::SUB || kind == NodeKind::MUL || kind == NodeKind::DIV || kind == NodeKind::MOD ||
            kind == NodeKind::LT || kind == NodeKind::LE || kind == NodeKind::GT || kind == NodeKind::GE ||
            kind == NodeKind::EQ || kind == NodeKind::NE ||
            kind == NodeKind::ASSIGN || kind == NodeKind::ADD_ASSIGN || kind == NodeKind::SUB_ASSIGN ||
            kind == NodeKind::MUL_ASSIGN || kind == NodeKind::DIV_ASSIGN || kind == NodeKind::MOD_ASSIGN)
        {
            is_valid = true;
        }
        break;
    case type::STR:
        if (kind == NodeKind::ADD || kind == NodeKind::ADD_ASSIGN ||
            kind == NodeKind::EQ || kind == NodeKind::NE || kind == NodeKind::ASSIGN)
        {
            is_valid = true;
        }
        break;
    case type::BOOL:
        if (kind == NodeKind::ASSIGN ||
            kind == NodeKind::LOGIC_AND || kind == NodeKind::LOGIC_OR ||
            kind == NodeKind::EQ || kind == NodeKind::NE)
        {
            is_valid = true;
        }
        break;
    case type::FUNC:
        if (kind == NodeKind::ASSIGN)
        {
            is_valid = true;
        }
        break;
    case type::VOID:
        break;
    case type::UNKNOWN:
        throw std::logic_error(__FUNCTION__ ": unknown type specified");

    default:
        throw std::logic_error(__FUNCTION__ ": unknown type specified");
    }
    return is_valid;
}








bool Type::operator==(const Type& rhs)
{
    if (this->t == type::FUNC && rhs.t == type::FUNC)
    {
        if ((*this->func_return) != (*rhs.func_return))
        {
            return false;
        }
        if (this->func_args.size() != rhs.func_args.size())
        {
            return false;
        }
        for (int i = 0; i < this->func_args.size(); i++)
        {
            if (*this->func_args[i] != *rhs.func_args[i])
            {
                return false;
            }
        }
        return true;
    }
    return this->t == rhs.t;
}

bool Type::operator!=(const Type& rhs)
{
	return !(*this == rhs);
}

