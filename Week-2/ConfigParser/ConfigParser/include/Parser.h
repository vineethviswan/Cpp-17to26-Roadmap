#pragma once

#include "Logger.h"
#include "Types.h"

class Parser
{
public:
    Parser () = default;
    ~Parser () = default;
    std::variant<Document, Error> Parse (const std::vector<Token> &tokens);

private:
    Document document;
};