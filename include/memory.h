#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

class Memory {
    
public:
    [[nodiscard]] static void* alloc(const std::size_t size) {
        return malloc(size);
    }

    static void free(void* ptr) {
        ::free(ptr);
    }

    static void write(void* ptr, const int64_t val) {
        std::memcpy(ptr, &val, sizeof(int64_t));
    }

    [[nodiscard]] static int64_t read(const void* ptr) {
        int64_t val;
        std::memcpy(&val, ptr, sizeof(int64_t));
        return val;
    }
};
