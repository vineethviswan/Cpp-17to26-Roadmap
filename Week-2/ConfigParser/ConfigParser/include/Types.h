#pragma once

#ifndef TYPES
#define TYPES

#include <string>

enum class TokenType
{
    SectionHeader, 
    Key,
    Equals,
    Value,
    Comment,
    BlankLine,
};

struct Token
{
    TokenType type;
    std::string value;
    int lineNumber;
    int columnNumber;
};

enum class ValueType
{
    Integer,
    String,
    Boolean,
    Array
};

struct Error
{
    int lineNumber;
    int columnNumber;
    std::string message;
};

#endif // !TYPES
