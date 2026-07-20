
#include "Report.h"
#include "Types.h"

#include <iomanip>
#include <sstream>
#include <ostream>
#include <string>
#include <memory>
#include <cstdio>
#include <map>
#include <cstdint>

#include <limits>

namespace finv
{
	namespace
	{
		const char* GroupByToString(GroupBy g)
		{
			switch (g)
			{
				case GroupBy::NONE: return "none";
				case GroupBy::EXTENSION: return "extension";
				case GroupBy::SIZEBUCKET: return "size";
				case GroupBy::FOLDER: return "folder";
			}
			return "unknown";
		}

		// Simple JSON string escaper
		std::string JsonEscape(const std::string &s)
		{
			std::string out;
			out.reserve(s.size() + 8);
			for (unsigned char c : s)
			{
				switch (c)
				{
				case '\"': out += "\\\""; break;
				case '\\': out += "\\\\"; break;
				case '\b': out += "\\b"; break;
				case '\f': out += "\\f"; break;
				case '\n': out += "\\n"; break;
				case '\r': out += "\\r"; break;
				case '\t': out += "\\t"; break;
				default:
					if (c < 0x20)
					{
						char buf[7];
						std::snprintf(buf, sizeof(buf), "\\u%04x", c);
						out += buf;
					}
					else
					{
						out += static_cast<char>(c);
					}
				}
			}
			return out;
		}

		// Human readable bytes
		std::string HumanSize(uint64_t bytes)
		{
			constexpr uint64_t KB = 1024ULL;
			constexpr uint64_t MB = KB * 1024ULL;
			constexpr uint64_t GB = MB * 1024ULL;
			std::ostringstream ss;
			ss.setf(std::ios::fixed);
			ss.precision(2);
			if (bytes >= GB) ss << (static_cast<double>(bytes) / GB) << " GB";
			else if (bytes >= MB) ss << (static_cast<double>(bytes) / MB) << " MB";
			else if (bytes >= KB) ss << (static_cast<double>(bytes) / KB) << " KB";
			else ss << bytes << " B";
			return ss.str();
		}
	}

	void TextReport::Write (std::ostream &os, const std::filesystem::path &root, GroupBy groupBy,
			const std::map<SummaryKey, SummaryEntry> &groups, const Totals &totals) const
	{
		os << "Root: " << root.string() << "\n";
		os << "Group By: " << GroupByToString(groupBy) << "\n";
		os << "\n";

		// Header
		os << std::left << std::setw(32) << "Group" << std::right << std::setw(10) << "Files"
		   << std::setw(18) << "Total" << std::setw(12) << "Min" << std::setw(12) << "Max" << "\n";
		os << std::string(84, '-') << "\n";

		for (const auto &p : groups)
		{
			const auto &key = p.first;
			const auto &e = p.second;
			std::string keyOut = key;
			if (keyOut.size() > 30)
				keyOut = keyOut.substr(0, 27) + "...";

			os << std::left << std::setw(32) << keyOut
			   << std::right << std::setw(10) << e.fileCount
			   << std::setw(18) << HumanSize(e.totalSizeBytes)
			   << std::setw(12) << HumanSize(e.minSizeBytes == std::numeric_limits<std::uintmax_t>::max() ? 0 : e.minSizeBytes)
			   << std::setw(12) << HumanSize(e.maxSizeBytes) << "\n";
		}

		os << "\n";
		os << "Totals: files=" << totals.totalFileCount << ", bytes=" << totals.totalSizeBytes << "\n";
	}

	void JsonReport::Write (std::ostream &os, const std::filesystem::path &root, GroupBy groupBy,
			const std::map<SummaryKey, SummaryEntry> &groups, const Totals &totals) const
	{
		// Minimal hand-rolled JSON
		os << "{";
		os << "\"root\":\"" << JsonEscape(root.string()) << "\",";
		os << "\"groupBy\":\"" << JsonEscape(GroupByToString(groupBy)) << "\",";

		os << "\"groups\": [";
		bool first = true;
		for (const auto &p : groups)
		{
			if (!first) os << ",";
			first = false;
			const auto &k = p.first;
			const auto &e = p.second;
			os << "{";
			os << "\"key\":\"" << JsonEscape(k) << "\",";
			os << "\"fileCount\":" << e.fileCount << ",";
			os << "\"totalBytes\":" << e.totalSizeBytes << ",";
			os << "\"minBytes\":" << (e.minSizeBytes == std::numeric_limits<std::uintmax_t>::max() ? 0 : e.minSizeBytes) << ",";
			os << "\"maxBytes\":" << e.maxSizeBytes;
			os << "}";
		}
		os << "],";

		os << "\"totals\":{";
		os << "\"fileCount\":" << totals.totalFileCount << ",";
		os << "\"totalBytes\":" << totals.totalSizeBytes;
		os << "}";

		os << "}";
	}

	std::unique_ptr<Report> CreateReport (ReportFormat format)
	{
		switch (format)
		{
			case ReportFormat::PLAINTEXT: return std::make_unique<TextReport>();
			case ReportFormat::JSON: return std::make_unique<JsonReport>();
		}
		return std::make_unique<TextReport>();
	}

}
