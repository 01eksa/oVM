#pragma once
#include <string>
#include <fstream>
#include <cstdint>
#include <memory>
#include <format>
#include "program.h"
#include "config.h"

#pragma pack(push, 1)
struct OVMHeader {
    uint32_t format;
    uint16_t version_major, version_minor;
    uint64_t data_size, code_size;
    uint64_t stack_size, call_stack_size;
};
#pragma pack(pop)


inline Program load(const std::string& filename) {
    std::ifstream ifile(filename, std::ios::binary);
    if (!ifile.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    OVMHeader header{};
    ifile.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.format != config::MAGIC) {
        throw std::runtime_error(std::format("Wrong format: {:x}", header.format));
    }

    if (header.version_major != config::MAJOR || header.version_minor > config::MINOR) {
        throw std::runtime_error("The binary version is newer than the virtual machine version. Please install the update.");
    }

    Program p;
    p.data_size = header.data_size;
    p.code_size = header.code_size;
    p.stack_size = header.stack_size;
    p.call_stack_size = header.call_stack_size;
    p.data = std::make_unique<uint8_t[]>(p.data_size);
    p.code = std::make_unique<uint8_t[]>(p.code_size);
    ifile.read(reinterpret_cast<char*>(p.data.get()), static_cast<std::streamsize>(p.data_size));
    ifile.read(reinterpret_cast<char*>(p.code.get()), static_cast<std::streamsize>(p.code_size));

    return p;
}
