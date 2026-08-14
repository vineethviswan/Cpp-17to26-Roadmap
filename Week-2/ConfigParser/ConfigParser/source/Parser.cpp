
#include "Parser.h"
#include "Logger.h"
#include <algorithm>
#include <cctype>

namespace
{
    bool TryParseBoolean(const std::string& value, bool& result)
    {
        std::string lower = value;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                      [](unsigned char c) { return std::tolower(c); });

        if (lower == "true")
        {
            result = true;
            return true;
        }
        else if (lower == "false")
        {
            result = false;
            return true;
        }
        return false;
    }

    bool TryParseInteger(const std::string& value, int& result)
    {
        try
        {
            result = std::stoi(value);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    bool IsArray(const std::string& value)
    {
        std::string trimmed = value;
        // Trim leading spaces
        size_t start = trimmed.find_first_not_of(" \t");
        if (start != std::string::npos)
        {
            trimmed = trimmed.substr(start);
        }
        // Trim trailing spaces
        size_t end = trimmed.find_last_not_of(" \t");
        if (end != std::string::npos)
        {
            trimmed = trimmed.substr(0, end + 1);
        }
        return !trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']';
    }

    std::vector<std::string> ParseArray(const std::string& value)
    {
        std::vector<std::string> result;

        // Remove the [ and ]
        std::string content = value;
        size_t start = content.find('[');
        size_t end = content.rfind(']');

        if (start != std::string::npos && end != std::string::npos && start < end)
        {
            content = content.substr(start + 1, end - start - 1);
        }

        // Split by comma and trim whitespace
        size_t pos = 0;
        while (pos < content.length())
        {
            size_t commaPos = content.find(',', pos);
            std::string element;

            if (commaPos == std::string::npos)
            {
                element = content.substr(pos);
                pos = content.length();
            }
            else
            {
                element = content.substr(pos, commaPos - pos);
                pos = commaPos + 1;
            }

            // Trim leading spaces
            size_t elemStart = element.find_first_not_of(" \t");
            if (elemStart != std::string::npos)
            {
                element = element.substr(elemStart);
            }

            // Trim trailing spaces
            size_t elemEnd = element.find_last_not_of(" \t");
            if (elemEnd != std::string::npos)
            {
                element = element.substr(0, elemEnd + 1);
            }
            else if (!element.empty())
            {
                element.clear();
            }

            if (!element.empty())
            {
                result.push_back(element);
            }
        }

        return result;
    }

    Value ConvertValue(const std::string& valueStr, int lineNumber)
    {
        if (IsArray(valueStr))
        {
            return ParseArray(valueStr);
        }

        bool boolVal;
        if (TryParseBoolean(valueStr, boolVal))
        {
            return boolVal;
        }

        int intVal;
        if (TryParseInteger(valueStr, intVal))
        {
            return intVal;
        }

        return valueStr;
    }
}

std::variant<Document, Error> Parser::Parse (const std::vector<Token>& tokens)
{
    Logger::Log(Logger::Level::INFO, "Starting to parse tokens. Total tokens: {}", tokens.size());

    Document doc;
    std::string currentSection;

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const Token& token = tokens[i];

        // Skip blank lines and comments
        if (token.type == TokenType::BlankLine || token.type == TokenType::Comment)
        {
            continue;
        }

        // Handle section headers
        if (token.type == TokenType::SectionHeader)
        {
            currentSection = token.value;
            Logger::Log(Logger::Level::DEBUG, "Found section: [{}] at line {}", currentSection, token.lineNumber);

            if (doc.find(currentSection) == doc.end())
            {
                doc[currentSection] = Section();
            }
            continue;
        }

        // Handle key-value pairs
        if (token.type == TokenType::Key)
        {
            // Ensure we have a current section
            if (currentSection.empty())
            {
                Logger::Log(Logger::Level::ERROR, "Key '{}' found at line {} without a section header", 
                           token.value, token.lineNumber);
                return Error { token.lineNumber, token.columnNumber, 
                             "Key found without section header" };
            }

            // Next tokens should be Equals and Value
            if (i + 2 >= tokens.size())
            {
                Logger::Log(Logger::Level::ERROR, "Incomplete key-value pair at line {}", token.lineNumber);
                return Error { token.lineNumber, token.columnNumber, 
                             "Incomplete key-value pair" };
            }

            if (tokens[i + 1].type != TokenType::Equals)
            {
                Logger::Log(Logger::Level::ERROR, "Expected '=' after key at line {}", token.lineNumber);
                return Error { tokens[i + 1].lineNumber, tokens[i + 1].columnNumber, 
                             "Expected '=' after key" };
            }

            if (tokens[i + 2].type != TokenType::Value)
            {
                Logger::Log(Logger::Level::ERROR, "Expected value after '=' at line {}", token.lineNumber);
                return Error { tokens[i + 2].lineNumber, tokens[i + 2].columnNumber, 
                             "Expected value after '='" };
            }

            const std::string& key = token.value;
            const std::string& valueStr = tokens[i + 2].value;

            Logger::Log(Logger::Level::DEBUG, "Parsing key '{}' with value '{}' in section [{}]", 
                       key, valueStr, currentSection);

            Value value = ConvertValue(valueStr, token.lineNumber);
            doc[currentSection][key] = value;

            // Skip the Equals and Value tokens
            i += 2;
            continue;
        }
    }

    Logger::Log(Logger::Level::INFO, "Parsing completed successfully. Total sections: {}", doc.size());
    return doc;
}
