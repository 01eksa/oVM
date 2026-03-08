#pragma once
#include <cstring>
#include <iostream>
#include <cmath>
#include <format>

#include "stack.h"
#include "callstack.h"
#include "registers.h"
#include "program.h"
#include "memory.h"
#include "utils.h"

class VM {
    using Handler = void(VM::*)();

    Handler dispatch[256] = {};
    Handler vmcall_dispatch[256] = {};

    std::unique_ptr<uint8_t[]> code;
    uint64_t code_size;

    std::unique_ptr<uint8_t[]> data;
    uint64_t data_size;

    Registers registers;
    Stack stack;
    CallStack call_stack;

    bool running = false;

public:
    explicit VM(Program p) :
    code(std::move(p.code)),
    code_size(p.code_size),
    data(std::move(p.data)),
    data_size(p.data_size),
    stack(p.stack_size ? p.stack_size : Stack::DEFAULT_CAPACITY),
    call_stack(p.call_stack_size ? p.call_stack_size : CallStack::DEFAULT_CAPACITY) {}
    ~VM() = default;

    void run() {
        init_dispatch();
        init_vmcall_dispatch();

        running = true;

        while (running) {
            if (registers.CP >= code_size) {
                throw std::runtime_error("Command pointer out of range!");
            }
            const auto op = eat<uint8_t>();

            if (const auto handler = dispatch[op]) {
                (this->*handler)();
            }
            else {
                throw std::runtime_error(std::format("Unknown command: {:x}", op));
            }
        }

    }

private:
    void init_dispatch();

    void init_vmcall_dispatch();

    std::string debug_info() const {
        return std::format("CP: {}\nBF: {}\nEF: {}\nSP: {}",
            registers.CP, registers.BF, registers.EF, stack.get_SP());
    }

    std::string error_message(std::string_view message) const {
        return std::format("{}\nDebug info:\n{}", message, debug_info());
    }

    template <typename T>
    T eat() {
        T result;
        std::memcpy(&result, &code[registers.CP], sizeof(T));
        registers.CP += sizeof(T);

        return result;
    }

    template <typename T>
    void write_reg(const T reg, const int64_t value) {
        switch (static_cast<RegisterCode>(reg)) {
            case RegisterCode::FR:
                registers.FR = value;
                break;
            case RegisterCode::ARG1:
                registers.ARG1 = value;
                break;
            case RegisterCode::ARG2:
                registers.ARG2 = value;
                break;
            case RegisterCode::ARG3:
                registers.ARG3 = value;
                break;
            case RegisterCode::ARG4:
                registers.ARG4 = value;
                break;
            case RegisterCode::REG1:
                registers.REG1 = value;
                break;
            case RegisterCode::REG2:
                registers.REG2 = value;
                break;
            case RegisterCode::REG3:
                registers.REG3 = value;
                break;
            case RegisterCode::REG4:
                registers.REG4 = value;
                break;
            case RegisterCode::CR:
                registers.CR = value;
                break;
            default:
                registers.EF = true;
        }
    }

    template <typename T>
    int64_t read_reg(T reg) {
        int64_t value = 0;
        switch (static_cast<RegisterCode>(reg)) {
            case RegisterCode::FR:
                value = registers.FR;
                break;
            case RegisterCode::ARG1:
                value = registers.ARG1;
                break;
            case RegisterCode::ARG2:
                value = registers.ARG2;
                break;
            case RegisterCode::ARG3:
                value = registers.ARG3;
                break;
            case RegisterCode::ARG4:
                value = registers.ARG4;
                break;
            case RegisterCode::REG1:
                value = registers.REG1;
                break;
            case RegisterCode::REG2:
                value = registers.REG2;
                break;
            case RegisterCode::REG3:
                value = registers.REG3;
                break;
            case RegisterCode::REG4:
                value = registers.REG4;
                break;
            case RegisterCode::CR:
                value = registers.CR;
                break;
            default:
                registers.EF = true;
        }

        return value;
    }

    // program execution management
    void op_exit() {
        running = false;
    }

    void op_vmcall() {
        if (const auto handler = vmcall_dispatch[registers.FR])
            (this->*handler)();
        else
            throw std::runtime_error(std::format("Unknown vmcall: {:x}", registers.FR));
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
        registers.CP = addr;
    }

    void op_jif() {
        const auto addr = eat<uint64_t>();
        if (registers.BF)
            registers.CP = addr;
    }

    void op_jnif() {
        const auto addr = eat<uint64_t>();
        if (!registers.BF)
            registers.CP = addr;
    }

    void op_jef() {
        const auto addr = eat<uint64_t>();
        if (registers.EF)
            registers.CP = addr;
    }

    void op_jnef() {
        const auto addr = eat<uint64_t>();
        if (!registers.EF)
            registers.CP = addr;
    }

    void op_loop() {
        const auto addr = eat<uint64_t>();
        if (--registers.CR > 0)
            registers.CP = addr;
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

    void op_push_data() {
        const auto offset = eat<uint64_t>();
        if (offset < data_size) {
            const auto ptr = static_cast<int64_t>(reinterpret_cast<uintptr_t>(&data[offset]));
            stack.push(ptr);
        }
        else
            throw std::runtime_error(error_message("Offset larger than data size"));
    }

    void op_pop_data() {
        const auto offset = eat<uint64_t>();
        const auto bits = stack.pop();
        if (offset + sizeof(bits) < data_size) {
            const auto ptr = &data[offset];
            std::memcpy(ptr, &bits, sizeof(bits));
        }
        else
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
        const auto reg = eat<uint8_t>();
        const auto value = eat<int64_t>();

        write_reg(reg, value);
    }

    void op_push_reg() {
        const auto reg = eat<uint8_t>();
        const auto val = read_reg(reg);

        stack.push(val);
    }

    void op_pop_reg() {
        const auto reg = eat<uint8_t>();
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
        const auto to = eat<uint8_t>();
        const auto from = eat<uint8_t>();

        write_reg(to, read_reg(from));
    }

    // memory
    void op_alloc() {
        const auto size = stack.pop();
        const auto ptr = Memory::alloc(size);

        stack.push_ptr(ptr);
    }

    void op_free() {
        const auto ptr = stack.pop_ptr();
        
        Memory::free(ptr);
    }

    void op_write() {
        const auto bits = stack.pop();
        const auto ptr = stack.pop_ptr();

        std::memcpy(ptr, &bits, sizeof(bits));
    }

    void op_read() {
        const auto ptr = stack.pop_ptr();
        int64_t bits;
        std::memcpy(&bits, ptr, sizeof(bits));

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
        const auto left = stack.pop();
        
        stack.push(left + right);
    }
    
    void op_sub() {
        const auto right = stack.pop();
        const auto left = stack.pop();
        
        stack.push(left - right);
    }
    
    void op_mul() {
        const auto right = stack.pop();
        const auto left = stack.pop();
        
        stack.push(left * right);
    }
    
    void op_div() {
        const auto right = stack.pop();
        const auto left = stack.pop();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }
        
        stack.push(left / right);
    }

    void op_mod() {
        const auto right = stack.pop();
        const auto left = stack.pop();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push(left % right);
    }

    void op_pow() {
        const auto right = stack.pop();
        const auto left = stack.pop();

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
        const auto left = stack.pop_as<double>();

        stack.push_as<double>(left + right);
    }

    void op_sub_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

        stack.push_as<double>(left - right);
    }

    void op_mul_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

        stack.push_as<double>(left * right);
    }

    void op_div_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push_as<double>(left / right);
    }

    void op_mod_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();
        if (right == 0) {
            registers.EF = true;
            stack.push(0);
            return;
        }

        stack.push_as<double>(std::fmod(left, right));
    }

    void op_pow_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

        stack.push_as<double>(std::pow(left, right));
    }

    // bit operations
    void op_and() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        stack.push(left & right);
    }

    void op_or() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        stack.push(left | right);
    }

    void op_xor() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        stack.push(left ^ right);
    }

    void op_inv() {
        stack.push(~stack.pop());
    }

    void op_shl() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        stack.push(left << right);
    }

    void op_shr() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        stack.push(left >> right);
    }

    // logic
    void op_not() {
        registers.BF = !registers.BF;
    }

    void op_cmp() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        registers.BF = left == right;
    }

    void op_cmplt() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        registers.BF = left < right;
    }

    void op_cmpgt() {
        const auto right = stack.pop();
        const auto left = stack.pop();

        registers.BF = left > right;
    }

    void op_cmp_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

        registers.BF = left == right;
    }

    void op_cmplt_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

        registers.BF = left < right;
    }

    void op_cmpgt_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

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
            val = 0;
        }

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
            val = 0;
        }

        stack.push_as<double>(val);
    }

    // VM calls
    void vmcall_printstr() {
        const auto ptr = static_cast<uintptr_t>(registers.ARG1);
        std::cout << reinterpret_cast<const char*>(ptr);
    }

    void vmcall_scanstr() {
        const auto ptr = reinterpret_cast<char*>(static_cast<uintptr_t>(registers.ARG1));
        const auto buffer_size = registers.ARG2;

        std::cin.getline(ptr, buffer_size);
    }
};
