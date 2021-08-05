#pragma once

#include "token.h"
#include "tokens.h"
#include <string>

class Tokenizer {
public:
	Tokenizer(const std::wstring& input, const std::wstring& filename);
	Tokens tokenize();


	Token* new_token(TokenKind kind, const std::wstring& value, const wchar_t* pos);
	bool starts_with(wchar_t* str, const wchar_t* pre);
	bool is_first_ident(wchar_t c);
	bool is_ident(wchar_t c);
	bool is_bool(wchar_t* c);

	std::wstring fetch_ident(wchar_t* str);
	std::wstring fetch_keyword(wchar_t* str);
	std::wstring fetch_punct(wchar_t* str);
	std::wstring fetch_bool(wchar_t* str);
	std::wstring read_string_literal(wchar_t* str);
	wchar_t* string_literal_end(wchar_t* p);
	int read_escaped_char(wchar_t* p);
	std::wstring read_bat_literal(wchar_t* str, wchar_t** result);
	std::wstring read_comment(wchar_t* str);
private:
	const std::wstring& input;
	const std::wstring filename;
};

