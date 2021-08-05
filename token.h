#pragma once
#include <string>

enum class TokenKind {
    IDENT,          // Identifires
    PUNCT,          // Punctuators
    NUM,            // Numeric
    BOOL,           // Boolean
    STR,            // String
    BAT,            // Batch literal
    COMMENT,        // Comment literal
    END_OF_TOKEN,   // End-Of-File markers
    KEYWORD         // return, if, for
};

/** token */
struct Token {
    TokenKind kind;
    Token* next;
    std::wstring value;
    const wchar_t* pos;
};