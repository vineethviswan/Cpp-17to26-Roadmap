#pragma once

#ifndef TYPES
#define TYPES

#include <string>
#include <variant>
#include <vector>
#include <map>

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

using Value = std::variant<int, std::string, bool, std::vector<std::string>>;
using Section = std::map<std::string, Value>;
using Document = std::map<std::string, Section>;

#endif // !TYPES
