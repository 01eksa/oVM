#pragma once
#include <cstdint>

struct Registers {
    // VM registers
    uint64_t    CP = 0;         // command pointer
    bool        BF = false;     // bool flag
    bool        EF = false;     // error flag
    uint64_t    CR = 0;         // counter register

    // VMCALL registers
    int64_t FR   = 0;
    int64_t ARG1 = 0;
    int64_t ARG2 = 0;
    int64_t ARG3 = 0;
    int64_t ARG4 = 0;

    // user registers
    int64_t REG1 = 0;
    int64_t REG2 = 0;
    int64_t REG3 = 0;
    int64_t REG4 = 0;
};

enum class RegisterCode {
    FR   = 0x00,
    ARG1 = 0x01,
    ARG2 = 0x02,
    ARG3 = 0x03,
    ARG4 = 0x04,
    REG1 = 0x10,
    REG2 = 0x11,
    REG3 = 0x12,
    REG4 = 0x13,
    CR   = 0x20,
};
