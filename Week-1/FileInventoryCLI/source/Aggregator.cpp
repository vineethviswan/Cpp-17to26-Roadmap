
#include "Aggregator.h"
#include "Types.h"

namespace finv
{
    void Aggregator::Add (const FileRecord &record)
    {
        SummaryKey key = ResolveKey (record);
        auto &entry = groups[key];
        entry.fileCount++;
        entry.totalSizeBytes += record.sizeBytes;
        if (record.sizeBytes < entry.minSizeBytes)
            entry.minSizeBytes = record.sizeBytes;
        if (record.sizeBytes > entry.maxSizeBytes)
            entry.maxSizeBytes = record.sizeBytes;
        totals.totalFileCount++;
        totals.totalSizeBytes += record.sizeBytes;
    }
    SummaryKey Aggregator::ResolveKey (const FileRecord &record) const
    {
        switch (mode)
        {
            case GroupBy::EXTENSION:
                return record.extension;
            case GroupBy::SIZEBUCKET:
                if (record.sizeBytes < 1024)
                    return "<1KB";
                else if (record.sizeBytes < 1024 * 1024)
                    return "1KB-1MB";
                else if (record.sizeBytes < 1024 * 1024 * 1024)
                    return "1MB-1GB";
                else
                    return ">1GB";
            case GroupBy::FOLDER:
                return record.path.parent_path ().string ();
            case GroupBy::NONE:
            default:
                return "ALL";
        }
    }
}
