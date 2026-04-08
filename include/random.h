#pragma once

#include <cstdint>
#include <limits>
#include <random>

namespace Random {

    inline std::random_device                     rd;
    inline std::mt19937_64                        gen(rd());
    inline std::uniform_int_distribution<int64_t> dist(
        std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()
    );

    inline int64_t randint() {
        return dist(gen);
    }

    inline int64_t randint(const int64_t min, const int64_t max) {
        std::uniform_int_distribution<int64_t> _dist(min, max);
        return _dist(gen);
    }

}
