#pragma once
#include <cstdint>

enum class OpCode : uint8_t {
    // program execution management
    EXIT    = 0x00,
    VMCALL  = 0x01,
    CALL    = 0x02,     // 9
    RET     = 0x03,
    JMP     = 0x04,     // 9
    JIF     = 0x05,     // 9
    JNIF    = 0x06,     // 9
    JEF     = 0x07,     // 9
    JNEF    = 0x08,     // 9
    LOOP    = 0x09,     // 9
    DEBUG   = 0x0a,

    // stack
    PUSH    = 0x10,    // 9
    POP     = 0x11,
    DUP     = 0x12,
    PEA     = 0x13,    // 9
    LOAD    = 0x14,    // 9
    STORE   = 0x15,    // 9

    // call stack
    PUSHCP = 0x20,
    POPCP  = 0x21,

    // registers
    SETR    = 0x30,     // 10
    PUSHR   = 0x31,     // 2
    POPR    = 0x32,     // 2
    INCR    = 0x33,     // 2
    DECR    = 0x34,     // 2
    CHECKEF = 0x35,
    CLEAREF = 0x36,
    MOV     = 0x37,     // 3
    LEA     = 0x38,     // 10
    LOADR   = 0x39,     // 10
    STORER  = 0x3a,     // 10

    // memory
    ALLOC   = 0x40,
    FREE    = 0x41,
    WRITE   = 0x42,
    READ    = 0x43,
    WRITEB  = 0x44,
    READB   = 0x45,

    // math
    INC = 0x50,
    DEC = 0x51,

    ADD = 0x52,
    SUB = 0x53,
    MUL = 0x54,
    DIV = 0x55,
    MOD = 0x56,
    POW = 0x57,

    INCF = 0x58,
    DECF = 0x59,

    ADDF = 0x5A,
    SUBF = 0x5B,
    MULF = 0x5C,
    DIVF = 0x5D,
    MODF = 0x5E,
    POWF = 0x5F,

    // bit operations
    AND = 0x60,
    OR  = 0x61,
    XOR = 0x62,
    INV = 0x63,
    SHL = 0x64,
    SHR = 0x65,

    // logic
    NOT    = 0x66,
    CMP    = 0x67,
    CMPLT  = 0x68,
    CMPGT  = 0x69,
    CMPF   = 0x6a,
    CMPLTF = 0x6b,
    CMPGTF = 0x6c,

    // basic functions
    PRINTENDL   = 0x70,
    PRINTINT    = 0x71,
    SCANINT     = 0x72,
    PRINTFLOAT  = 0x73,
    SCANFLOAT   = 0x74,
    ITOF        = 0x75,
    FTOI        = 0x76,
};

enum class VMCall : uint8_t {
    // Memory & IO
    memcpy      = 0x00,
    printstr    = 0x01,
    scanstr     = 0x02,

    // Files
    readfile    = 0x10,
    writefile   = 0x11,
    appendfile  = 0x12,

    // Math
    randint     = 0x20,
};
