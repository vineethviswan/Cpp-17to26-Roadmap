
#include "Scanner.h"
#include "Types.h"

#include <filesystem>
#include <system_error>
#include <functional>
#include <cctype>

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

        directory_options options = directory_options::skip_permission_denied;

        for (recursive_directory_iterator it(root, options, ec), end; it != end; it.increment(ec))
        {
            if (ec)
            {
                // Skip entries that cause errors (permissions, symlink loops, etc.).
                ec.clear();
                continue;
            }

            const directory_entry &entry = *it;

            // Only consider regular files
            std::error_code st;
            if (!entry.is_regular_file(st))
            {
                continue;
            }

            // Respect maxDepth if configured
            if (maxDepth)
            {
                std::error_code relEc;
                path rel = entry.path().lexically_relative(root);
                if (!rel.empty())
                {
                    int depth = 0;
                    for (auto &part : rel)
                        ++depth;
                    if (depth > *maxDepth)
                        continue;
                }
            }

            FileRecord rec;
            rec.path = entry.path();

            // extension (normalize to lowercase, include leading '.')
            try
            {
                rec.extension = entry.path().extension().string();
                for (auto &c : rec.extension)
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
            catch (...) { rec.extension.clear(); }

            try
            {
                rec.sizeBytes = entry.file_size();
            }
            catch (const std::filesystem::filesystem_error &)
            {
                // Could not determine size; skip this file
                continue;
            }

            try
            {
                rec.lastModified = entry.last_write_time();
            }
            catch (...) { /* ignore timestamp failures */ }

            // Currently PathFilter has no public API; filtering would be applied here when available.

            // Deliver record to caller
            onRecord(rec);
        }
    }
}
