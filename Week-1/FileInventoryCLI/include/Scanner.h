#pragma once

#include "Filter.h"
#include "Types.h"

#include <filesystem>
#include <functional>
#include <optional>
#include <utility>

namespace finv
{
    class Scanner
    {
    public:
        Scanner (std::filesystem::path root, PathFilter filter, std::optional<int> maxDepth = std::nullopt) :
            root (std::move (root)), filter (std::move (filter)), maxDepth (maxDepth)
        {
        }

        // Streaming form — avoids holding the whole tree in memory for large scans.
        // Caller passes a callback invoked per accepted FileRecord.
        void Scan (const std::function<void (const FileRecord &)> &onRecord) const;

    private:
        std::filesystem::path root;
        PathFilter filter;
        std::optional<int> maxDepth;
    };
}
