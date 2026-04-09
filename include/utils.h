#pragma once
#include <cstring>
#include <limits>
#include <random>

namespace utils {

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

    template <typename T1, typename T2>
    T1 convert(T2 from) {
        static_assert(sizeof(T1) == sizeof(T2), "Types must have the same size");
        T1 result;
        std::memcpy(&result, &from, sizeof(T1));
        return result;
    }

    inline bool fix_fail() {
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return true;
        }

        return false;
    }

}
