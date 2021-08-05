#include "error.h"
#include "format.h"
#include <string>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>


void printerr(const wchar_t* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    vfwprintf(stderr, fmt, arg);
    fwprintf(stderr, L"\n");
    va_end(arg);
}




parse_error::parse_error(const std::wstring& filename, const std::wstring& source, const wchar_t* pos, const std::wstring& message)
{
    this->set_message(filename.c_str(), source.c_str(), pos, message.c_str());
}


parse_error::parse_error(const std::wstring& filename, const std::wstring& source, const wchar_t* pos, const wchar_t* message)
{
    this->set_message(filename.c_str(), source.c_str(), pos, message);
}


parse_error::parse_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message)
{
    this->set_message(file, source, pos, message);
}


parse_error::parse_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const std::wstring& message)
{
    this->set_message(file, source, pos, message.c_str());
}


char const* parse_error::what() const
{
    return this->narrow_message.c_str();
}


void parse_error::set_message(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message)
{
    wchar_t* line_head = (wchar_t*)pos;
    while (source < line_head && line_head[-1] != L'\n')
    {
        line_head--;
    }

    wchar_t* line_tail = (wchar_t*)pos;
    while (*line_tail != L'\n')
    {
        line_tail++;
    }

    int line_num = 1;
    for (wchar_t* p = (wchar_t*)source; p < line_head; p++)
    {
        if (*p == '\n')
        {
            line_num++;
        }
    }

    std::wstring line(line_head, line_tail);
    this->message = format(L"%s:%d:", file, line_num);
    int indent = (int)this->message.size();
    this->message += format(L"%s\n", line.c_str());


    int error_pos = int(pos - line_head + indent);
    this->message += format(L"%*s", error_pos, L" ");
    this->message += format(L"^--- %s\n", message);

    size_t size = 0;
    ::wcstombs_s(&size, nullptr, 0, this->message.c_str(), 0);
    this->narrow_message = std::string(size, '\0');
    ::wcstombs_s(
        &size,
        this->narrow_message.data(),
        this->narrow_message.size(),
        this->message.c_str(),
        this->narrow_message.size()
    );
}






tokenize_error::tokenize_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const wchar_t* message)
{
    wchar_t* line_head = (wchar_t*)pos;
    while (source < line_head && line_head[-1] != L'\n')
    {
        line_head--;
    }

    wchar_t* line_tail = (wchar_t*)pos;
    while (*line_tail != L'\n')
    {
        line_tail++;
    }

    int line_num = 1;
    for (wchar_t* p = (wchar_t*)source; p < line_head; p++)
    {
        if (*p == '\n')
        {
            line_num++;
        }
    }

    std::wstring line(line_head, line_tail);
    this->message = format(L"%s:%d:", file, line_num);
    int indent = (int)this->message.size();
    this->message += format(L"%s\n", line.c_str());


    int error_pos = int(pos - line_head + indent);
    this->message += format(L"%*s", error_pos, L" ");
    this->message += format(L"^---%s\n", message);

    size_t size = 0;
    ::wcstombs_s(&size, nullptr, 0, this->message.c_str(), 0);
    this->narrow_message = std::string(size, '\0');
    ::wcstombs_s(
        &size,
        this->narrow_message.data(),
        this->narrow_message.size(),
        this->message.c_str(),
        this->narrow_message.size()
    );
}


tokenize_error::tokenize_error(const wchar_t* file, const wchar_t* source, const wchar_t* pos, const std::wstring& message)
{
    wchar_t* line_head = (wchar_t*)pos;
    while (source < line_head && line_head[-1] != L'\n')
    {
        line_head--;
    }

    wchar_t* line_tail = (wchar_t*)pos;
    while (*line_tail != L'\n')
    {
        line_tail++;
    }

    int line_num = 1;
    for (wchar_t* p = (wchar_t*)source; p < line_head; p++)
    {
        if (*p == '\n')
        {
            line_num++;
        }
    }

    std::wstring line(line_head, line_tail);
    this->message = format(L"%s:%d:", file, line_num);
    int indent = (int)this->message.size();
    this->message += format(L"%s\n", line.c_str());


    int error_pos = int(pos - line_head + indent);
    this->message += format(L"%*s", error_pos, L" ");
    this->message += format(L"^---%s\n", message.c_str());

    size_t size = 0;
    ::wcstombs_s(&size, nullptr, 0, this->message.c_str(), 0);
    this->narrow_message = std::string(size, '\0');
    ::wcstombs_s(
        &size,
        this->narrow_message.data(),
        this->narrow_message.size(),
        this->message.c_str(),
        this->narrow_message.size()
    );
}


char const* tokenize_error::what() const
{
    return this->narrow_message.c_str();
}
