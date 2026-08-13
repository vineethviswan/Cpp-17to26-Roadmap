
#include "Logger.h"
#include "Tokenizer.h"

int main (int argc, char *argv[])
{
    // Initialize logger from environment variables
    Logger::InitializeFromEnvironment ();

    Logger::Log (Logger::Level::INFO, "Config INI Parser!!", argc);

    Tokenizer tokenizer;
    std::variant<std::vector<Token>, Error> result = tokenizer.Tokenize ("test_config.ini");
    
    return 0;
}
