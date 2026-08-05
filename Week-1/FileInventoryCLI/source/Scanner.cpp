
#include "Scanner.h"
#include "Types.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <system_error>

namespace
{
    char ToLowerAscii(char ch)
    {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }

    std::string NormalizedExtension(const std::filesystem::path& filePath)
    {
        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ToLowerAscii);
        return ext;
    }
}

namespace finv
{
    // Streaming form — avoids holding the whole tree in memory for large scans.
    // Caller passes a callback invoked per accepted FileRecord.
    void Scanner::Scan (const std::function<void (const FileRecord&)>& onRecord) const
    {
        using namespace std::filesystem;
        std::error_code ec;

        if (!exists(root, ec) || !is_directory(root, ec))
        {
            // Nothing to scan; caller is expected to validate root beforehand but be defensive here.
            return;
        }

        const directory_options options = directory_options::skip_permission_denied;
        for (recursive_directory_iterator it(root, options, ec), end; it != end; it.increment(ec))
        {
            if (ec)
            {
                // Skip entries that cause errors (permissions, symlink loops, etc.).
                ec.clear();
                continue;
            }

            const directory_entry& entry = *it;
            const path& entryPath = entry.path();

            // Only consider regular files
            std::error_code statEc;
            if (!entry.is_regular_file(statEc) || statEc)
            {
                continue;
            }

            // Respect maxDepth if configured
            if (maxDepth && static_cast<int>(it.depth()) > *maxDepth)
            {
                continue;
            }

            // Apply path filtering (include/exclude patterns)
            if (!filter.Matches(entryPath))
            {
                continue;
            }

            FileRecord rec;
            rec.path = entryPath;
            rec.extension = NormalizedExtension(entryPath);

            std::error_code sizeEc;
            rec.sizeBytes = entry.file_size(sizeEc);
            if (sizeEc)
            {
                // Could not determine size; skip this file
                continue;
            }

            std::error_code timeEc;
            rec.lastModified = entry.last_write_time(timeEc);
            if (timeEc)
            {
                rec.lastModified = {};
            }

            // Deliver record to caller
            onRecord(rec);
        }
    }
}
