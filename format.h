#pragma once
#include <string>

std::wstring format(const wchar_t* fmt, ...);

std::wstring replace(std::wstring source, std::wstring target, std::wstring dest);

std::wstring tolower(const std::wstring& str);
