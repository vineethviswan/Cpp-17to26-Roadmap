#include "Filter.h"
#include "Types.h"

#include <filesystem>
#include <string_view>
#include <algorithm>
#include <string>
#include <cctype>
#include <vector>

namespace finv
{
    bool PathFilter::Matches(const std::filesystem::path& path) const
    {
        // Convert path to string for pattern matching
        std::string pathStr = path.string();
        
        // Make lowercase for case-insensitive matching
        std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(),
                      [](unsigned char c) { return std::tolower(c); });

        // If there are include patterns, the path must match at least one
        if (!config_.includePatterns.empty())
        {
            if (!MatchesAny(pathStr, config_.includePatterns))
                return false;
        }

        // If there are exclude patterns, the path must NOT match any
        if (!config_.excludePatterns.empty())
        {
            if (MatchesAny(pathStr, config_.excludePatterns))
                return false;
        }

        return true;
    }

    bool PathFilter::MatchesAny(std::string_view path, 
                               const std::vector<std::string>& patterns) noexcept
    {
        return std::any_of(patterns.begin(), patterns.end(),
            [path](const std::string& pattern) {
                // Convert pattern to lowercase for case-insensitive comparison
                std::string lowerPattern = pattern;
                std::transform(lowerPattern.begin(), lowerPattern.end(), 
                             lowerPattern.begin(),
                             [](unsigned char c) { return std::tolower(c); });
                return GlobMatch(path, lowerPattern);
            });
    }

    bool PathFilter::GlobMatch(std::string_view text, std::string_view pattern) noexcept
    {
        std::size_t textIdx = 0;
        std::size_t patIdx = 0;
        std::size_t starIdx = std::string_view::npos;
        std::size_t matchIdx = 0;

        while (textIdx < text.length())
        {
            if (patIdx < pattern.length())
            {
                if (pattern[patIdx] == '?')
                {
                    // ? matches any single character
                    ++textIdx;
                    ++patIdx;
                }
                else if (pattern[patIdx] == '*')
                {
                    // * matches any sequence of characters
                    starIdx = patIdx;
                    matchIdx = textIdx;
                    ++patIdx;
                }
                else if (pattern[patIdx] == text[textIdx])
                {
                    // Characters match exactly
                    ++textIdx;
                    ++patIdx;
                }
                else if (starIdx != std::string_view::npos)
                {
                    // Previous * didn't match enough, try matching more
                    patIdx = starIdx + 1;
                    matchIdx++;
                    textIdx = matchIdx;
                }
                else
                {
                    return false;
                }
            }
            else if (starIdx != std::string_view::npos)
            {
                // More text but pattern exhausted; backtrack to last *
                patIdx = starIdx + 1;
                matchIdx++;
                textIdx = matchIdx;
            }
            else
            {
                return false;
            }
        }

        // Check for remaining pattern characters (should only be *)
        while (patIdx < pattern.length() && pattern[patIdx] == '*')
        {
            ++patIdx;
        }

        return patIdx == pattern.length();
    }
}