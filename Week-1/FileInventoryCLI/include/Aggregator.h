#pragma once

#include "Types.h"

#include <map>

namespace finv
{
    class Aggregator
    {
    public:
        explicit Aggregator (GroupBy groupBy) : mode (groupBy) { }
        void Add (const FileRecord &record);
        [[nodiscard]] const std::map<SummaryKey, SummaryEntry> &GetGroups () const { return groups; }
        [[nodiscard]] const Totals &GetTotals () const { return totals; }

    private:
        GroupBy mode;
        std::map<SummaryKey, SummaryEntry> groups;
        Totals totals {};

        SummaryKey ResolveKey (const FileRecord &record) const;
    };
}
