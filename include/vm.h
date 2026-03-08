#pragma once
#include <cstring>
#include <iostream>
#include <limits>
#include <cmath>
#include <format>

#include "stack.h"
#include "callstack.h"
#include "registers.h"
#include "program.h"
#include "memory.h"
#include "opcodes.h"

namespace  io {
    inline bool fix_fail() {
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return true;
        }

        return false;
    }
}

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
    void init_dispatch() {
        dispatch[static_cast<uint8_t>(OpCode::EXIT)] = &VM::op_exit;
        dispatch[static_cast<uint8_t>(OpCode::VMCALL)] = &VM::op_vmcall;
        dispatch[static_cast<uint8_t>(OpCode::CALL)] = &VM::op_call;
        dispatch[static_cast<uint8_t>(OpCode::RET)] = &VM::op_ret;
        dispatch[static_cast<uint8_t>(OpCode::JMP)] = &VM::op_jmp;
        dispatch[static_cast<uint8_t>(OpCode::JIF)] = &VM::op_jif;
        dispatch[static_cast<uint8_t>(OpCode::JNIF)] = &VM::op_jnif;
        dispatch[static_cast<uint8_t>(OpCode::JEF)] = &VM::op_jef;
        dispatch[static_cast<uint8_t>(OpCode::JNEF)] = &VM::op_jnef;
        dispatch[static_cast<uint8_t>(OpCode::LOOP)] = &VM::op_loop;
        dispatch[static_cast<uint8_t>(OpCode::DEBUG)] = &VM::op_debug;

        dispatch[static_cast<uint8_t>(OpCode::PUSH)] = &VM::op_push;
        dispatch[static_cast<uint8_t>(OpCode::POP)] = &VM::op_pop;
        dispatch[static_cast<uint8_t>(OpCode::DUP)] = &VM::op_dup;
        dispatch[static_cast<uint8_t>(OpCode::PUSHDATA)] = &VM::op_push_data;
        dispatch[static_cast<uint8_t>(OpCode::POPDATA)] = &VM::op_pop_data;

        dispatch[static_cast<uint8_t>(OpCode::PUSHCP)] = &VM::op_push_cp;
        dispatch[static_cast<uint8_t>(OpCode::POPCP)] = &VM::op_pop_cp;

        dispatch[static_cast<uint8_t>(OpCode::SETREG)] = &VM::op_set_reg;
        dispatch[static_cast<uint8_t>(OpCode::PUSHREG)] = &VM::op_push_reg;
        dispatch[static_cast<uint8_t>(OpCode::POPREG)] = &VM::op_pop_reg;
        dispatch[static_cast<uint8_t>(OpCode::INCREG)] = &VM::op_inc_reg;
        dispatch[static_cast<uint8_t>(OpCode::DECREG)] = &VM::op_dec_reg;
        dispatch[static_cast<uint8_t>(OpCode::CHECKEF)] = &VM::op_check_ef;
        dispatch[static_cast<uint8_t>(OpCode::CLEAREF)] = &VM::op_clear_ef;
        dispatch[static_cast<uint8_t>(OpCode::MOV)] = &VM::op_mov;


        dispatch[static_cast<uint8_t>(OpCode::ALLOC)] = &VM::op_alloc;
        dispatch[static_cast<uint8_t>(OpCode::FREE)] = &VM::op_free;
        dispatch[static_cast<uint8_t>(OpCode::WRITE)] = &VM::op_write;
        dispatch[static_cast<uint8_t>(OpCode::READ)] = &VM::op_read;

        dispatch[static_cast<uint8_t>(OpCode::INC)] = &VM::op_inc;
        dispatch[static_cast<uint8_t>(OpCode::DEC)] = &VM::op_dec;
        dispatch[static_cast<uint8_t>(OpCode::ADD)] = &VM::op_add;
        dispatch[static_cast<uint8_t>(OpCode::SUB)] = &VM::op_sub;
        dispatch[static_cast<uint8_t>(OpCode::MUL)] = &VM::op_mul;
        dispatch[static_cast<uint8_t>(OpCode::DIV)] = &VM::op_div;
        dispatch[static_cast<uint8_t>(OpCode::MOD)] = &VM::op_mod;
        dispatch[static_cast<uint8_t>(OpCode::POW)] = &VM::op_pow;

        dispatch[static_cast<uint8_t>(OpCode::INCF)] = &VM::op_inc_float;
        dispatch[static_cast<uint8_t>(OpCode::ADDF)] = &VM::op_add_float;
        dispatch[static_cast<uint8_t>(OpCode::SUBF)] = &VM::op_sub_float;
        dispatch[static_cast<uint8_t>(OpCode::MULF)] = &VM::op_mul_float;
        dispatch[static_cast<uint8_t>(OpCode::DIVF)] = &VM::op_div_float;
        dispatch[static_cast<uint8_t>(OpCode::MODF)] = &VM::op_mod_float;
        dispatch[static_cast<uint8_t>(OpCode::POWF)] = &VM::op_pow_float;

        dispatch[static_cast<uint8_t>(OpCode::AND)] = &VM::op_and;
        dispatch[static_cast<uint8_t>(OpCode::OR)] = &VM::op_or;
        dispatch[static_cast<uint8_t>(OpCode::XOR)] = &VM::op_xor;
        dispatch[static_cast<uint8_t>(OpCode::INV)] = &VM::op_inv;
        dispatch[static_cast<uint8_t>(OpCode::SHL)] = &VM::op_shl;
        dispatch[static_cast<uint8_t>(OpCode::SHR)] = &VM::op_shr;

        dispatch[static_cast<uint8_t>(OpCode::NOT)] = &VM::op_not;
        dispatch[static_cast<uint8_t>(OpCode::CMP)] = &VM::op_cmp;
        dispatch[static_cast<uint8_t>(OpCode::CMPLT)] = &VM::op_cmplt;
        dispatch[static_cast<uint8_t>(OpCode::CMPGT)] = &VM::op_cmpgt;
        dispatch[static_cast<uint8_t>(OpCode::CMPF)] = &VM::op_cmp_float;
        dispatch[static_cast<uint8_t>(OpCode::CMPLTF)] = &VM::op_cmplt_float;
        dispatch[static_cast<uint8_t>(OpCode::CMPGTF)] = &VM::op_cmpgt_float;

        dispatch[static_cast<uint8_t>(OpCode::PRINTENDL)] = &VM::op_print_endl;
        dispatch[static_cast<uint8_t>(OpCode::PRINTINT)] = &VM::op_print_int;
        dispatch[static_cast<uint8_t>(OpCode::SCANINT)] = &VM::op_scan_int;
        dispatch[static_cast<uint8_t>(OpCode::PRINTFLOAT)] = &VM::op_print_float;
        dispatch[static_cast<uint8_t>(OpCode::SCANFLOAT)] = &VM::op_scan_float;
    }

    void init_vmcall_dispatch() {
        vmcall_dispatch[0x01] = &VM::vmcall_printstr;
        vmcall_dispatch[0x02] = &VM::vmcall_scanstr;
    }

    std::string debug_info() {
        return std::format("CP: {}\nBF: {}\nEF: {}\nSP: {}",
            registers.CP, registers.BF, registers.EF, stack.get_SP());
    }

    std::string error_message(std::string_view message) {
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

    void op_debug() {

    }

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
        
        stack.push(left / right);
    }

    void op_mod() {
        const auto right = stack.pop();
        const auto left = stack.pop();

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

        stack.push_as<double>(left / right);
    }

    void op_mod_float() {
        const auto right = stack.pop_as<double>();
        const auto left = stack.pop_as<double>();

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

        if (io::fix_fail()) {
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

        if (io::fix_fail()) {
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
