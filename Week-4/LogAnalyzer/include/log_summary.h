#pragma once

#ifndef LOG_SUMMARY
#define LOG_SUMMARY

#include <algorithm>
#include <ranges>
#include <string>
#include <unordered_map>
#include <utility>

#include "log_types.h"

class LogSummary
{
public:
    // Build a LogSummary from a range of LogEntry objects via for_each/accumulation
    template<std::ranges::viewable_range Range>
    static LogSummary From (Range &&entries)
    {
        LogSummary summary;
        std::ranges::for_each (std::forward<Range> (entries),
                [&summary] (const LogEntry &entry)
                {
                    summary.counts[entry.level]++;
                    summary.pattern_counts[entry.source]++;
                });
        return summary;
    }

    // Count entries for a specific log level
    size_t CountFor (LogLevel level) const
    {
        auto it = counts.find (level);
        return (it != counts.end ()) ? it->second : 0;
    }

    // Get total count of all entries
    size_t Total () const
    {
        size_t sum = 0;
        for (const auto &[level, count]: counts)
        {
            sum += count;
        }
        return sum;
    }

private:
    std::unordered_map<std::string, size_t> pattern_counts;
    std::unordered_map<LogLevel, size_t> counts;
};

#endif // !LOG_SUMMARY
