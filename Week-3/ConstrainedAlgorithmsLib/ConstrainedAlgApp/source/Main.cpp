
#include "Logger.h"
#include "Search.h"
#include "Filter.h"
#include "Format.h"

#include <vector>
#include <string>

int main ()
{
    // Initialize logger from environment variables
    Logger::InitializeFromEnvironment ();
    Logger::Log (Logger::Level::INFO, "Constrained Algorithms Library!");

    // Example usage of Search    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto result = Search (vec, [](int x) { return x == 3; });
    Logger::Log (Logger::Level::INFO, "Searching for 3 in vector: {}", result.has_value () ? "Found" : "Not Found");

    // Example usage of Filter
    auto filtered = Filter (vec, [] (int x) { return x % 2 == 0; });
    Logger::Log (Logger::Level::INFO, "Filtered even numbers: {}", Format (filtered.value ()));

    // Example usage of Format
    std::string formatted = Format (vec, " | ");
    Logger::Log (Logger::Level::INFO, "Formatted vector: {}", formatted);

    return 0;
}
