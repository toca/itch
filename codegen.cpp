#include "codegen.h"
#include "type.h"
#include "error.h"
#include "format.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <vector>
#include <string>

/* counter for variables, label, etc... */
int count = 0;
int indent = 0;
const wchar_t* TRUTHY = L"0==0";
const wchar_t* FALSY = L"0==1";


void codegen(Program* program)
{

    // informatin
    println(L"@REM +--------------------------------------------------------------+");
    println(L"@REM | This file was generated by itch https://github.com/toca/itch |");
    println(L"@REM +--------------------------------------------------------------+");

    // epilogue
    println(L"@REM ==== prologue ====================");
    println(L"@ECHO OFF");
    println(L"SETLOCAL");
    println(L"SET ERRORLEVEL=");
    println(L"@REM ==================================\n\n");

    // declar global variable
    println(L"@REM Global variables =================");
    gen_body(program->global_var, L"");
    println(L"@REM ==================================");
    
    // command-line arguments
    println(L"SET i=0");
    println(L":__ARGS_LOOP");
    println(L"CALL SET __arg%%i%%=%%~0");
    println(L"SHIFT");
    println(L"SET /A i+=1");
    println(L"CALL SET __temp_arg=%%0");
    println(L"IF defined __temp_arg GOTO :__ARGS_LOOP\n");

    // call main function
    println(L"CALL :main __main_result");
    println(L"EXIT /B %%__main_result%%\n");


    for(Func* cur_func = program->func; cur_func; cur_func = cur_func->next)
    {
        println(L"@REM func %s(%s): %s ====",
            cur_func->name.c_str(),
            arg2str(cur_func->args).c_str(),
            cur_func->type->to_string().c_str()
        );
        println(L":%s", cur_func->name.c_str());
        println(L"SETLOCAL");
        println(L"SET __FUNC_POST_PROC=");
        // args
        for (int i = 0; i < cur_func->args.size(); i++)
        {
            println(L"set \"%s=%%~%d\"", cur_func->args[i]->name.c_str(), i + 1);
        }
        indent++;
        
        // body
        gen_body(cur_func->body, cur_func->name.c_str());
        indent--;

        // end
        println(L":end_%s", cur_func->name.c_str());
        if (!cur_func->type->is_void())
        {
            println(L"CALL SET \"__POST_PROC=%%__FUNC_POST_PROC%%\"");
            print(L"ENDLOCAL & SET \"%%%d=%%__%s_result%%\"", cur_func->args.size() + 1, cur_func->name.c_str());
            for (auto var : cur_func->outer_vars)
            {
                print(L" & SET \"%s=%%%s%%\"", var->name.c_str(), var->name.c_str());
            }
            print(L" %%__POST_PROC%%");
            print(L"\n");
        }
        else
        {
            println(L"CALL SET \"__POST_PROC=%%__FUNC_POST_PROC%%\"");
            print(L"ENDLOCAL");
            for (auto var : cur_func->outer_vars)
            {
                print(L" & SET \"%s=%%%s%%\"", var->name.c_str(), var->name.c_str());
            }
            print(L" %%__POST_PROC%%");
            print(L"\n");
        }

        std::wstring func_post_proc = L"";
        for (auto var : cur_func->outer_vars)
        {
            func_post_proc = format(L" & SET %s=%%%%%s%%%%", var->name.c_str(), var->name.c_str());
        }
        
        println(L"IF \"%%__FUNC_POST_PROC%%\"==\"%s\" (", func_post_proc.c_str());
        println(L"    @REM do nothing");
        println(L") else (");
        println(L"    SET \"__FUNC_POST_PROC=%%__FUNC_POST_PROC%%%s\"", func_post_proc.c_str());
        println(L")");

        // end function
        println(L"EXIT /B\n");
    }

}


void gen_body(Node* node, const wchar_t* func_name)
{
    for (; node; node= node->next)
    {
        switch (node->kind)
        {
        case NodeKind::NIL:
            continue;
        case NodeKind::COMMENT:
            println(L"@REM %s", node->str_val.c_str());
            continue;
        case NodeKind::BLOCK:
            println(L"SETLOCAL");
            println(L"@REM {");
            indent++;
            gen_body(node->body, func_name);
            indent--;
            println(L"@REM }");
            indent--;
            print(L"ENDLOCAL");
            discharge(node->outer_vars);
            break;
        case NodeKind::RETURN:
        {
            if (node->lhs)
            {
                std::wstring result = gen_expr(node->lhs);
                if (node->lhs->type->is_integer())
                {
                    println(L"SET /A __%s_result=%s", func_name, result.c_str());
                }
                else
                {
                    println(L"SET \"__%s_result=%s\"", func_name, result.c_str());
                }
            }
            extrication(node->outer_vars, node->return_var_name, node->depth);
            println(L"GOTO :end_%s", func_name);
            break;
        }
        case NodeKind::IF:
        {
            /* -- if --
            * result = if_condition
            * if (!result) goto :end_if_label
            *     body....
            * :end_if_label
            */

            /* -- if else --
            * result = if_condition
            * if (!result) goto :else_label
            *     if_body...
            *     goto :end_if_laleb
            * :else_label
            *     else_body...
            * :end_if_label
            */

            println(L"@REM IF ----");
            println(L"SETLOCAL");
            indent++;
            std::wstring result = gen_expr(node->condition);
            std::wstring end_if_label = format(L"__END_IF_%d__", count++);

            if (node->else_stmt)
            {
                std::wstring else_label = format(L"__ELSE_%d__", count++);
                println(L"IF NOT %s GOTO :%s", result.c_str(), else_label.c_str());
                gen_body(node->body, func_name);
                println(L"GOTO :%s", end_if_label.c_str());
                println(L":%s", else_label.c_str());
                gen_body(node->else_stmt, func_name);
            }
            else
            {
                println(L"IF NOT %s GOTO :%s", result.c_str(), end_if_label.c_str());
                gen_body(node->body, func_name);
            }

            println(L":%s", end_if_label.c_str());
            indent--;
            print(L"ENDLOCAL");
            discharge(node->outer_vars);
            println(L"@REM END_IF ----\n");
            break;
        }
        case NodeKind::FOR:
        {
            // init
            // :begin-for
            // !condition ? goto end-for
            //   statements...
            //   ... ... ...
            // :continue-for
            // increment
            // :end-for
            //

            println(L"@REM FOR----");
            println(L"SETLOCAL");
            indent++;
            // for-init
            gen_expr(node->init);
            // begin-label
            println(L":%s", node->begin.c_str());
            // for-condition
            if (node->condition)
            {
                std::wstring cond = gen_expr(node->condition);
                println(L"IF NOT %s GOTO :%s", cond.c_str(), node->end.c_str());
            }
            // body----
            gen_body(node->body, func_name);
            // continue-label
            println(L":%s", node->cont.c_str());
            // for-increment
            gen_expr(node->increment);
            // loop
            println(L"GOTO :%s", node->begin.c_str());
            println(L":%s", node->end.c_str());
            indent--;
            print(L"ENDLOCAL");
            discharge(node->outer_vars);
            println(L"@REM END_FOR----");
            break;
        }
        case NodeKind::EXPR_STMT:
        {
            std::wstring result = gen_expr(node->lhs);
            //println(L"%s", result.c_str());
            break;
        }
        case NodeKind::ASSIGN:
        case NodeKind::ADD_ASSIGN:
        case NodeKind::SUB_ASSIGN:
        case NodeKind::MUL_ASSIGN:
        case NodeKind::DIV_ASSIGN:
        case NodeKind::MOD_ASSIGN:
            gen_expr(node);
            break;
        case NodeKind::LVAR: // declare
            println(L"SET %s=", node->name.c_str());
            break;
        case NodeKind::GOTO:
            extrication(node->outer_vars, node->return_var_name, node->depth);
            println(L"GOTO :%s", node->dest.c_str());
            break;
        default:
            fprintf(stderr, "Error invalid statement");
            exit(1);
            break;
        }
    }
}


std::wstring gen_expr(Node* node)
{
    if (!node)
    {
        return L"";
    }

    switch (node->kind)
    {
    case NodeKind::NUM:
        return std::to_wstring(node->int_val);
    case NodeKind::BOOL:
        return node->bool_val ? TRUTHY : FALSY;
    case NodeKind::STR:
        return node->str_val;
        //return replace(node->str_val.c_str(), L"\"", L"\"\"");
    case NodeKind::BAT:
        println(L"%s", node->str_val.c_str());
        return node->str_val;
    case NodeKind::LVAR:
        return node->name;
    case NodeKind::RVAR:
        return format(L"%%%s%%", node->name.c_str());
    case NodeKind::CALL:
    {
        std::wstring args = L"";
        for(auto var = node->args; var; var = var->next)
        {
            auto result = gen_expr(var);
            if (var->kind == NodeKind::RVAR)
            {
                // avoid variable expansion
                result = format(L"%%%s%%", result.c_str());
            }
            else if (var->kind == NodeKind::STR)
            {
                // on CALL "%" disapere
                result = replace(result, L"%%", L"%%%%");
            }
            args += format(L"\"%s\" ", result.c_str());
        }
        print(L"CALL :%s %s", node->name.c_str(), args.c_str());
        // if func has return val
        if (!node->type->is_void())
        {
            wprintf(L"__%s_result\n", node->name.c_str());
            return format(L"%%__%s_result%%", node->name.c_str());
        }
        else
        {
            wprintf(L"\n");
            return L"";
        }
    }
    case NodeKind::INDIRECT_CALL:
    {
        std::wstring args = L"";
        for (auto var = node->args; var; var = var->next)
        {
            auto result = gen_expr(var);
            if (var->kind == NodeKind::RVAR)
            {
                // avoid variable expansion
                result = format(L"%%%s%%", result.c_str());
            }
            else if (var->kind == NodeKind::STR)
            {
                // on CALL "%" disapere
                result = replace(result, L"%%", L"%%%%");
            }
            args += format(L"\"%s\" ", result.c_str());
        }
        print(L"CALL :%%%s%% %s", node->name.c_str(), args.c_str());
        // if func has return val
        if (!node->type->is_void())
        {
            wprintf(L"__%s_result\n", node->name.c_str());
            return format(L"%%__%s_result%%", node->name.c_str());
        }
        else
        {
            wprintf(L"\n");
            return L"";
        }
    }
    case NodeKind::FUNCTION:
        return node->func_name.c_str();
    }


    // unary
    std::wstring left_expr = gen_expr(node->lhs);
    
    if (node->kind == NodeKind::NEG)
    {
        return format(L"-%s", left_expr.c_str());
    }
    else if (node->kind == NodeKind::NOT)
    {
        println(L"IF %s ( SET \"__not_%d=0==1\" ) ELSE ( SET \"__not_%d=0==0\" )",
            left_expr.c_str(),
            count,
            count
        );
        return format(L"%%__not_%d%%", count++);
    }

    // binary
    std::wstring right_expr = gen_expr(node->rhs);


    switch (node->kind)
    {
    case NodeKind::ADD:
        if (node->type->is_integer())
        {
            println(L"SET /A \"__expr_res%d=(%s+%s)\"", count, left_expr.c_str(), right_expr.c_str());
        }
        else if (node->type->is_string())
        {
            println(L"SET \"__expr_res%d=%s%s\"", count, left_expr.c_str(), right_expr.c_str());
        }
        return format(L"%%__expr_res%d%%", count++);
    case NodeKind::SUB:
        println(L"SET /A \"__expr_res%d=(%s-%s)\"", count, left_expr.c_str(), right_expr.c_str());
        return format(L"%%__expr_res%d%%", count++);
    case NodeKind::MUL:
        println(L"SET /A \"__expr_res%d=(%s*%s)\"", count, left_expr.c_str(), right_expr.c_str());
        return format(L"%%__expr_res%d%%", count++);
    case NodeKind::DIV:
        println(L"SET /A \"__expr_res%d=(%s/%s)\"", count, left_expr.c_str(), right_expr.c_str());
        return format(L"%%__expr_res%d%%", count++);
    case NodeKind::MOD:
        println(L"SET /A \"__expr_res%d=%s%%%%%s\"",count, left_expr.c_str(), right_expr.c_str());
        return format(L"%%__expr_res%d%%", count++);
    case NodeKind::ASSIGN:
        if (node->type->is_integer())
        {
            println(L"SET /A \"%s=%s\"", left_expr.c_str(), right_expr.c_str());
        }
        else if (node->lhs->type->is_string())
        {
            if (node->rhs->kind == NodeKind::RVAR)
            {
                println(L"IF \"%s\"==\"\" ( SET \"%s=\" ) ELSE ( SET \"%s=%s\" )",
                    right_expr.c_str(),
                    left_expr.c_str(),
                    left_expr.c_str(),
                    right_expr.c_str()
                );
            }
            else
            {
                println(L"SET \"%s=%s\"", left_expr.c_str(), right_expr.c_str());
            }
        }
        else
        {
            println(L"SET \"%s=%s\"", left_expr.c_str(), right_expr.c_str());
        }
        return left_expr;
    case NodeKind::ADD_ASSIGN:
        if (node->type->is_integer())
        {
            println(L"SET /A \"%s+=%s\"", left_expr.c_str(), right_expr.c_str());
        }
        else if (node->type->is_string())
        {
            println(L"SET \"%s=%%%s%%%s\"", left_expr.c_str(), left_expr.c_str(), right_expr.c_str());
        }
        return left_expr;
    case NodeKind::SUB_ASSIGN:
        println(L"SET /A \"%s-=%s\"", left_expr.c_str(), right_expr.c_str());
        return left_expr;
    case NodeKind::MUL_ASSIGN:
        println(L"SET /A \"%s*=%s\"", left_expr.c_str(), right_expr.c_str());
        return left_expr;
    case NodeKind::DIV_ASSIGN:
        println(L"SET /A \"%s/=%s\"", left_expr.c_str(), right_expr.c_str());
        return left_expr;
    case NodeKind::MOD_ASSIGN:
        println(L"SET /A \"%s%%%%=%s\"", left_expr.c_str(), right_expr.c_str());
        return left_expr;
    case NodeKind::LOGIC_AND:
        println(L"SET __and_%d=0==0", count);
        println(L"IF NOT %s ( SET __and_%d=0==1 )", left_expr.c_str(), count);
        println(L"IF NOT %s ( SET __and_%d=0==1 )", right_expr.c_str(), count);
        return format(L"%%__and_%d%%", count++);
    case NodeKind::LOGIC_OR:
        println(L"SET __or_%d=0==1", count);
        println(L"IF %s ( SET __or_%d=0==0 )", left_expr.c_str(), count);
        println(L"IF %s ( SET __or_%d=0==0 )", right_expr.c_str(), count);
        return format(L"%%__or_%d%%", count++);
    case NodeKind::EQ:
        return comparison(L"EQU", node->lhs, node->rhs);
    case NodeKind::NE:
        return comparison(L"NEQ", node->lhs, node->rhs);
    case NodeKind::LT:
        return comparison(L"LSS", node->lhs, node->rhs);
    case NodeKind::LE:
        return comparison(L"LEQ", node->lhs, node->rhs);
    case NodeKind::GT:
        return comparison(L"GTR", node->lhs, node->rhs);
    case NodeKind::GE:
        return comparison(L"GEQ", node->lhs, node->rhs);
    default:
        fwprintf(stderr, L"gen: unexpected expression.");
        exit(0);
    }



    return L"";
}




std::wstring arg2str(const std::vector<Var*>& vars)
{
    std::wstring result = L"";
    for (Var* var : vars)
    {
        result += format(L", %s: %s", var->name.c_str(), var->type->to_string().c_str());
    }
    if (!result.empty())
    {
        result.erase(0, 2);
    }
    return result;
}

void println(const wchar_t* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    if (indent) wprintf(L"%*c", indent*4, L' ');
    vwprintf(fmt, arg);
    wprintf(L"\n");
    va_end(arg);
}

void print(const wchar_t* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    if (indent) wprintf(L"%*c", indent * 4, L' ');
    vwprintf(fmt, arg);
    va_end(arg);
}

std::wstring comparison(const std::wstring& op, Node* lhs, Node* rhs)
{
    std::wstring temp_var = format(L"__COMP_RESULT_%d__", count++);
    std::wstring left = calc(lhs);
    std::wstring right = calc(rhs);
    if (lhs->type->is_integer())
    {
        println(L"IF %s %s %s ( SET \"%s=0==0\" ) ELSE ( SET \"%s=0==1\")",
            left.c_str(),
            op.c_str(),
            right.c_str(),
            temp_var.c_str(),
            temp_var.c_str()
        );
    }
    else
    {
        println(L"IF \"%s\" %s \"%s\" ( SET \"%s=0==0\" ) ELSE ( SET \"%s=0==1\")",
            left.c_str(),
            op.c_str(),
            right.c_str(),
            temp_var.c_str(),
            temp_var.c_str()
        );
    }
    return format(L"%%%s%%", temp_var.c_str());
}


std::wstring calc(Node* node)
{
    std::wstring expr = gen_expr(node);
    if (node->type->is_integer())
    {
        println(L"SET /A \"__calc_result_%d=%s\"", count, expr.c_str());
    }
    else
    {
        println(L"SET \"__calc_result_%d=%s\"", count, expr.c_str());
    }
    return format(L"%%__calc_result_%d%%", count++);

}

void discharge(std::vector<Var*> vars, const std::wstring& return_var)
{
    for (auto var : vars)
    {
        wprintf(L" & SET \"%s=%%%s%%\"", var->name.c_str(), var->name.c_str());
    }
    if (!return_var.empty())
    {
        wprintf(L" & SET \"%s=%%%s%%\"", return_var.c_str(), return_var.c_str());
    }
    wprintf(L"\n");
}

void extrication(std::vector<Var*> vars, const std::wstring& return_var, int depth)
{
    std::wstring str = L"";
    for (int i = 0; i < depth; i++)
    {
        if (i == 0)
        {
            str += L"ENDLOCAL";
        }
        else
        {
            str += L" & ENDLOCAL";
        }
    }

    for (auto var : vars)
    {
        const wchar_t* opt = var->type->is_integer() ? L"/A" : L"";
        if (str.empty())
        {
            str += format(L"SET %s \"%s=%%%s%%\"", opt, var->name.c_str(), var->name.c_str());
        }
        else
        {
            str += format(L" & SET %s \"%s=%%%s%%\"",opt,  var->name.c_str(), var->name.c_str());
        }
    }
    if (!return_var.empty() && depth)
    {
        if (str.empty())
        {
            //str += format(L"SET /A \"%s=%%%s%%\"", return_var.c_str(), return_var.c_str());
            str += format(L"SET \"%s=%%%s%%\"", return_var.c_str(), return_var.c_str());
        }
        else
        {
            //str += format(L" & SET /A \"%s=%%%s%%\"", return_var.c_str(), return_var.c_str());
            str += format(L" & SET \"%s=%%%s%%\"", return_var.c_str(), return_var.c_str());
        }
    }
    if (!str.empty())
    {
        if (indent) wprintf(L"%*c", indent * 4, L' ');
        wprintf(L"%s\n", str.c_str());
    }
}
