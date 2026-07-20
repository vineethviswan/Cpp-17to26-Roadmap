
#include "Logger.h"
#include "Aggregator.h"
#include "CliOptions.h"
#include "Filter.h"
#include "Report.h"
#include "Scanner.h"
#include "Types.h"

#include <filesystem>
#include <fstream>
#include <iostream>

using namespace finv;

int main (int argc, char *argv[])
{
    Logger::Log (Logger::Level::INFO, "File Inventory CLI");

    CLIOptions options;
    try
    {
        options = ParseArgs (argc, argv);
    }
    catch (const CliParseError &e)
    {
        Logger::Log (Logger::Level::ERROR, e.what ());
        return 1;
    }

    if (!std::filesystem::is_directory (options.rootDirectory))
    {
        Logger::Log (Logger::Level::ERROR, "Root directory does not exist: {}", options.rootDirectory.string ());
        return 1;
    }

    PathFilter filter (options.filterConfig);
    Scanner scanner (options.rootDirectory, filter);
    Aggregator aggregator (options.groupBy);

    scanner.Scan ([&aggregator] (const FileRecord &record) { aggregator.Add (record); });

    auto report = CreateReport (options.reportFormat);

    if (options.outputFile)
    {
        std::ofstream ofs (*options.outputFile);
        if (!ofs)
        {
            Logger::Log (Logger::Level::ERROR, "Failed to open output file: {}", options.outputFile->string ());
            return 1;
        }
        report->Write (ofs, options.rootDirectory, options.groupBy, aggregator.GetGroups (), aggregator.GetTotals ());
    }
    else
    {
        report->Write (
                std::cout, options.rootDirectory, options.groupBy, aggregator.GetGroups (), aggregator.GetTotals ());
    }

    return 0;
}
