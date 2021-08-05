#pragma once

#include <vector>
#include <string>
#include "token.h"

class Tokens
{
private:
	std::vector<Token*> tokens;
	int cur;
public:
	Tokens(const std::wstring& input, const std::wstring& filename);

	void push(Token* token);
	Token* front();
	void pop();
	size_t size();
	void reset();
	bool is_end();
	void dump();

	Token* consume(const std::wstring& val);
	Token* try_consume(const std::wstring& val);
	Token* try_consume(TokenKind kind);
	bool peek(const std::wstring& val);

private:
	Token* back;
	const std::wstring& input;
	const std::wstring filename;
};

