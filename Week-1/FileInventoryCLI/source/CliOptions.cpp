#include "CliOptions.h"
#include "Types.h"

#include <string>
#include <algorithm>
#include <cctype>

namespace finv
{
    namespace
    {
        static std::string ToLower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return s;
        }
    }

    CLIOptions ParseArgs (int argc, char** argv)
    {
        CLIOptions opts;

        bool rootSet = false;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg = argv[i];

            if (arg.rfind("--", 0) == 0)
            {
                // support --name=value and --name value
                auto eq = arg.find('=');
                std::string name;
                std::string value;
                if (eq != std::string::npos)
                {
                    name = arg.substr(2, eq - 2);
                    value = arg.substr(eq + 1);
                }
                else
                {
                    name = arg.substr(2);
                }

                name = ToLower(name);

                auto takeValue = [&]() -> std::string {
                    if (!value.empty())
                        return value;
                    if (i + 1 >= argc)
                        throw CliParseError("Option '" + name + "' requires a value");
                    return std::string(argv[++i]);
                };

                if (name == "include")
                {
                    opts.filterConfig.includePatterns.push_back(takeValue());
                }
                else if (name == "exclude")
                {
                    opts.filterConfig.excludePatterns.push_back(takeValue());
                }
                else if (name == "group-by" || name == "group")
                {
                    std::string v = ToLower(takeValue());
                    if (v == "extension" || v == "ext")
                        opts.groupBy = GroupBy::EXTENSION;
                    else if (v == "size" || v == "sizebucket")
                        opts.groupBy = GroupBy::SIZEBUCKET;
                    else if (v == "folder" || v == "dir")
                        opts.groupBy = GroupBy::FOLDER;
                    else if (v == "none")
                        opts.groupBy = GroupBy::NONE;
                    else
                        throw CliParseError("Unknown group-by value: " + v);
                }
                else if (name == "format" || name == "fmt")
                {
                    std::string v = ToLower(takeValue());
                    if (v == "text" || v == "plaintext")
                        opts.reportFormat = ReportFormat::PLAINTEXT;
                    else if (v == "json")
                        opts.reportFormat = ReportFormat::JSON;
                    else
                        throw CliParseError("Unknown format value: " + v);
                }
                else if (name == "output" || name == "o")
                {
                    opts.outputFile = std::filesystem::path(takeValue());
                }
                else if (name == "max-depth" || name == "maxdepth")
                {
                    std::string v = takeValue();
                    try
                    {
                        int n = std::stoi(v);
                        if (n < 0)
                            throw CliParseError("max-depth must be non-negative");
                        opts.filterConfig.maxDepth = n;
                    }
                    catch (const std::invalid_argument &)
                    {
                        throw CliParseError("Invalid max-depth value: " + v);
                    }
                    catch (const std::out_of_range &)
                    {
                        throw CliParseError("max-depth value out of range: " + v);
                    }
                }
                else
                {
                    throw CliParseError("Unknown option: --" + name);
                }
            }
            else
            {
                if (rootSet)
                    throw CliParseError("Multiple root directories provided");
                opts.rootDirectory = std::filesystem::path(arg);
                rootSet = true;
            }
        }

        if (!rootSet)
            throw CliParseError("Root directory not specified");

        return opts;
    }
}