#include "parser.h"
#include "node.h"
#include "tokens.h"
#include "parse_context.h"
#include "error.h"
#include "format.h"


Parser::Parser(Tokens& tokens, const std::wstring& filename, const std::wstring& source)
    :tokens(tokens), filename(filename), source(source), context(filename, source), func_cursor(nullptr)
{
}

/** parse */
Program* Parser::parse()
{
    Node* global_var = pre_proc();

    Func head{};
    this->func_cursor = &head;

    while (!tokens.is_end())
    {
        Token* token = tokens.front();
        if (token->value == L"func" && token->kind == TokenKind::KEYWORD)
        {
            this->add_func(function());
        }
        else
        {
            // top-level-object = func | global variable
            tokens.pop();
        }
    }
    this->func_cursor = head.next;
    post_proc(head.next);

    Program* program = new Program();
    program->func = head.next;
    program->global_var = global_var;
    return program;
}



/** 
* parse pre-process
*   collect func-definitions and global variables
*/
Node* Parser::pre_proc()
{
    Node head{};
    Node* cur = &head;

    // %ERRORLEVEL%
    Var* var = new Var();
    var->type = Type::new_integer();
    var->name = L"ERRORLEVEL";
    var->token = nullptr;
    var->kind = VarKind::CONST;
    context.define_var(var);

    // collect global variables & func-def
    while (!tokens.is_end())
    {
        Token* token = tokens.front();

        if (token->value == L"func" && token->kind == TokenKind::KEYWORD)
        {
            context.enter_scope();
            context.define_func(func_def());
            context.leave_scope();
            //skip function block
            int depth = 1;
            while (!tokens.is_end())
            {
                token = tokens.front();
                tokens.pop();

                if (token->value == L"{" && token->kind == TokenKind::PUNCT)
                {
                    depth++;
                }
                else if (token->value == L"}" && token->kind == TokenKind::PUNCT)
                {
                    depth--;
                }
                if (depth <= 0)
                {
                    break;
                }
            }
        }
        else if (this->is_variable(token))
        {
            cur->next = declaration();
            cur = cur->next;
        }
        else if (token->kind == TokenKind::COMMENT)
        {
            tokens.pop();
        }
        else
        {
            throw parse_error(
                this->filename.c_str(),
                this->source.c_str(),
                token->pos,
                L"unexpected token"
            );
        }
    }
    tokens.reset();
    return head.next;
}



/** parse post-prosess */
void Parser::post_proc(Func* func)
{
    for (Func* cur = func; cur; cur = cur->next)
    {
        this->solve_scope(cur);
    }
}




/**
* function = "func" func_name "(" params ")" type_spec "{" compound_statement
* params = param (',' param)*
*/
Func* Parser::function(bool anonymous)
{
    context.enter_scope();
    Func* func = func_def(anonymous);
    
    if (!func->type->is_void())
    {
        this->context.return_var_name(format(L"__%s_result", func->name.c_str()));
    }

    // func body
    func->body = compound_statement();

    // check return
    std::vector<Type*> return_types{};
    bool func_return_value = collect_return_types(func->body, &return_types);
    if (!func->type->is_void() && !func_return_value)
    {
        throw parse_error(this->filename, this->source, func->token->pos, format(L"funcion: '%s' missing return", func->name.c_str()));
    }
    for (auto each : return_types)
    {
        if (!func->type->is_void() && each->is_void())
        {
            throw parse_error(this->filename, this->source, func->token->pos, format(L"funcion: '%s' missing return", func->name.c_str()));
        }
        if ((*func->type) != (*each))
        {
            throw parse_error(this->filename, this->source, func->token->pos, format(L"funcion: '%s' return type mismatch", func->name.c_str()));
        }
    }

    func->outer_vars = context.get_outer_vars();
    context.leave_scope();
    return func;
}



/**
* create only function definition
*/
Func* Parser::func_def(bool anonymous)
{
    Token* token = tokens.consume(L"func");
    std::wstring name;
    if (anonymous)
    {
        name = this->unique_label(L"lambda");
    }
    else
    {
        token = tokens.try_consume(TokenKind::IDENT);
        if (!token) {
            throw parse_error(this->filename.c_str(), this->source.c_str(), tokens.front()->pos, L"function: name required\n");
        }
        name = token->value;
    }


    Func* func = new Func();
    func->name = name;
    func->token = token;

    // args
    tokens.consume(L"(");

    for (int i = 0; !tokens.try_consume(L")"); i++)
    {
        if (0 != i) tokens.consume(L",");
        if (8 <= i)
        {
            throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, L"func: number of arguments must be 8 or less.");
        }

        Var* var = param();
        func->args.push_back(var);
        context.define_var(var);
    }

    // type
    func->type = type_spec();

    // signature
    func->sig = Type::new_function();
    func->sig->func_return = func->type;
    for (auto var : func->args)
    {
        func->sig->func_args.push_back(var->type);
    }

    // end block
    tokens.consume(L"{");

    return func;
}




/**
* param = ident type_spec
*/
Var* Parser::param()
{
    Token* token = tokens.try_consume(TokenKind::IDENT);
    if (!token)
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), tokens.front()->pos, L"param: argument name required");
    }
    
    Type* type = type_spec();
    
    Var* var = new Var();
    var->name = token->value;
    var->type = type;
    var->token = token;

    return var;
}


void Parser::solve_scope(Func* func)
{
    std::vector<Var*> vars{};
    for (Node* node = func->body; node; node = node->next)
    {
        if (node->body)
        {
            vars = this->solve_vars(node->body);
            for (auto var : vars)
            {
                node->push_outer_var(var);
            }
            continue;
        }
    }
}


std::vector<Var*> Parser::solve_vars(Node* body)
{
    std::vector<Var*> vars{};
    for (Node* node = body; node; node = node->next)
    {
        std::vector<Node*> func_calls{};
        this->collect_func_call(node, &func_calls);
        for (Node* call : func_calls)
        {
            Func* func = this->find_func(call->name);
            if (!func) continue;
            for (Var* var : func->outer_vars)
            {
                vars.push_back(var);
            }
        }
        if (node->kind == NodeKind::RETURN || node->kind == NodeKind::GOTO)
        {
            for (Var* var : vars)
            {
                node->push_outer_var(var);
            }
        }
    }
    return vars;
}

void Parser::collect_func_call(Node* node, std::vector<Node*>* result)
{
    if (node->kind == NodeKind::CALL)
    {
        result->push_back(node);
    }
    if (node->lhs)
    {
        collect_func_call(node->lhs, result);
    }
    if (node->rhs)
    {
        collect_func_call(node->rhs, result);
    }
}



/**
* stmt = "return" expr? ";"
*      | "if"   "(" expr ")" stmt ("else" stmt)?
*      | "for"  "(" declaration? ";" expr? ";" assign? ")" statement
*      | "{" compound_stmt
*      | expr_stmt
*/
Node* Parser::statement()
{
    Token* token = tokens.front();

    if (token->value == L"return" && token->kind == TokenKind::KEYWORD)
    {
        tokens.pop();
        if (tokens.try_consume(L";"))
        {
            Node* node = new_node(NodeKind::RETURN, token);
            node->type = Type::new_void();
            node->depth = this->context.depth();
            node->outer_vars = this->context.get_outer_vars();
            return node;
        }
        else
        {
            Node* lhs = expr();
            tokens.consume(L";");

            Node* node = new_node(NodeKind::RETURN, token);
            node->type = lhs->type;
            node->lhs = lhs;
            node->depth = this->context.depth();
            node->outer_vars = this->context.get_outer_vars();
            node->return_var_name = this->context.return_var_name();
            return node;
        }
    }
    else if (token->value == L"if" && token->kind == TokenKind::KEYWORD)
    {
        tokens.pop();
        // scope
        this->context.enter_scope();

        tokens.consume(L"(");

        Node* node = new_node(NodeKind::IF, token);
        node->condition = expr();
        if (!node->condition->type->is_boolean())
        {
            throw parse_error(filename.c_str(), source.c_str(), token->next->pos, L"if: condition must be type 'bool'.");
        }

        tokens.consume(L")");

        if (!tokens.try_consume(L"{"))
        {
            throw parse_error(
                this->filename.c_str(),
                this->source.c_str(),
                tokens.front()->pos,
                L"statement: if-clause required '{'"
            );
        }
        node->body = compound_statement();

        // else statement
        if (tokens.try_consume(L"else"))
        {
            if (tokens.peek(L"if"))
            {
                node->else_stmt = statement();
            }
            else if (tokens.try_consume(L"{"))
            {
                node->else_stmt = compound_statement();
            }
            else
            {
                throw parse_error(
                    this->filename.c_str(),
                    this->source.c_str(),
                    tokens.front()->pos,
                    L"statement: else-clause required '{'"
                );
            }
        }

        // scope
        node->outer_vars = this->context.get_outer_vars();
        this->context.leave_scope();

        return node;
    }
    else if (token->value == L"for" && token->kind == TokenKind::KEYWORD)
    {
        tokens.pop();
        tokens.consume(L"(");

        // labels
        auto begin = unique_label(L"BEGIN_FOR");
        auto cont = unique_label(L"CONTINUE_FOR");
        auto end = unique_label(L"END_FOR");

        // scope
        this->context.enter_scope(cont, end);

        Node* node = new_node(NodeKind::FOR, token);
        if (!tokens.try_consume(L";"))
        {
            node->init = declaration();
        }
        if (!tokens.try_consume(L";"))
        {
            node->condition = expr();
            tokens.consume(L";");
        }
        if (!tokens.peek(L")"))
        {
            node->increment = assign();
        }

        node->begin = begin;
        node->cont = cont;
        node->end = end;

        tokens.consume(L")");

        if (!tokens.try_consume(L"{"))
        {
            throw parse_error(
                this->filename.c_str(),
                this->source.c_str(),
                tokens.front()->pos,
                L"statement: for-clause required '{'"
            );
        }
        node->body = compound_statement();


        // scope
        node->outer_vars = this->context.get_outer_vars();
        this->context.leave_scope();
 
        return node;
    }
    else if (token->value == L"continue" && token->kind == TokenKind::KEYWORD)
    {
        tokens.pop();
        Node* node = new_node(NodeKind::GOTO, token);
        std::wstring dest = context.get_continue_label();
        if (dest.empty())
        {
            throw parse_error(filename.c_str(), source.c_str(), token->pos, L"continue: stray continue.");
        }
        node->dest = dest;
        node->depth = this->context.depth()-1;
        node->outer_vars = this->context.get_outer_vars();
        return node;
    }
    else if (token->value == L"break" && token->kind == TokenKind::KEYWORD)
    {
        tokens.pop();
        Node* node = new_node(NodeKind::GOTO, token);
        std::wstring dest = context.get_end_label();
        if (dest.empty())
        {
            throw parse_error(filename.c_str(), source.c_str(), token->pos, L"break: stray break.");
        }
        node->dest = dest;
        node->depth = this->context.depth()-1;
        node->outer_vars = this->context.get_outer_vars();
        return node;
    }
    else if (this->is_variable(token))
    {
        return declaration();
    }
    else if (token->value == L"{" && token->kind == TokenKind::PUNCT)
    {
        tokens.pop();
        // scope
        this->context.enter_scope();

        Node* node = new_node(NodeKind::BLOCK, token);
        node->body = compound_statement();
        node->outer_vars = this->context.get_outer_vars();
        // scope
        this->context.leave_scope();
        return node;
    }
    else if (this->is_assign(token))
    {
        return assign_statement();
    }
    else if (token->kind == TokenKind::COMMENT)
    {
        tokens.pop();
        return new_comment_node(token);
    }
    else
    {
        Node* node = expr_statement();
        return node;
    }
}



/**
* compound_statement = stmt* "}"
*/
Node* Parser::compound_statement()
{
    Node head{};
    Node* cur = &head;
    while (!tokens.try_consume(L"}"))
    {
        cur->next = statement();
        cur = cur->next;
    }
    return head.next;
}



/**
* declaration = var_kind ident type_spec* ('=' equality)* ';'
* declarator = 'var' | 'const'
*/
Node* Parser::declaration()
{
    VarKind kind;
    if (tokens.try_consume(L"var")) {
        kind = VarKind::VAR;
    }
    else if (tokens.try_consume(L"const"))
    {
        kind = VarKind::CONST;
    }
    else
    {
        throw std::logic_error("not declaration");
    }



    Token* ident = tokens.try_consume(TokenKind::IDENT);
    if (!ident)
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), tokens.front()->pos, L"declaration: variable name required");
    }


    if (!tokens.peek(L":") && !tokens.peek(L"="))
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), tokens.front()->pos, L"declaration: type required");
    }

    Var* var = new Var();
    if (tokens.peek(L":"))
    {
        var->type = type_spec();
    }

    var->name = ident->value;
    var->token = ident;
    var->kind = kind;

    context.define_var(var);

    switch (kind)
    {
    case VarKind::CONST:
    {
        tokens.consume(L"=");
        Node* rhs = equality();
        // shorthand type
        if (!var->type)
        {
            var->type = rhs->type;
        }
        Node* node = new_const_lvar(ident);
        node = new_binary(NodeKind::ASSIGN, node, rhs, ident);
        tokens.consume(L";");
        return node;
    }
    case VarKind::VAR:
    {
        if (tokens.try_consume(L";"))
        {
            if (!var->type)
            {
                throw parse_error(this->filename, this->source, ident->pos, L"var type required");
            }
            // default value
            Node* rhs = nullptr;
            if (var->type->is_integer())
            {
                rhs = new_node(NodeKind::NUM, nullptr);
                rhs->int_val = 0;
                rhs->type = Type::new_integer();
            }
            else if (var->type->is_boolean())
            {
                rhs = new_node(NodeKind::BOOL, nullptr);
                rhs->bool_val = false;
                rhs->type = Type::new_boolean();
            }
            else if (var->type->is_string())
            {
                rhs = new_node(NodeKind::STR, nullptr);
                rhs->str_val = L"";
                rhs->type = Type::new_string();
            }
            else
            {
                throw parse_error(this->filename.c_str(), this->source.c_str(), nullptr, L"declaration: unknown type");
            }
            Node* node = new_lvar_node(ident);
            node = new_binary(NodeKind::ASSIGN, node, rhs, ident);
            return node;
        }
        else
        {
            tokens.consume(L"=");
            Node* rhs = equality();
            // shorthand type
            if (!var->type)
            {
                var->type = rhs->type;
            }
            Node* node = new_lvar_node(ident);
            node = new_binary(NodeKind::ASSIGN, node, rhs, ident);
            tokens.consume(L";");
            return node;
        }
    default:
        throw std::logic_error("unknown VarKind");
    }
    }
}



/**
* expr_statement = expr? ;
*/
Node* Parser::expr_statement()
{
    if (Token* token = tokens.try_consume(L";"))
    {
        Node* node = new_node(NodeKind::NIL, token);
        node->type = Type::new_void();
        return node;
    }
    
    Node* lhs = expr();
    Node* node = new_node(NodeKind::EXPR_STMT, lhs->token);
    node->lhs = lhs;
    tokens.consume(L";");
    return node;
}



/**
* assign_statement = assign ";"
*/
Node* Parser::assign_statement()
{
    Node* node = assign();
    tokens.consume(L";");
    return node;
}



/**
* assign = lvar "=" expr
*        | lvar ( "+" | "-" | "*" | "/" | "%" ) "=" expr
*        | lvar ( "++" | "--" )
*/
Node* Parser::assign()
{
    Token* token = tokens.try_consume(TokenKind::IDENT);
    if (!token)
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), tokens.front()->pos, L"assign: not assignable.");
    }
 
    // increment or decrement
    Node* lhs = new_lvar_node(token);
    if (Token* token = tokens.try_consume(L"++"))
    {
        return new_binary(NodeKind::ADD_ASSIGN, lhs, new_num_node(1, token), token);
    }
    else if (Token* token = tokens.try_consume(L"--"))
    {
        return new_binary(NodeKind::SUB_ASSIGN, lhs, new_num_node(1, token), token);
    }


    NodeKind kind;
    if (tokens.try_consume(L"="))
        kind = NodeKind::ASSIGN;
    else if (tokens.try_consume(L"+="))
        kind = NodeKind::ADD_ASSIGN;
    else if (tokens.try_consume(L"-="))
        kind = NodeKind::SUB_ASSIGN;
    else if (tokens.try_consume(L"*="))
        kind = NodeKind::MUL_ASSIGN;
    else if (tokens.try_consume(L"/="))
        kind = NodeKind::DIV_ASSIGN;
    else if (tokens.try_consume(L"%="))
        kind = NodeKind::MOD_ASSIGN;
    else throw parse_error(
        this->filename.c_str(),
        this->source.c_str(),
        token->pos,
        L"assign: expected assign operator"
    );
    

    Node* rhs = expr();
    Node* node = new_binary(kind, lhs, rhs, token);
    return node;
}



/**
* expr = logical_or
*/
Node* Parser::expr()
{
    return logical_or();
}


/**
* logical_or = logical_and ("||" logical_and)*
*/
Node* Parser::logical_or()
{
    Node* node = logical_and();
    while (Token* token = tokens.try_consume(L"||"))
    {
        Node* rhs = logical_and();
        node = new_binary(NodeKind::LOGIC_OR, node, rhs, token);
    }
    return node;
}


/**
* logical_and = equality ("&&" equality)*
*/
Node* Parser::logical_and()
{
    Node* node = equality();
    while (Token* token = tokens.try_consume(L"&&"))
    {
        Node* rhs = equality();
        node = new_binary(NodeKind::LOGIC_AND, node, rhs, token);
    }
    return node;
}



/**
* equality = relational ("==" relational | "!=" relational)*
*/
Node* Parser::equality()
{
    Node* node = relational();
    while (true)
    {
        if (Token* token = tokens.try_consume(L"=="))
        {
            Node* rhs = relational();
            node = new_bool_binary(NodeKind::EQ, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L"!="))
        {
            Node* rhs = relational();
            node = new_bool_binary(NodeKind::NE, node, rhs, token);
            continue;
        }
        return node;
    }
}



/**
* relational = add ("<" add | "<=" add | ">" add | ">=" add)*
*/
Node* Parser::relational()
{
    Node* node = add();
    while (true)
    {
        if (Token* token = tokens.try_consume(L"<"))
        {
            Node* rhs = add();
            node = new_bool_binary(NodeKind::LT, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L"<="))
        {
            Node* rhs = add();
            node = new_bool_binary(NodeKind::LE, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L">"))
        {
            Node* rhs = add();
            node = new_bool_binary(NodeKind::GT, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L">="))
        {
            Node* rhs = add();
            node = new_bool_binary(NodeKind::GE, node, rhs, token);
            continue;

        }
        return node;
    }
}



/**
* add = mul ("+" mul | "-" mul)*
*/
Node* Parser::add()
{
    Node* node = mul();
    while (true)
    {
        if (Token* token = tokens.try_consume(L"+"))
        {
            Node* rhs = mul();
            node = new_binary(NodeKind::ADD, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L"-"))
        {
            Node* rhs = mul();
            node = new_binary(NodeKind::SUB, node, rhs, token);
            continue;
        }
        return node;
    }
}



/**
* mul = unary ("*" unary | "/" unary | "%" unary)*
*/
Node* Parser::mul()
{
    auto node = unary();
    while (true)
    {
        if (Token* token = tokens.try_consume(L"*"))
        {
            Node *rhs = unary();
            node = new_binary(NodeKind::MUL, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L"/"))
        {
            Node* rhs = unary();
            node = new_binary(NodeKind::DIV, node, rhs, token);
            continue;
        }
        else if (Token* token = tokens.try_consume(L"%"))
        {
            Node* rhs = unary();
            node = new_binary(NodeKind::MOD, node, rhs, token);
            continue;
        }
        return node;
    }
}



/**
* unary = ("+" | "-" | "!") unary
*       | postfix
*/
Node* Parser::unary()
{
    Token* token = tokens.front();
    if (token->kind == TokenKind::PUNCT)
    {
        if (token->value == L"+")
        {
            tokens.pop();
            return unary();
        }
        else if (token->value == L"-")
        {
            tokens.pop();
            Node* node = unary();
            return new_unary(NodeKind::NEG, node, token);
        }
        else if (token->value == L"!")
        {
            tokens.pop();
            Node* node = unary();
            return new_unary(NodeKind::NOT, node, token);
        }
    }
    return postfix();
}



/**
* postfix = primary
*/
Node* Parser::postfix()
{
    return primary();
}



/**
* primary = "(" expr ")" | ident func-args? | num | bool
*/
Node* Parser::primary()
{
    if (tokens.try_consume(L"("))
    {
        Node* node = expr();
        tokens.consume(L")");
        return node;
    }

    Token* token = tokens.front();
    if (token->kind == TokenKind::IDENT)
    {
        // function call
        if (token->next->value == L"(")
        {
            return func_call();
        }
        else
        {
            tokens.pop();
            return new_rvar_node(token);
        }
    }

    if (token->kind == TokenKind::NUM)
    {
        tokens.pop();
        return new_num_node(std::stoi(token->value), token);
    }

    if (token->kind == TokenKind::BOOL)
    {
        tokens.pop();
        return new_bool_node(token);
    }

    if (token->kind == TokenKind::STR)
    {
        tokens.pop();
        return new_string_node(token);
    }

    if (token->kind == TokenKind::BAT)
    {
        tokens.pop();
        return new_bat_node(token);
    }
    if (token->value == L"func" && token->kind == TokenKind::KEYWORD)
    {
        Func* func = function(true);
        this->add_func(func);
        Node* node = new_node(NodeKind::FUNCTION, token);
        node->type = Type::new_function();
        node->type->func_return = func->type;
        node->func_name = func->name;
        for (auto arg : func->args)
        {
            node->type->func_args.push_back(arg->type);
        }

        return node;
    }

    throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, format(L"primary: Unexpected token '%s'", token->value.c_str()));
}



/**
* func_call = ident "(" func_args ")"
*/
Node* Parser::func_call()
{
    std::vector<Type*> func_args;
    Type* func_type;
    std::wstring func_name;
    NodeKind call_kind = NodeKind::CALL;

    Token* token = tokens.try_consume(TokenKind::IDENT);
    if (token == nullptr)
    {
        throw parse_error(this->filename, this->source, tokens.front()->pos, L"func_call: func name required.");
    }

    // normal function
    if (Func* func = context.find_func_def(token->value))
    {
        func_args = func->sig->func_args;
        func_type = func->type;
        func_name = func->name;
    }
    // func object
    else if(Var* var = context.find_var(token->value))
    {
        if (!var->type->is_function())
        {
            throw parse_error(this->filename, this->source, tokens.front()->pos, format(L"func_call: '%s' is not callable.", token->value.c_str()));
        }
        func_type = var->type->func_return;
        func_args = var->type->func_args;
        func_name = format(L"%s", var->name.c_str());
        call_kind = NodeKind::INDIRECT_CALL;
    }
    else
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, format(L"func_call: undefined identifire '%s'.", token->value.c_str()));
    }


    tokens.consume(L"(");

    // args
    Node head{};
    Node* cur = &head;
    size_t num_args = 0;
    while (!tokens.try_consume(L")"))
    {
        if (cur != &head)
        {
            tokens.consume(L",");
        }
        cur->next = expr();
        cur = cur->next;
        num_args++;


        // argument check
        if (func_args.size() < num_args)
        {
            throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, format(L"func_call: Too many arguments.", token->value.c_str()));
        }
        if ((*func_args[num_args-1]) != (*cur->type))
        {
            throw parse_error(this->filename, this->source, token->pos,
                format(
                    L"func_call: Argument type mismatch. expect '%s', actual '%s'",
                    func_args[num_args-1]->to_string().c_str(),
                    cur->type->to_string().c_str()
                )
            );
        }
    }
    if (num_args < func_args.size())
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, L"func_call: Too few arguments");
    }


    Node* node = new_node(call_kind, token);
    node->name = func_name;
    node->args = head.next;
    node->type = func_type;

    return node;
}



/* lvar node*/
Node* Parser::lvar()
{
    Token* token = tokens.front();
    if (token->kind != TokenKind::IDENT)
    {
        throw parse_error(
            this->filename.c_str(),
            this->source.c_str(),
            token->pos,
            L"expression is not assignable"
        );
    }
    tokens.pop();
    return new_lvar_node(token);
}


/**
* type_spec = ":" ("int" | "bool" | "string" | "void" | ("func" func-sig) )
*/
Type* Parser::type_spec()
{
    Token* p = tokens.consume(L":");
    return this->type();
}


Type* Parser::type()
{
    Token* token = tokens.try_consume(TokenKind::KEYWORD);
    if (!token)
    {
        throw parse_error(
            this->filename.c_str(),
            this->source.c_str(),
            tokens.front()->pos,
            L"type: required"
        );
    }

    if (token->value == L"func")
    {
        return this->func_sig();

    }
    else
    {
        return Type::to_type(token->value);
    }
}

Type* Parser::func_sig()
{
    auto pos = tokens.front()->pos;
    
    tokens.consume(L"(");

    Type* type = Type::new_function();
    for (int i = 0; !tokens.try_consume(L")"); i++)
    {
        if (0 != i) tokens.consume(L",");
        if (8 <= i)
        {
            throw parse_error(this->filename.c_str(), this->source.c_str(), pos, L"func: number of arguments must be 8 or less.");
        }

        Type* arg_type = this->type();
        type->func_args.push_back(arg_type);
    }
    type->func_return = this->type_spec();
    return type;
}



void Parser::validate_operation(Node* node, NodeKind kind)
{
    if (!node->type->validate(kind))
    {
        throw parse_error(this->filename, this->source, node->token->pos, format(L"invalid op '%s' to %s",
            kind2op(kind).c_str(), node->type->to_string().c_str()
        ));
    }
}


void Parser::validate_operation(Node* lhs, Node* rhs, NodeKind kind)
{
    if (!lhs->type->validate(kind, rhs->type))
    {
        throw parse_error(this->filename, this->source, lhs->token->pos, format(L"invalid op '%s' to %s and %s",
            kind2op(kind).c_str(), lhs->type->to_string().c_str(), lhs->type->to_string().c_str()
        ));
    }
}




Node* Parser::new_node(NodeKind kind, Token* token)
{
    Node* node = new Node();
    node->kind = kind;
    node->token = token;
    return node;
}

Node* Parser::new_binary(NodeKind kind, Node* lhs, Node* rhs, Token* token)
{
    Node* node = new_node(kind, token);
    node->lhs = lhs;
    node->rhs = rhs;
    node->type = lhs->type;
    node->func_name = lhs->func_name;
    
    validate_operation(node->lhs, node->rhs, node->kind);

    return node;
}

Node* Parser::new_bool_binary(NodeKind kind, Node* lhs, Node* rhs, Token* token)
{
    Node* node = new_binary(kind,lhs, rhs, token);
    node->type = Type::new_boolean();
    return node;
}

Node* Parser::new_unary(NodeKind kind, Node* lhs, Token* token)
{
    validate_operation(lhs, kind);

    Node* node = new_node(kind, token);
    node->lhs = lhs;
    node->type = lhs->type;
    return node;
}

Node* Parser::new_lvar_node(Token* token)
{
    Var* var = context.find_var(token->value);
    if (!var)
    {
        throw parse_error(this->filename, this->source, token->pos, format(L"undefined identifire '%s'", token->value.c_str()));
    }
    if (var->kind == VarKind::CONST)
    {
        throw parse_error(this->filename, this->source, token->pos, L"cannot assign to const var");
    }
    if (!var->is_local)
    {
        this->context.push_outer_var(var);
    }
    Node* node = new_node(NodeKind::LVAR, token);
    node->name = token->value;
    node->type = var->type;
    return node;
}

Node* Parser::new_rvar_node(Token* token)
{
    if (Var* var = context.find_var(token->value))
    {
        Node* node = new_node(NodeKind::RVAR, token);
        node->type = var->type;
        node->name = token->value;
        return node;
    }
    else if (Func* func = context.find_func_def(token->value))
    {
        Node* node = new_node(NodeKind::FUNCTION, token);
        node->type = func->sig;
        node->func_name = func->name;
        node->name = func->name;
        return node;
    }
    else
    {
        throw parse_error(this->filename.c_str(), this->source.c_str(), token->pos, format(L"undefined identifire '%s'", token->value.c_str()));
    }
}

Node* Parser::new_num_node(int num, Token* token)
{
    Node* node = new_node(NodeKind::NUM, token);
    node->type = Type::new_integer();
    node->int_val = num;
    return node;
}

Node* Parser::new_bool_node(Token* token)
{
    Node* node = new_node(NodeKind::BOOL, token);
    node->type = Type::new_boolean();
    node->bool_val = token->value == L"true";
    return node;
}

Node* Parser::new_string_node(Token* token)
{
    Node* node = new_node(NodeKind::STR, token);
    node->type = Type::new_string();
    node->str_val = token->value;
    return node;
}

Node* Parser::new_bat_node(Token* token)
{
    Node* node = new_node(NodeKind::BAT, token);
    node->type = Type::new_void();
    node->str_val = token->value;
    return node;
}

Node* Parser::new_comment_node(Token* token)
{
    Node* node = new_node(NodeKind::COMMENT, token);
    node->type = Type::new_void();
    node->str_val = token->value;
    return node;
}

Node* Parser::new_const_lvar(Token* token)
{
    Var* var = context.find_var(token->value);
    if (!var)
    {
        throw parse_error(this->filename, this->source, token->pos, format(L"undefined identifire '%s'", token->value.c_str()));
    }
    if (!var->is_local)
    {
        this->context.push_outer_var(var);
    }
    Node* node = new_node(NodeKind::LVAR, token);
    node->name = token->value;
    node->type = var->type;
    return node;
}

void Parser::add_func(Func* func)
{
    this->func_cursor->next = func;
    this->func_cursor = this->func_cursor->next;
}

Func* Parser::find_func(const std::wstring& name)
{
    for (Func* func = this->func_cursor; func; func = func->next) {
        if (func->name == name) {
            return func;
        }
    }
    return nullptr;
}

bool Parser::is_assign(Token* token)
{
    if (token->next->kind != TokenKind::PUNCT)
    {
        return false;
    }
    if (token->kind != TokenKind::IDENT)
    {
        return false;
    }
    return 
        token->next->value == L"="  || token->next->value == L"%=" ||
        token->next->value == L"+=" || token->next->value == L"-=" ||
        token->next->value == L"*=" || token->next->value == L"/=" ||
        token->next->value == L"++" || token->next->value ==L"--";

}

bool Parser::is_variable(Token* token)
{
    if (token->kind != TokenKind::KEYWORD)
    {
        return false;
    }
    return token->value == L"var" || token->value == L"const" || token->value == L"use";
}

std::wstring Parser::unique_label(const std::wstring& prefix)
{
    static int count = 0;
    return format(L"__%s_%d", prefix.c_str(), count++);
}


bool Parser::collect_return_types(Node* node, std::vector<Type*>* types)
{
    // TODO fix type class
    for (Node* cur = node; cur; cur = cur->next) 
    {
        // end
        if (cur->kind == NodeKind::RETURN)
        {
            types->push_back(cur->type);
            // branch return value
            return true;
        }
        // continue
        if (cur->kind == NodeKind::FOR)
        {
            // unkown yet
            collect_return_types(cur->body, types);
        }
        if (cur->kind == NodeKind::IF)
        {
            if (cur->else_stmt)
            {
                if (collect_return_types(cur->body, types) & collect_return_types(cur->else_stmt, types))
                {
                    // if, else both return value
                    return true;
                }
            }
            else
            {
                collect_return_types(cur->body, types);
            }
        }
    }
    return false;
}

