#pragma once

#include "Types.h"

#include <optional>
#include <stdexcept>
#include <filesystem>

namespace finv
{
    struct CLIOptions
    {
        std::filesystem::path rootDirectory;
        FilterConfig filterConfig;
        GroupBy groupBy = GroupBy::EXTENSION;
        ReportFormat reportFormat = ReportFormat::PLAINTEXT;
        std::optional<std::filesystem::path> outputFile;
    };

    struct CliParseError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    CLIOptions ParseArgs (int argc, char **argv);
}
