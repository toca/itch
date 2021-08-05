#include "tokenizer.h"
#include "tokens.h"
#include "error.h"
#include <array>


Tokenizer::Tokenizer(const std::wstring& input, const std::wstring& filename)
	:input(input), filename(filename)
{

}


Tokens Tokenizer::tokenize()
{
	Tokens tokens(this->input, this->filename);
	wchar_t* str = (wchar_t*)this->input.c_str();

	Token head{};
	Token* prev = &head;
	while (*str) 
	{
		// skip space
		if (::iswspace(*str))
		{
			str++;
			continue;
		}

		// string literal
		if (*str == L'"')
		{
			std::wstring value = read_string_literal(str);
			tokens.push(new_token(TokenKind::STR, value, str));
			str += value.size() + 2; // "...."
			continue;
		}

		// batch (command) literal
		if (*str == L'`')
		{
			wchar_t* pos = str;
			std::wstring value = read_bat_literal(str, &str);
			tokens.push(new_token(TokenKind::BAT, value, pos));
			continue;

		}
		// hexadecimal
		if (starts_with(str, L"0x"))
		{
			str += 2; // "0x"
			wchar_t* pos = str;
			uint32_t value = ::wcstoul(str, &str, 16);
			tokens.push(new_token(TokenKind::NUM, std::to_wstring(value), pos));
			continue;
		}
		// octal
		if (starts_with(str, L"0"))
		{
			str += 1; // "0"
			wchar_t* pos = str;
			uint32_t value = ::wcstoul(str, &str, 8);
			tokens.push(new_token(TokenKind::NUM, std::to_wstring(value), pos));
			continue;

		}
		// integer literal
		if (::iswdigit(*str)) {
			wchar_t* pos = str;
			uint32_t value = ::wcstoul(str, &str, 10);
			tokens.push(new_token(TokenKind::NUM, std::to_wstring(value), pos));
			continue;
		}


		// comment
		if (wcsncmp(str, L"//", 2) == 0)
		{
			std::wstring value = read_comment(str);
			tokens.push(new_token(TokenKind::COMMENT, value, str));
			str += value.size() + 2; // 2 => '//'
			continue;
		}

		// punctuator
		std::wstring punct = fetch_punct(str);
		if (!punct.empty())
		{
			tokens.push(new_token(TokenKind::PUNCT, punct, str));
			str += punct.size();
			continue;
		}

		// bool literal
		if (is_bool(str))
		{
			std::wstring value = fetch_bool(str);
			tokens.push(new_token(TokenKind::BOOL, value, str));
			str += value.size();
			continue;
		}
		
		// keyword
		std::wstring keyword = fetch_keyword(str);
		if (!keyword.empty())
		{
			tokens.push(new_token(TokenKind::KEYWORD, keyword, str));
			str += keyword.size();
			continue;
		}

		// ident
		if (is_first_ident(*str))
		{
			std::wstring value = fetch_ident(str);
			tokens.push(new_token(TokenKind::IDENT, value, str));
			str += value.size();
			continue;
		}


		// on error
		throw tokenize_error(filename.c_str(), this->input.c_str(), str, L"Error unexpected token! %s\n");
	}
	Token* token = new Token();
	token->kind = TokenKind::END_OF_TOKEN;
	token->pos = str - 1;
	tokens.push(token);
	return tokens;
}



Token* Tokenizer::new_token(TokenKind kind, const std::wstring& value, const wchar_t* pos)
{
	Token* token = new Token();
	token->kind = kind;
	token->value = value;
	token->pos = pos;
	return token;
}


bool Tokenizer::starts_with(wchar_t* str, const wchar_t* pre)
{
	return ::wcsncmp(str, pre, wcslen(pre)) == 0;
}


bool Tokenizer::is_first_ident(wchar_t c)
{
	if ('a' <= c && c <= 'z')
	{
		return true;
	}
	if ('A' <= c && c <= 'Z')
	{
		return true;
	}
	if (c == '_')
	{
		return true;
	}
	return false;
}


bool Tokenizer::is_ident(wchar_t c)
{
	if ('a' <= c && c <= 'z')
	{
		return true;
	}
	if ('A' <= c && c <= 'Z')
	{
		return true;
	}
	if (c == '_')
	{
		return true;
	}
	if ('0' <= c && c <= '9')
	{
		return true;
	}
	return false;
}


bool Tokenizer::is_bool(wchar_t* str)
{
	if (starts_with(str, L"true") && !::iswalnum(*(str + 4))) return true;
	if (starts_with(str, L"false") && !::iswalnum(*(str + 5))) return true;
	return false;

}


std::wstring Tokenizer::fetch_ident(wchar_t* str)
{
	wchar_t* begin = str;
	wchar_t* end = str + 1;
	for (int i = 0; i < 255; i++)
	{
		if (is_ident(*end))
		{
			end++;
		}
		else 
		{
			break;
		}
	}
	return std::wstring(begin, end);
}


std::wstring Tokenizer::fetch_keyword(wchar_t* str)
{
	static std::array<std::wstring, 13> keywords{ L"return", L"if", L"else", L"for", L"break", L"continue", L"func", L"var", L"const", L"void", L"int", L"bool", L"string"};
	for (auto each : keywords)
	{
		if (
			starts_with(str, each.c_str()) && *(str + each.size()) != L'_' &&
			(::iswpunct(*(str + each.size())) || ::iswblank(*(str + each.size())))
		)
		{
			return each;
		}
	}
	return L"";
}


std::wstring Tokenizer::fetch_punct(wchar_t* str)
{
	if (starts_with(str, L"==") || starts_with(str, L"!=") || starts_with(str, L"<=") || starts_with(str, L">=") ||
		starts_with(str, L"+=") || starts_with(str, L"-=") || starts_with(str, L"*=") || starts_with(str, L"/=") ||
		starts_with(str, L"%=") || 
		starts_with(str, L"++") || starts_with(str, L"--") ||
		starts_with(str, L"&&") || starts_with(str, L"||")
	) {
		return std::wstring(str, str+2);
	}
	else if (iswpunct(*str) && *str != L'_')
	{
		return std::wstring(1, *str);
	}
	else
	{
		return std::wstring(L"");
	}
}


std::wstring Tokenizer::fetch_bool(wchar_t* str)
{
	if (starts_with(str, L"true")) return L"true";
	if (starts_with(str, L"false")) return L"false";
	return L"";
}


std::wstring Tokenizer::read_string_literal(wchar_t* start) {
	if (*start == L'"')
	{
		start++;
	}
	else
	{
		throw std::logic_error("string literal not starts with \"");
	}
	wchar_t* end = string_literal_end(start);
	std::wstring result(end - start, L'\0');
	int cur = 0;
	for (wchar_t* p = start; p < end;)
	{
		if (*p == L'\\' && (*p -1 != L'\\') /*&& L'n' != *(p+1)*/)
		{
			result.data()[cur++] = read_escaped_char(p+1);
			p += 2;
		}
		else
		{
			result.data()[cur++] = *p++;
		}
	}
	return result;
}


wchar_t* Tokenizer::string_literal_end(wchar_t* p)
{
	wchar_t* start = p;
	for (; *p != L'"'; p++)
	{
		if (*p == L'\n' || *p == L'\0')
		{
			throw tokenize_error(this->filename.c_str(), this->input.c_str(), start, L"unclosed string-literal");
		}
		if (*p == L'\\') //skip escaped char
		{
			p++;
		}
	}
	return p;
}


int Tokenizer::read_escaped_char(wchar_t* p)
{
	switch (*p)
	{
	case 'a': return L'\a';
	case 'b': return L'\b';
	case 'e': return  0x1b;
	case 't': return L'\t';
	//case 'n': return L'\n';
	case 'n': return L'\034';
	case 'v': return L'\v';
	case 'f': return L'\f';
	case 'r': return L'\r';
	default: return *p;
	}
}


std::wstring Tokenizer::read_bat_literal(wchar_t* start, wchar_t** result) {
	std::vector<wchar_t> buf;
	buf.reserve(1024);
	wchar_t* p = start + 1;
	for (; *p != L'`'; p++)
	{
		if (*p == L'\n' || *p == L'\0')
		{
			throw tokenize_error(this->filename.c_str(), input.c_str(), start, L"unclosed bat-literal");
		}
		if (*p == L'\\' && *(p + 1) == L'`')
		{
			buf.push_back(L'`');
			p++;
		}
		else
		{
			buf.push_back(*p);
		}
	}
	*result = p + 1;
	return std::wstring(buf.begin(), buf.end());
}


std::wstring Tokenizer::read_comment(wchar_t* start)
{
	wchar_t* p = start + 2; // "//"
	for (; *p != L'\n'; p++)
	{
		if (*p == L'\0')
		{
			throw tokenize_error(this->filename.c_str(), this->input.c_str(), start, L"unclosed literal");
		}
	}
	return std::wstring(start + 2, p);
}
