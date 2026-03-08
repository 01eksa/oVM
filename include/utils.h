#pragma once
#include <cstring>
#include <limits>

namespace utils {
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
