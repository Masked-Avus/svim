#pragma once

#include <ctime>

namespace svim {
    using Milliseconds = int;

    inline Milliseconds get_current_time() {
        return std::clock() / (CLOCKS_PER_SEC / 1000);
    }
}
