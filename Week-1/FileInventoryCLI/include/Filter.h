#pragma once

#include "Types.h"

#include <utility>

namespace finv
{
    class PathFilter
    {
    public:
        explicit PathFilter (FilterConfig config) : config_ (std::move (config)) { }

    private:
        FilterConfig config_;
    };
}
