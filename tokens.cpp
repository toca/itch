#include "tokens.h"
#include "format.h"
#include "error.h"


Tokens::Tokens(const std::wstring& input, const std::wstring& filename)
	: tokens()
	, cur(0) 
	, back(nullptr)
	, input(input)
	, filename(filename)
{

}



void Tokens::push(Token* token)
{
	tokens.push_back(token);
	if (this->back)
	{
		this->back->next = token;
	}
	this->back = token;
}


Token* Tokens::front()
{
	if (this->tokens.size() - 1 <= cur)
	{
		throw parse_error(this->filename, this->input, tokens.back()->pos, L"tokens: unexpected end-of-token");
	}
	return tokens[cur];
}


void Tokens::pop()
{
	cur++;
}


size_t Tokens::size() 
{
	return tokens.size();
}


void Tokens::reset()
{
	cur = 0;
}

bool Tokens::is_end()
{
	return cur == tokens.size() - 1;
}

void Tokens::dump()
{
	for (auto token: this->tokens)
	{
		std::wstring kind = L"";
		switch (token->kind)
		{
		case TokenKind::IDENT:
			kind = L"IDENT";
			break;
		case TokenKind::PUNCT:
			kind = L"PUNCT";
			break;
		case TokenKind::NUM:
			kind = L"NUM";
			break;
		case TokenKind::BOOL:
			kind = L"BOOL";
			break;
		case TokenKind::STR:
			kind = L"STR";
			break;
		case TokenKind::KEYWORD:
			kind = L"KEYWORD";
			break;
		case TokenKind::END_OF_TOKEN:
			kind = L"END_OF_TOKEN";
			break;
		}
		wprintf(L"Kind: %s; value: %s\n", kind.c_str(), token->value.c_str());
	}
}




Token* Tokens::consume(const std::wstring& val)
{
	auto token = this->front();

	if (token->kind == TokenKind::COMMENT ||
		token->kind == TokenKind::IDENT ||
		token->kind == TokenKind::STR ||
		token->kind == TokenKind::BAT)
	{
		throw parse_error(this->filename, this->input, token->pos, format(L"'%s' expected. (kind)", val.c_str()));
	}
	if (token->value != val)
	{
		throw parse_error(this->filename, this->input, token->pos, format(L"'%s' expected.", val.c_str()));
	}
	else
	{
		this->pop();
		return token;
	}

}



Token* Tokens::try_consume(const std::wstring& val)
{
	auto token = this->front();
	if (token->value == val &&
		token->kind != TokenKind::COMMENT &&
		token->kind != TokenKind::IDENT &&
		token->kind != TokenKind::STR &&
		token->kind != TokenKind::BAT)
	{
		this->pop();
		return token;
	}
	else
	{
		return nullptr;
	}
}




Token* Tokens::try_consume(TokenKind kind)
{
	auto token = this->front();
	if (token->kind == kind)
	{
		this->pop();
		return token;
	}
	else
	{
		return nullptr;
	}
}



bool Tokens::peek(const std::wstring& val)
{
	auto token = this->front();
	if (token->kind == TokenKind::COMMENT ||
		token->kind == TokenKind::IDENT ||
		token->kind == TokenKind::STR ||
		token->kind == TokenKind::BAT)
	{
		return false;
	}
	return token->value == val;
}