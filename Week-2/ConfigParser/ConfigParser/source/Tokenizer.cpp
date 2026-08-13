
#include "Tokenizer.h"
#include "Logger.h"

#include <fstream>

std::variant<std::vector<Token>, Error> Tokenizer::Tokenize (const std::string &input)
{
    std::ifstream file(input);

    if (!file.is_open())
    {
        Logger::Log(Logger::Level::ERROR, "Failed to open file: {}", input);
        return Error { 0, 0, "Failed to open file: " + input };
    }

    Logger::Log(Logger::Level::INFO, "Tokenizing file: {}", input);

    std::vector<Token> tokens;
    std::string line;
    int lineNumber = 1;

    while (std::getline(file, line))
    {
        // Skip blank lines
        if (line.empty() || line.find_first_not_of(" \t") == std::string::npos)
        {
            tokens.push_back({ TokenType::BlankLine, "", lineNumber, 0 });
            ++lineNumber;
            continue;
        }

        // Skip comments
        if (line[line.find_first_not_of(" \t")] == ';')
        {
            tokens.push_back({ TokenType::Comment, line, lineNumber, 0 });
            ++lineNumber;
            continue;
        }

        // Parse section headers [SectionName]
        size_t firstNonSpace = line.find_first_not_of(" \t");
        if (firstNonSpace != std::string::npos && line[firstNonSpace] == '[')
        {
            size_t closePos = line.find(']', firstNonSpace);
            if (closePos != std::string::npos)
            {
                std::string sectionName = line.substr(firstNonSpace + 1, closePos - firstNonSpace - 1);
                tokens.push_back({ TokenType::SectionHeader, sectionName, lineNumber, static_cast<int>(firstNonSpace) });
                ++lineNumber;
                continue;
            }
            else
            {
                file.close();
                Logger::Log(Logger::Level::ERROR, "Mismatched brackets in section header at line {}", lineNumber);
                return Error { lineNumber, static_cast<int>(firstNonSpace), "Mismatched brackets in section header" };
            }
        }

        // Parse key=value pairs
        size_t equalsPos = line.find('=');
        if (equalsPos != std::string::npos)
        {
            std::string key = line.substr(0, equalsPos);
            std::string value = line.substr(equalsPos + 1);

            // Trim trailing spaces from key
            size_t keyEnd = key.find_last_not_of(" \t");
            if (keyEnd != std::string::npos)
            {
                key = key.substr(0, keyEnd + 1);
            }

            // Trim trailing spaces from value
            size_t valueEnd = value.find_last_not_of(" \t");
            if (valueEnd != std::string::npos)
            {
                value = value.substr(0, valueEnd + 1);
            }

            // Trim leading spaces from value and calculate actual column
            size_t valueStart = value.find_first_not_of(" \t");
            int valueColumn = static_cast<int>(equalsPos + 1);
            if (valueStart != std::string::npos)
            {
                value = value.substr(valueStart);
                valueColumn += static_cast<int>(valueStart);
            }

            if (!key.empty())
            {
                tokens.push_back({ TokenType::Key, key, lineNumber, 0 });
                tokens.push_back({ TokenType::Equals, "=", lineNumber, static_cast<int>(equalsPos) });
                tokens.push_back({ TokenType::Value, value, lineNumber, valueColumn });
            }
            else
            {
                file.close();
                Logger::Log(Logger::Level::ERROR, "Empty key in key=value pair at line {}", lineNumber);
                return Error { lineNumber, 0, "Empty key in key=value pair" };
            }
        }
        else
        {
            file.close();
            Logger::Log(Logger::Level::ERROR, "Invalid line format at line {}: expected key=value pair", lineNumber);
            return Error { lineNumber, 0, "Invalid line format: expected key=value pair" };
        }

        ++lineNumber;
    }

    file.close();
    tokenList = tokens;
    Logger::Log(Logger::Level::INFO, "Tokenization completed successfully. Total tokens: {}", tokens.size());
    return tokens;
}
