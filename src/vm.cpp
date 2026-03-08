#include "vm.h"
#include "opcodes.h"

void VM::init_dispatch() {
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
        dispatch[static_cast<uint8_t>(OpCode::DECF)] = &VM::op_dec_float;
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
        dispatch[static_cast<uint8_t>(OpCode::ITOF)] = &VM::op_int_to_float;
        dispatch[static_cast<uint8_t>(OpCode::FTOI)] = &VM::op_float_to_int;
}

void VM::init_vmcall_dispatch() {
        vmcall_dispatch[0x01] = &VM::vmcall_printstr;
        vmcall_dispatch[0x02] = &VM::vmcall_scanstr;
}

void VM::op_debug() {
        bool debug = true;
        std::cout
            << "Debug mode\n"
            << "Commands:\n"
            << "0 - quit debug mode\n"
            << "1 - show registers\n"
            << "2 - show stack\n";

        while (debug) {
                uint16_t command;
                std::cout << "> ";
                std::cin >> command;

                if (utils::fix_fail()) {
                        std::cout << "Incorrect command\n";
                        continue;
                }

                switch (command) {
                        case 0:
                                debug = false;
                                break;
                        case 1:
                                std::cout
                                    << "SP:   " << stack.get_SP() << '\n'

                                    << "CP:   " << registers.CP << '\n'
                                    << "BF:   " << registers.BF << '\n'
                                    << "EF:   " << registers.EF << '\n'
                                    << "CR:   " << registers.CR << '\n'

                                    << "FR:   " << registers.FR << '\n'
                                    << "ARG1: " << registers.ARG1 << '\n'
                                    << "ARG2: " << registers.ARG2 << '\n'
                                    << "ARG3: " << registers.ARG3 << '\n'
                                    << "ARG4: " << registers.ARG4 << '\n'

                                    << "REG1: " << registers.REG1 << '\n'
                                    << "REG2: " << registers.REG2 << '\n'
                                    << "REG3: " << registers.REG3 << '\n'
                                    << "REG4: " << registers.REG4 << '\n';
                                break;
                        case 2:
                                std::cout << std::hex;
                                for (uint64_t i = 0; i < stack.get_SP(); i++) {
                                        std::cout << "0x" << stack.look(i) << ' ';
                                }
                                std::cout << std::dec << std::endl;
                                break;
                        default:
                                std::cout << "Unknown command\n";
                }

        }
}
