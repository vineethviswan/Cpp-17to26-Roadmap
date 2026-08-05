
#include "Aggregator.h"
#include "Types.h"

#include <utility>

namespace finv
{
    SummaryEntry MergeEntry (const SummaryEntry &current, const FileRecord &rec)
    {
        SummaryEntry next = current;
        next.fileCount++;
        next.totalSizeBytes += rec.sizeBytes;

        if (!next.minSizeBytes || rec.sizeBytes < *next.minSizeBytes)
        {
            next.minSizeBytes = rec.sizeBytes;
        }

        if (!next.maxSizeBytes || rec.sizeBytes > *next.maxSizeBytes)
        {
            next.maxSizeBytes = rec.sizeBytes;
        }

        return next;
    }

    Totals MergeTotals (const Totals &current, const FileRecord &rec)
    {
        Totals next = current;
        next.totalFileCount++;
        next.totalSizeBytes += rec.sizeBytes;
        return next;
    }

    SummaryKey ResolveKey (GroupBy mode, const FileRecord &rec)
    {
        switch (mode)
        {
            case GroupBy::EXTENSION:
                return rec.extension;
            case GroupBy::SIZEBUCKET:
                if (rec.sizeBytes < 1024)
                    return "<1KB";
                else if (rec.sizeBytes < 1024 * 1024)
                    return "1KB-1MB";
                else if (rec.sizeBytes < 1024 * 1024 * 1024)
                    return "1MB-1GB";
                else
                    return ">1GB";
            case GroupBy::FOLDER:
                return rec.path.parent_path ().string ();
            case GroupBy::NONE:
            default:
                return "ALL";
        }
    }

    AggregationResult AggregateOne (const AggregationResult &current, const FileRecord &rec, GroupBy mode)
    {
        AggregationResult next = current;
        auto key = ResolveKey (mode, rec);

        auto it = next.groups.find (key);
        const SummaryEntry baseline = (it != next.groups.end ()) ? it->second : SummaryEntry {};
        next.groups[key] = MergeEntry (baseline, rec);
        next.totals = MergeTotals (next.totals, rec);

        return next;
    }

    void Aggregator::Add (const FileRecord &record)
    {
        result = AggregateOne (result, record, mode);
    }
}
