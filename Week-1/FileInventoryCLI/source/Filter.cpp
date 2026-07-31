#include "Filter.h"
#include "Types.h"

#include <filesystem>
#include <string_view>
#include <algorithm>
#include <string>
#include <cctype>
#include <vector>

namespace
{
    std::string NormalizeForMatch(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::replace(value.begin(), value.end(), '\\', '/');
        return value;
    }

    [[nodiscard]] bool HasWildcard(std::string_view pattern) noexcept
    {
        return pattern.find_first_of("*?") != std::string_view::npos;
    }

    [[nodiscard]] bool EndsWith(std::string_view text, std::string_view suffix) noexcept
    {
        return suffix.size() <= text.size() &&
               text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
    }
}

namespace finv
{
    bool PathFilter::Matches(const std::filesystem::path& path) const
    {
        const std::string normalizedPath = NormalizeForMatch(path.generic_string());

        // Include defaults to "allow all" when no include patterns are provided.
        if (!config_.includePatterns.empty() && !MatchesAny(normalizedPath, config_.includePatterns))
        {
            return false;
        }

        // Exclude patterns always take precedence.
        if (!config_.excludePatterns.empty() && MatchesAny(normalizedPath, config_.excludePatterns))
        {
            return false;
        }

        return true;
    }

    bool PathFilter::MatchesAny(std::string_view path,
                               const std::vector<std::string>& patterns) noexcept
    {
        return std::any_of(patterns.begin(), patterns.end(),
            [path](const std::string& pattern)
            {
                if (pattern.empty())
                {
                    return false;
                }

                const std::string normalizedPattern = NormalizeForMatch(pattern);

                // Scope: simple wildcard (*, ?) + extension shorthand + substring matching.
                if (HasWildcard(normalizedPattern))
                {
                    return GlobMatch(path, normalizedPattern);
                }

                if (normalizedPattern.front() == '.')
                {
                    return EndsWith(path, normalizedPattern);
                }

                return path.find(normalizedPattern) != std::string_view::npos;
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
                    ++textIdx;
                    ++patIdx;
                }
                else if (pattern[patIdx] == '*')
                {
                    starIdx = patIdx;
                    matchIdx = textIdx;
                    ++patIdx;
                }
                else if (pattern[patIdx] == text[textIdx])
                {
                    ++textIdx;
                    ++patIdx;
                }
                else if (starIdx != std::string_view::npos)
                {
                    patIdx = starIdx + 1;
                    ++matchIdx;
                    textIdx = matchIdx;
                }
                else
                {
                    return false;
                }
            }
            else if (starIdx != std::string_view::npos)
            {
                patIdx = starIdx + 1;
                ++matchIdx;
                textIdx = matchIdx;
            }
            else
            {
                return false;
            }
        }

        while (patIdx < pattern.length() && pattern[patIdx] == '*')
        {
            ++patIdx;
        }

        return patIdx == pattern.length();
    }
}
