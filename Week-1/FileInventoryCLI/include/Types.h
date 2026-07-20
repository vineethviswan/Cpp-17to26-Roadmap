#pragma once

#include <cstdint>
#include <filesystem>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace finv
{
    struct FileRecord
    {
        std::filesystem::path path;
        std::string extension; // normalized, lowercase, includes leading '.'
        std::uintmax_t sizeBytes = 0;
        std::filesystem::file_time_type lastModified {};
    };

    enum class GroupBy
    {
        NONE,
        EXTENSION,
        SIZEBUCKET,
        FOLDER
    };

    enum class ReportFormat
    {
        PLAINTEXT,
        JSON
    };

    struct FilterConfig
    {
        std::vector<std::string> includePatterns;
        std::vector<std::string> excludePatterns;
        std::optional<int> maxDepth;
    };

    // The grouping key is just a string once resolved (extension, size bucket, or folder name)
    // No need for a variant type here; resolution happens once per record based on GroupBy mode.
    using SummaryKey = std::string;

    struct SummaryEntry
    {
        std::size_t fileCount = 0;
        std::uintmax_t totalSizeBytes = 0;
        std::uintmax_t minSizeBytes = std::numeric_limits<std::uintmax_t>::max ();
        std::uintmax_t maxSizeBytes = 0;
    };

    struct Totals
    {
        std::size_t totalFileCount = 0;
        std::uintmax_t totalSizeBytes = 0;
    };

}
