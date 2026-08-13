#pragma once

#ifndef TOKENIZER
#define TOKENIZER

#include "Types.h"

#include <variant>
#include <vector>
#include <string>

class Tokenizer
{
public:
    Tokenizer () = default;
    ~Tokenizer () = default;

    std::variant<std::vector<Token>, Error> Tokenize (const std::string &input);

private:
    std::vector<Token> tokenList;
};

#endif // !TOKENIZER
