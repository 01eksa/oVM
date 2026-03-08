#pragma once
#include <string>
#include <fstream>
#include <cstdint>
#include <memory>
#include <format>
#include "program.h"


inline Program load(const std::string& filename) {
    std::ifstream ifile(filename, std::ios::binary);
    if (!ifile.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    uint32_t format;
    ifile.read(reinterpret_cast<char*>(&format), sizeof(format));
    if (format != 0x3E4D564F) {
        throw std::runtime_error(std::format("Wrong format: {:x}", format));
    }

    uint16_t version_major, version_minor;
    ifile.read(reinterpret_cast<char*>(&version_major), sizeof(version_major));
    ifile.read(reinterpret_cast<char*>(&version_minor), sizeof(version_minor));

    uint64_t data_size, code_size;
    ifile.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
    ifile.read(reinterpret_cast<char*>(&code_size), sizeof(code_size));

    uint64_t stack_size, call_stack_size;
    ifile.read(reinterpret_cast<char*>(&stack_size), sizeof(stack_size));
    ifile.read(reinterpret_cast<char*>(&call_stack_size), sizeof(call_stack_size));

    Program p;
    p.data_size = data_size;
    p.code_size = code_size;
    p.stack_size = stack_size;
    p.call_stack_size = call_stack_size;
    p.data = std::make_unique<uint8_t[]>(p.data_size);
    p.code = std::make_unique<uint8_t[]>(p.code_size);
    ifile.read(reinterpret_cast<char*>(p.data.get()), data_size);
    ifile.read(reinterpret_cast<char*>(p.code.get()), code_size);

    return p;
}
