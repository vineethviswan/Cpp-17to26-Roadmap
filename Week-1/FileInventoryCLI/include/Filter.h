#pragma once

#include "Types.h"

#include <filesystem>
#include <string_view>
#include <utility>
#include <vector>
#include <string>

namespace finv
{
    class PathFilter
    {
    public:
        explicit PathFilter(FilterConfig config) : config_(std::move(config)) {}

        /// @brief Check if a file path should be accepted based on include/exclude patterns
        /// @param path The file path to check
        /// @return true if the path matches (or should be included), false otherwise
        [[nodiscard]] bool Matches(const std::filesystem::path& path) const;

        FilterConfig config_;
    private:        

        /// @brief Check if a path matches any pattern in a list
        /// @param path The path to check
        /// @param patterns List of glob/literal patterns
        /// @return true if path matches any pattern
        static bool MatchesAny(std::string_view path, 
                              const std::vector<std::string>& patterns) noexcept;

        /// @brief Simple glob pattern matcher supporting * and ?
        /// @param text The text to match against
        /// @param pattern The glob pattern (* for any chars, ? for single char)
        /// @return true if text matches the pattern
        static bool GlobMatch(std::string_view text, std::string_view pattern) noexcept;
    };
}