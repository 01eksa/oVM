#include "vm.h"

#include "opcodes.h"

void VM::init_dispatch() {
    op_dispatch[static_cast<uint8_t>(OpCode::EXIT)]   = &VM::op_exit;
    op_dispatch[static_cast<uint8_t>(OpCode::VMCALL)] = &VM::op_vmcall;
    op_dispatch[static_cast<uint8_t>(OpCode::CALL)]   = &VM::op_call;
    op_dispatch[static_cast<uint8_t>(OpCode::RET)]    = &VM::op_ret;
    op_dispatch[static_cast<uint8_t>(OpCode::JMP)]    = &VM::op_jmp;
    op_dispatch[static_cast<uint8_t>(OpCode::JIF)]    = &VM::op_jif;
    op_dispatch[static_cast<uint8_t>(OpCode::JNIF)]   = &VM::op_jnif;
    op_dispatch[static_cast<uint8_t>(OpCode::JEF)]    = &VM::op_jef;
    op_dispatch[static_cast<uint8_t>(OpCode::JNEF)]   = &VM::op_jnef;
    op_dispatch[static_cast<uint8_t>(OpCode::LOOP)]   = &VM::op_loop;
    op_dispatch[static_cast<uint8_t>(OpCode::DEBUG)]  = &VM::op_debug;

    op_dispatch[static_cast<uint8_t>(OpCode::PUSH)]  = &VM::op_push;
    op_dispatch[static_cast<uint8_t>(OpCode::POP)]   = &VM::op_pop;
    op_dispatch[static_cast<uint8_t>(OpCode::DUP)]   = &VM::op_dup;
    op_dispatch[static_cast<uint8_t>(OpCode::PEA)]   = &VM::op_pea;
    op_dispatch[static_cast<uint8_t>(OpCode::LOAD)]  = &VM::op_load;
    op_dispatch[static_cast<uint8_t>(OpCode::STORE)] = &VM::op_store;

    op_dispatch[static_cast<uint8_t>(OpCode::PUSHCP)] = &VM::op_push_cp;
    op_dispatch[static_cast<uint8_t>(OpCode::POPCP)]  = &VM::op_pop_cp;

    op_dispatch[static_cast<uint8_t>(OpCode::SETR)]    = &VM::op_set_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::PUSHR)]   = &VM::op_push_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::POPR)]    = &VM::op_pop_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::INCR)]    = &VM::op_inc_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::DECR)]    = &VM::op_dec_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::CHECKEF)] = &VM::op_check_ef;
    op_dispatch[static_cast<uint8_t>(OpCode::CLEAREF)] = &VM::op_clear_ef;
    op_dispatch[static_cast<uint8_t>(OpCode::MOV)]     = &VM::op_mov;
    op_dispatch[static_cast<uint8_t>(OpCode::LEA)]     = &VM::op_lea;
    op_dispatch[static_cast<uint8_t>(OpCode::LOADR)]   = &VM::op_load_reg;
    op_dispatch[static_cast<uint8_t>(OpCode::STORER)]  = &VM::op_store_reg;

    op_dispatch[static_cast<uint8_t>(OpCode::ALLOC)]  = &VM::op_alloc;
    op_dispatch[static_cast<uint8_t>(OpCode::FREE)]   = &VM::op_free;
    op_dispatch[static_cast<uint8_t>(OpCode::WRITE)]  = &VM::op_write;
    op_dispatch[static_cast<uint8_t>(OpCode::READ)]   = &VM::op_read;
    op_dispatch[static_cast<uint8_t>(OpCode::WRITEB)] = &VM::op_write_byte;
    op_dispatch[static_cast<uint8_t>(OpCode::READB)]  = &VM::op_read_byte;

    op_dispatch[static_cast<uint8_t>(OpCode::INC)] = &VM::op_inc;
    op_dispatch[static_cast<uint8_t>(OpCode::DEC)] = &VM::op_dec;
    op_dispatch[static_cast<uint8_t>(OpCode::ADD)] = &VM::op_add;
    op_dispatch[static_cast<uint8_t>(OpCode::SUB)] = &VM::op_sub;
    op_dispatch[static_cast<uint8_t>(OpCode::MUL)] = &VM::op_mul;
    op_dispatch[static_cast<uint8_t>(OpCode::DIV)] = &VM::op_div;
    op_dispatch[static_cast<uint8_t>(OpCode::MOD)] = &VM::op_mod;
    op_dispatch[static_cast<uint8_t>(OpCode::POW)] = &VM::op_pow;

    op_dispatch[static_cast<uint8_t>(OpCode::INCF)] = &VM::op_inc_float;
    op_dispatch[static_cast<uint8_t>(OpCode::DECF)] = &VM::op_dec_float;
    op_dispatch[static_cast<uint8_t>(OpCode::ADDF)] = &VM::op_add_float;
    op_dispatch[static_cast<uint8_t>(OpCode::SUBF)] = &VM::op_sub_float;
    op_dispatch[static_cast<uint8_t>(OpCode::MULF)] = &VM::op_mul_float;
    op_dispatch[static_cast<uint8_t>(OpCode::DIVF)] = &VM::op_div_float;
    op_dispatch[static_cast<uint8_t>(OpCode::MODF)] = &VM::op_mod_float;
    op_dispatch[static_cast<uint8_t>(OpCode::POWF)] = &VM::op_pow_float;

    op_dispatch[static_cast<uint8_t>(OpCode::AND)] = &VM::op_and;
    op_dispatch[static_cast<uint8_t>(OpCode::OR)]  = &VM::op_or;
    op_dispatch[static_cast<uint8_t>(OpCode::XOR)] = &VM::op_xor;
    op_dispatch[static_cast<uint8_t>(OpCode::INV)] = &VM::op_inv;
    op_dispatch[static_cast<uint8_t>(OpCode::SHL)] = &VM::op_shl;
    op_dispatch[static_cast<uint8_t>(OpCode::SHR)] = &VM::op_shr;

    op_dispatch[static_cast<uint8_t>(OpCode::NOT)]    = &VM::op_not;
    op_dispatch[static_cast<uint8_t>(OpCode::CMP)]    = &VM::op_cmp;
    op_dispatch[static_cast<uint8_t>(OpCode::CMPLT)]  = &VM::op_cmplt;
    op_dispatch[static_cast<uint8_t>(OpCode::CMPGT)]  = &VM::op_cmpgt;
    op_dispatch[static_cast<uint8_t>(OpCode::CMPF)]   = &VM::op_cmp_float;
    op_dispatch[static_cast<uint8_t>(OpCode::CMPLTF)] = &VM::op_cmplt_float;
    op_dispatch[static_cast<uint8_t>(OpCode::CMPGTF)] = &VM::op_cmpgt_float;

    op_dispatch[static_cast<uint8_t>(OpCode::PRINTENDL)]  = &VM::op_print_endl;
    op_dispatch[static_cast<uint8_t>(OpCode::PRINTINT)]   = &VM::op_print_int;
    op_dispatch[static_cast<uint8_t>(OpCode::SCANINT)]    = &VM::op_scan_int;
    op_dispatch[static_cast<uint8_t>(OpCode::PRINTFLOAT)] = &VM::op_print_float;
    op_dispatch[static_cast<uint8_t>(OpCode::SCANFLOAT)]  = &VM::op_scan_float;
    op_dispatch[static_cast<uint8_t>(OpCode::ITOF)]       = &VM::op_int_to_float;
    op_dispatch[static_cast<uint8_t>(OpCode::FTOI)]       = &VM::op_float_to_int;
}

void VM::init_vmcall_dispatch() {
    vmcall_dispatch[static_cast<uint8_t>(VMCall::memcpy)]   = &VM::vmcall_memcpy;
    vmcall_dispatch[static_cast<uint8_t>(VMCall::printstr)] = &VM::vmcall_printstr;
    vmcall_dispatch[static_cast<uint8_t>(VMCall::scanstr)]  = &VM::vmcall_scanstr;

    vmcall_dispatch[static_cast<uint8_t>(VMCall::readfile)]   = &VM::vmcall_readfile;
    vmcall_dispatch[static_cast<uint8_t>(VMCall::writefile)]  = &VM::vmcall_writefile;
    vmcall_dispatch[static_cast<uint8_t>(VMCall::appendfile)] = &VM::vmcall_appendfile;

    vmcall_dispatch[static_cast<uint8_t>(VMCall::randint)] = &VM::vmcall_randint;
}

std::string VM::format_VM_registers() const {
    return std::format(
        "\nVM Registers:\n"
        "Name   | Value\n"
        "{:<6} | {}\n"
        "{:<6} | {}\n"
        "{:<6} | {}\n"
        "{:<6} | {}\n",
        "CP",
        registers.CP,
        "BF",
        registers.BF ? "true" : "false",
        "EF",
        registers.EF ? "true" : "false",
        "SP",
        stack.get_SP()
    );
}

std::string VM::error_message(std::string_view message) const {
    return std::format("{}\nDebug info:\n{}", message, format_VM_registers());
}

std::string VM::format_user_registers() const {
    auto result = std::string(
        "\nUser Registers:\n"
        "Name   |  HEX        |  DEC\n"
    );

    for (int i = 0; i < Registers::REG_COUNT; i++) {
        if (Registers::names[i].starts_with("RES")) continue;
        result += std::format("{0:<6} | {1:+#011x} | {1:+#0d}\n", Registers::names[i], registers.registers[i]);
    }

    return result;
}

std::string VM::format_stack() const {
    auto result = std::string(
        "\nStack:\n"
        "Index    |  HEX        |  DEC\n"
    );

    for (uint64_t i = stack.get_SP(); i > 0; i--) {
        result += std::format("{0:<8} | {1:+#011x} | {1:+#0d}\n", i, stack.look(i - 1));
    }

    return result;
}

void VM::op_debug() {
    if (!debug_enabled) return;

    bool debug = true;
    std::cout << "Debug mode\n"
              << "Commands:\n"
              << "0 - quit debug mode\n"
              << "1 - show registers\n"
              << "2 - show stack\n";

    while (debug) {
        uint16_t command;
        std::cout << "> ";
        std::cin >> command;

        if (utils::fix_fail()) {
            std::cout << "Invalid input\n";
            continue;
        }

        switch (command) {
            case 0:
                debug = false;
                break;
            case 1:
                std::cout << format_VM_registers() << format_user_registers();
                break;
            case 2:
                std::cout << format_stack();
                break;
            default:
                std::cout << "Unknown command\n";
        }
    }
}
