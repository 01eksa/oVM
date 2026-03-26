#pragma once

#include <cstdint>
#include <random>
#include <limits>

namespace Random {
    inline std::random_device _rd;
    inline std::mt19937_64 _gen(_rd());
    inline std::uniform_int_distribution<int64_t> _dist(std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());


    inline int64_t randint() {
        return _dist(_gen);
    }

    inline int64_t randint(const int64_t min, const int64_t max) {
        std::uniform_int_distribution<int64_t> dist(min, max);
        return dist(_gen);
    }
}
