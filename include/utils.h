#pragma once
#include <cstring>

namespace utils {
    template <typename T1, typename T2>
    T1 convert(T2 from) {
        static_assert(sizeof(T1) == sizeof(T2), "Types must have the same size");
        T1 result;
        std::memcpy(&result, &from, sizeof(T1));
        return result;
    }
}
