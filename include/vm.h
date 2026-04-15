#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <fstream>
#include <iostream>

#include "callstack.h"
#include "memory.h"
#include "program.h"
#include "registers.h"
#include "stack.h"
#include "utils.h"

class VM {
    using Handler = void (VM::*)();
    using Segment = std::unique_ptr<uint8_t[]>;

    static constexpr auto ops     = 128;
    static constexpr auto vmcalls = 64;

    Handler op_dispatch[ops]         = {nullptr};
    Handler vmcall_dispatch[vmcalls] = {nullptr};

    Segment     code;
    std::size_t code_size;

    Segment     data;
    std::size_t data_size;

    Registers registers;
    Stack     stack;
    CallStack call_stack;

    bool debug_enabled = true;
    bool running       = false;

  public:
    explicit VM(Program p, const bool debug_enabled = true)
        : code(std::move(p.code)),
          code_size(p.code_size),
          data(std::move(p.data)),
          data_size(p.data_size),
          stack(p.stack_size ? p.stack_size : Stack::DEFAULT_CAPACITY),
          call_stack(p.call_stack_size ? p.call_stack_size : CallStack::DEFAULT_CAPACITY),
          debug_enabled(debug_enabled) {
        std::ranges::fill(op_dispatch, &VM::op_illegal);
        std::ranges::fill(vmcall_dispatch, &VM::vmcall_illegal);

        init_dispatch();
        init_vmcall_dispatch();
    }
    ~VM() = default;

    void run() {
        running = true;

        while (running) {
            const auto op = eat<uint8_t>();
            if (op >= ops) [[unlikely]] {
                op_illegal();
            }

            const auto handler = op_dispatch[op];
            (this->*handler)();
        }
    }

  private:
    void init_dispatch();
    void init_vmcall_dispatch();

    [[nodiscard]] std::string format_VM_registers() const;
    [[nodiscard]] std::string error_message(std::string_view message) const;
    [[nodiscard]] std::string format_user_registers() const;
    [[nodiscard]] std::string format_stack() const;

    template <typename T>
    T eat() {
        if (registers.CP + sizeof(T) > code_size) {
            throw std::runtime_error("Unexpected end of code segment!");
        }
        T result;
        std::memcpy(&result, &code[registers.CP], sizeof(T));
        registers.CP += sizeof(T);

        return result;
    }

    template <typename T>
    void write_reg(const T reg, const int64_t value) {
        if (reg < 0 || reg >= Registers::REG_COUNT)
            registers.EF = true;
        else
            registers.registers[reg] = value;
    }

    template <typename T>
    int64_t read_reg(T reg) {
        int64_t value = 0;

        if (reg < 0 || reg >= Registers::REG_COUNT)
            registers.EF = true;
        else
            value = registers.registers[reg];

        return value;
    }

    // Ops
    void op_illegal() {
        throw std::runtime_error(std::format("Illegal command: 0x{:02x}", code[registers.CP - 1]));
    }

    // program execution management
    void op_exit() {
        running = false;
    }

    void op_vmcall() {
        if (registers.FR >= vmcalls || registers.FR < 0) [[unlikely]]
            vmcall_illegal();

        const auto handler = vmcall_dispatch[registers.FR];
        (this->*handler)();
    }

    void op_call() {
        const auto addr = eat<uint64_t>();
        call_stack.push(registers.CP);
        registers.CP = addr;
    }

    void op_ret() {
        registers.CP = call_stack.pop();
    }

    void op_jmp() {
        const auto addr = eat<uint64_t>();
        registers.CP    = addr;
    }

    void op_jif() {
        const auto addr = eat<uint64_t>();
        if (registers.BF) registers.CP = addr;
    }

    void op_jnif() {
        const auto addr = eat<uint64_t>();
        if (!registers.BF) registers.CP = addr;
    }

    void op_jef() {
        const auto addr = eat<uint64_t>();
        if (registers.EF) registers.CP = addr;
    }

    void op_jnef() {
        const auto addr = eat<uint64_t>();
        if (!registers.EF) registers.CP = addr;
    }

    void op_loop() {
        const auto addr = eat<uint64_t>();
        if (--registers.CR > 0) registers.CP = addr;
    }

    void op_debug();

    // stack
    void op_push() {
        stack.push(eat<int64_t>());
    }

    void op_pop() {
        stack.pop();
    }

    void op_dup() {
        stack.dup();
    }

    void op_pea() {
        const auto offset = eat<uint64_t>();
        if (offset < data_size) {
            const auto ptr = static_cast<int64_t>(reinterpret_cast<uintptr_t>(&data[offset]));
            stack.push(ptr);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    void op_load() {
        const auto offset = eat<uint64_t>();
        if (offset + sizeof(int64_t) <= data_size) {
            const auto ptr  = reinterpret_cast<void*>(&data[offset]);
            const auto bits = Memory::read(ptr);

            stack.push(bits);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    void op_store() {
        const auto offset = eat<uint64_t>();
        if (offset + sizeof(int64_t) <= data_size) {
            const auto ptr  = reinterpret_cast<void*>(&data[offset]);
            const auto bits = stack.pop();
            Memory::write(ptr, bits);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    // call stack
    void op_push_cp() {
        call_stack.push(registers.CP);
    }

    void op_pop_cp() {
        registers.CP = call_stack.pop();
    }

    // registers
    void op_set_reg() {
        const auto reg   = eat<uint8_t>();
        const auto value = eat<int64_t>();

        write_reg(reg, value);
    }

    void op_push_reg() {
        const auto reg = eat<uint8_t>();
        const auto val = read_reg(reg);

        stack.push(val);
    }

    void op_pop_reg() {
        const auto reg   = eat<uint8_t>();
        const auto value = stack.pop();

        write_reg(reg, value);
    }

    void op_inc_reg() {
        const auto reg = eat<uint8_t>();
        const auto val = read_reg(reg) + 1;

        write_reg(reg, val);
    }

    void op_dec_reg() {
        const auto reg = eat<uint8_t>();
        const auto val = read_reg(reg) - 1;

        write_reg(reg, val);
    }

    void op_check_ef() {
        registers.BF = registers.EF;
    }

    void op_clear_ef() {
        registers.EF = false;
    }

    void op_mov() {
        const auto target = eat<uint8_t>();
        const auto source = eat<uint8_t>();

        write_reg(target, read_reg(source));
    }

    void op_lea() {
        const auto target = eat<uint8_t>();
        const auto offset = eat<uint64_t>();
        if (offset < data_size) {
            const auto ptr = static_cast<int64_t>(reinterpret_cast<uintptr_t>(&data[offset]));
            write_reg(target, ptr);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    void op_load_reg() {
        const auto target = eat<uint8_t>();
        const auto offset = eat<uint64_t>();
        if (offset + sizeof(int64_t) <= data_size) {
            const auto ptr  = reinterpret_cast<void*>(&data[offset]);
            const auto bits = Memory::read(ptr);

            write_reg(target, bits);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    void op_store_reg() {
        const auto offset = eat<uint64_t>();
        const auto source = eat<uint8_t>();
        if (offset + sizeof(int64_t) <= data_size) {
            const auto target = reinterpret_cast<void*>(&data[offset]);
            const auto bits   = read_reg(source);
            Memory::write(target, bits);
        } else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    // memory
    void op_alloc() {
        const auto size = stack.pop();
        const auto ptr  = Memory::alloc(size);

        stack.push_ptr(ptr);
    }

    void op_free() {
        const auto ptr = stack.pop_ptr();

        Memory::free(ptr);
    }

    void op_write() {
        const auto bits = stack.pop();
        const auto ptr  = stack.pop_ptr();

        Memory::write(ptr, bits);
    }

    void op_read() {
        const auto ptr  = stack.pop_ptr();
        const auto bits = Memory::read(ptr);

        stack.push(bits);
    }

    void op_write_byte() {
        const auto bits = stack.pop();
        const auto ptr  = stack.pop_ptr();
        const auto byte = static_cast<uint8_t>(bits);

        Memory::write<uint8_t>(ptr, byte);
    }

    void op_read_byte() {
        const auto ptr  = stack.pop_ptr();
        const auto bits = Memory::read<uint8_t>(ptr);

        stack.push(bits);
    }

    // math
    void op_inc() {
        stack.push(stack.pop() + 1);
    }

    void op_dec() {
        stack.push(stack.pop() - 1);
    }

    void op_add() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left + right);
    }

    void op_sub() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left - right);
    }

    void op_mul() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left * right);
    }

    void op_div() {
        const auto right = stack.pop();
        const auto left  = stack.pop();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push(left / right);
    }

    void op_mod() {
        const auto right = stack.pop();
        const auto left  = stack.pop();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push(left % right);
    }

    void op_pow() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(static_cast<int64_t>(std::pow(left, right)));
    }

    void op_inc_float() {
        stack.push_as<double>(stack.pop_as<double>() + 1);
    }

    void op_dec_float() {
        stack.push_as<double>(stack.pop_as<double>() - 1);
    }

    void op_add_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        stack.push_as<double>(left + right);
    }

    void op_sub_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        stack.push_as<double>(left - right);
    }

    void op_mul_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        stack.push_as<double>(left * right);
    }

    void op_div_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push_as<double>(left / right);
    }

    void op_mod_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push_as<double>(std::fmod(left, right));
    }

    void op_pow_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        stack.push_as<double>(std::pow(left, right));
    }

    // bit operations
    void op_and() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left & right);
    }

    void op_or() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left | right);
    }

    void op_xor() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left ^ right);
    }

    void op_inv() {
        stack.push(~stack.pop());
    }

    void op_shl() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left << right);
    }

    void op_shr() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        stack.push(left >> right);
    }

    // logic
    void op_not() {
        registers.BF = !registers.BF;
    }

    void op_cmp() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        registers.BF = left == right;
    }

    void op_cmplt() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        registers.BF = left < right;
    }

    void op_cmpgt() {
        const auto right = stack.pop();
        const auto left  = stack.pop();

        registers.BF = left > right;
    }

    void op_cmp_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        registers.BF = left == right;
    }

    void op_cmplt_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        registers.BF = left < right;
    }

    void op_cmpgt_float() {
        const auto right = stack.pop_as<double>();
        const auto left  = stack.pop_as<double>();

        registers.BF = left > right;
    }

    // basic functions
    void op_print_endl() {
        std::cout << std::endl;
    }

    void op_print_int() {
        std::cout << stack.pop();
    }

    void op_scan_int() {
        int64_t val;
        std::cin >> val;

        if (utils::fix_fail()) {
            registers.EF = true;
            val          = 0;
        } else
            std::cin.ignore();

        stack.push(val);
    }

    void op_print_float() {
        std::cout << stack.pop_as<double>();
    }

    void op_scan_float() {
        double val;
        std::cin >> val;

        if (utils::fix_fail()) {
            registers.EF = true;
            val          = 0;
        } else
            std::cin.ignore();

        stack.push_as<double>(val);
    }

    void op_int_to_float() {
        const auto int_val   = stack.pop();
        const auto float_val = static_cast<double>(int_val);

        stack.push_as<double>(float_val);
    }

    void op_float_to_int() {
        const auto float_val = stack.pop_as<double>();
        const auto int_val   = static_cast<int64_t>(float_val);

        stack.push(int_val);
    }

    // VM calls
    void vmcall_illegal() {
        throw std::runtime_error(std::format("Illegal vmcall: 0x{:x}", registers.FR));
    }

    // Memory & IO
    void vmcall_memcpy() {
        const auto dest = reinterpret_cast<void*>(static_cast<uintptr_t>(registers.ARG1));
        const auto src  = reinterpret_cast<void*>(static_cast<uintptr_t>(registers.ARG2));
        const auto n    = registers.ARG3;

        std::memcpy(dest, src, n);
    }

    void vmcall_printstr() {
        const auto buffer = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        if (const auto size = registers.ARG2)
            std::cout.write(buffer, size);
        else
            std::cout << buffer;
    }

    void vmcall_scanstr() {
        const auto buffer = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        const auto size   = registers.ARG2;

        std::cin.getline(buffer, size);

        if (utils::fix_fail()) registers.EF = true;
    }

    // files
    void vmcall_readfile() {
        const auto file_name_ptr = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        const auto buffer_ptr    = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG2));
        const auto bytes_to_read = registers.ARG3;

        std::ifstream file(file_name_ptr, std::ios::binary);
        if (!file) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        file.read(buffer_ptr, bytes_to_read);
        stack.push(file.gcount());

        if (file.fail() && !file.eof()) {
            registers.EF = true;
        }
    }

    void vmcall_writefile() {
        const auto file_name_ptr  = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        const auto buffer_ptr     = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG2));
        const auto bytes_to_write = registers.ARG3;

        std::ofstream file(file_name_ptr, std::ios::binary);
        if (!file) {
            registers.EF = true;
            return;
        }

        file.write(buffer_ptr, bytes_to_write);

        if (file.fail()) {
            registers.EF = true;
        }
    }

    void vmcall_appendfile() {
        const auto file_name_ptr  = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        const auto buffer_ptr     = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG2));
        const auto bytes_to_write = registers.ARG3;

        std::ofstream file(file_name_ptr, std::ios::binary | std::ios::app);
        if (!file) {
            registers.EF = true;
            return;
        }

        file.write(buffer_ptr, bytes_to_write);

        if (file.fail()) {
            registers.EF = true;
        }
    }

    // math
    void vmcall_randint() {
        const auto start = registers.ARG1;
        const auto end   = registers.ARG2;
        int64_t    result;
        if (start == 0 && end == 0)
            result = utils::randint();
        else
            result = utils::randint(start, end);

        stack.push(result);
    }
};
