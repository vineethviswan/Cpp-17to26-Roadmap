#pragma once
#ifndef REPORT_FORMATTER
#define REPORT_FORMATTER

#include <iomanip>
#include <ostream>
#include <ranges>
#include <string>

#include "log_summary.h"
#include "log_types.h"

/*
ReportFormatter (class — presentation only)
    + format(const LogSummary&) -> string
    + print(const LogSummary&, ostream&)
*/

class ReportFormatter
{
public:
    // Format a LogSummary as a formatted string
    static std::string Format (const LogSummary &summary)
    {
        std::string report;

        report += "=================================================\n";
        report += "             LOG ANALYSIS REPORT                 \n";
        report += "=================================================\n\n";

        // Total count
        report += "OVERALL STATISTICS:\n";
        report += "-------------------\n";
        report += std::string ("Total Entries: ") + std::to_string (summary.Total ()) + "\n\n";

        // Breakdown by log level
        report += "BREAKDOWN BY LOG LEVEL:\n";
        report += "----------------------\n";

        constexpr LogLevel levels[] = {LogLevel::DEBUG, LogLevel::INFO, LogLevel::WARNING, LogLevel::ERROR};

        for (const auto &level: levels)
        {
            const auto count = summary.CountFor (level);
            const auto levelStr = ToString (level);
            report += std::string ("  ") + std::string (levelStr) + ": " + std::to_string (count) + "\n";
        }

        report += "\n";

        report += "\n=================================================\n";

        return report;
    }

    // Print a LogSummary to an ostream
    static void print (const LogSummary &summary, std::ostream &os) { os << Format (summary); }
};

#endif // !REPORT_FORMATTER
