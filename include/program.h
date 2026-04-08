#pragma once

#include <cstdint>
#include <memory>

struct Program {
    uint64_t stack_size;
    uint64_t call_stack_size;

    std::unique_ptr<uint8_t[]> data;
    uint64_t                   data_size;

    std::unique_ptr<uint8_t[]> code;
    uint64_t                   code_size;
};
