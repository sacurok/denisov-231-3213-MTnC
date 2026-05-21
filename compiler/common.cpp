#pragma once
#include <string>

using namespace std;

enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT_INT,
    CONSTANT_FLOAT,
    CONSTANT_STRING,
    CONSTANT_CHAR,
    OPERATOR,
    DELIMITER,
    PREPROCESSOR,
    TOK_EOF,
    UNKNOWN
};

struct Token {
    TokenType type;
    string value;
    int line;
};