#pragma once

#include "Types.h"

#include <filesystem>
#include <map>
#include <memory>
#include <ostream>

namespace finv
{
    class Report
    {
    public:
        virtual ~Report () = default;
        virtual void Write (std::ostream &os, const std::filesystem::path &root, GroupBy groupBy,
                const std::map<SummaryKey, SummaryEntry> &groups, const Totals &totals) const = 0;
    };

    class TextReport : public Report
    {
    public:
        void Write (std::ostream &os, const std::filesystem::path &root, GroupBy groupBy,
                const std::map<SummaryKey, SummaryEntry> &groups, const Totals &totals) const override;
    };

    class JsonReport : public Report
    {
    public:
        void Write (std::ostream &os, const std::filesystem::path &root, GroupBy groupBy,
                const std::map<SummaryKey, SummaryEntry> &groups, const Totals &totals) const override;
    };

    std::unique_ptr<Report> CreateReport (ReportFormat format);
}
