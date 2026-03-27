#pragma once
#include <cstdint>

enum class RegisterCode {
    FR   = 0x00,
    ARG1 = 0x01,
    ARG2 = 0x02,
    ARG3 = 0x03,
    ARG4 = 0x04,
    ARG5 = 0x05,
    ARG6 = 0x06,
    ARG7 = 0x07,
    ARG8 = 0x08,
    CR   = 0x10,
    REG1 = 0x11,
    REG2 = 0x12,
    REG3 = 0x13,
    REG4 = 0x14,
    REG5 = 0x15,
    REG6 = 0x16,
    REG7 = 0x17,
    REG8 = 0x18,
};

struct Registers {
    // VM registers
    uint64_t    CP = 0;         // command pointer
    bool        BF = false;     // bool flag
    bool        EF = false;     // error flag

    // User registers
    static constexpr int REG_COUNT = 32;
    union {
        int64_t registers[REG_COUNT]{0};
        struct {
            int64_t FR;
            int64_t ARG1;
            int64_t ARG2;
            int64_t ARG3;
            int64_t ARG4;
            int64_t ARG5;
            int64_t ARG6;
            int64_t ARG7;
            int64_t ARG8;

            int64_t _reserved[static_cast<int>(RegisterCode::CR) - static_cast<int>(RegisterCode::ARG8) - 1];

            int64_t CR;
            int64_t REG1;
            int64_t REG2;
            int64_t REG3;
            int64_t REG4;
            int64_t REG5;
            int64_t REG6;
            int64_t REG7;
            int64_t REG8;
        };
    };
};

static_assert(offsetof(Registers, CR) == offsetof(Registers, registers[static_cast<int>(RegisterCode::CR)]));
