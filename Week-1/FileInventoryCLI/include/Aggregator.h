#pragma once

#include "Types.h"

#include <map>

namespace finv
{
    [[nodiscard]] SummaryEntry MergeEntry (const SummaryEntry &current, const FileRecord &rec);
    [[nodiscard]] Totals MergeTotals (const Totals &current, const FileRecord &rec);
    [[nodiscard]] SummaryKey ResolveKey (GroupBy mode, const FileRecord &rec);
    [[nodiscard]] AggregationResult AggregateOne (
            const AggregationResult &current, const FileRecord &rec, GroupBy mode);

    class Aggregator
    {
    public:
        explicit Aggregator (GroupBy groupBy) : mode (groupBy) { }
        void Add (const FileRecord &record);
        [[nodiscard]] const std::map<SummaryKey, SummaryEntry> &GetGroups () const { return result.groups; }
        [[nodiscard]] const Totals &GetTotals () const { return result.totals; }
        [[nodiscard]] const AggregationResult &GetResult () const { return result; }

    private:
        GroupBy mode;
        AggregationResult result {};
    };
}
