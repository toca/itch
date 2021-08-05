#include "format.h"
#include <cstdarg>
#include <cstdlib>
#include <algorithm>

std::wstring format(const wchar_t* fmt, ...)
{
    va_list arg;

    va_start(arg, fmt);

    std::wstring result;
    result.resize(1024);
    auto len = _vsnwprintf_s(result.data(), result.size(), result.size(), fmt, arg);
    if (len < 0)
    {
        printf("Error size not enough?");
        exit(1);
    }
    result.resize(len);
    va_end(arg);

    return result;
}

std::wstring replace(std::wstring source, std::wstring target, std::wstring dest)
{
    std::string::size_type  pos(source.find(target));

    while (pos != std::string::npos)
    {
        source.replace(pos, target.length(), dest);
        pos = source.find(target, pos + dest.length());
    }

    return source;
}

std::wstring tolower(const std::wstring& str)
{
    std::wstring result;
    result.resize(str.size());
    std::transform(str.begin(), str.end(), result.begin(), std::tolower);
    return result;
}
