
#include "Logger.h"
#include "Tokenizer.h"
#include "Parser.h"

int main (int argc, char *argv[])
{
    // Initialize logger from environment variables
    Logger::InitializeFromEnvironment ();

    Logger::Log (Logger::Level::INFO, "Config INI Parser!!", argc);

    Tokenizer tokenizer;
    std::variant<std::vector<Token>, Error> tokenResult = tokenizer.Tokenize ("test_config.ini");

    // Check for tokenization error
    if (std::holds_alternative<Error> (tokenResult))
    {
        const Error& err = std::get<Error> (tokenResult);
        Logger::Log (Logger::Level::ERROR, "Tokenization error at line {}, column {}: {}", 
                    err.lineNumber, err.columnNumber, err.message);
        return 1;
    }

    std::vector<Token> tokens = std::get<std::vector<Token>> (tokenResult);

    Parser parser;
    std::variant<Document, Error> parseResult = parser.Parse (tokens);

    // Check for parsing error
    if (std::holds_alternative<Error> (parseResult))
    {
        const Error& err = std::get<Error> (parseResult);
        Logger::Log (Logger::Level::ERROR, "Parsing error at line {}, column {}: {}", 
                    err.lineNumber, err.columnNumber, err.message);
        return 1;
    }

    Document doc = std::get<Document> (parseResult);

    // For demonstration, print the parsed document
    for (const auto &[sectionName, section]: doc)
    {
        Logger::Log (Logger::Level::INFO, "[{}]", sectionName);
        for (const auto &[key, value]: section)
        {
            std::visit ([&key] (const auto &arg) 
                { 
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, std::vector<std::string>>)
                    {
                        std::string arrayStr = "[";
                        for (size_t i = 0; i < arg.size(); ++i)
                        {
                            if (i > 0) arrayStr += ", ";
                            arrayStr += arg[i];
                        }
                        arrayStr += "]";
                        Logger::Log (Logger::Level::INFO, "{} = {}", key, arrayStr);
                    }
                    else
                    {
                        Logger::Log (Logger::Level::INFO, "{} = {}", key, arg);
                    }
                }, 
                value);
        }
    }

    return 0;
}
