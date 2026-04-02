# oVM

A hybrid stack-register virtual machine.

oVM executes compiled `.ovm` binaries produced by [oASM](https://github.com/01eksa/oASM) — a Python-based assembler for
the oVM instruction set.

---

## Table of Contents

- [Architecture](#architecture)
    - [Execution Model](#execution-model)
    - [Registers](#registers)
    - [Stack](#stack)
    - [Call Stack](#call-stack)
    - [Data Segment](#data-segment)
    - [Heap](#heap)
- [Binary Format (.ovm)](#binary-format-ovm)
- [Instruction Set](#instruction-set)
    - [Control Flow](#control-flow)
    - [Stack Operations](#stack-operations)
    - [Register Operations](#register-operations)
    - [Memory Operations](#memory-operations)
    - [Integer Arithmetic](#integer-arithmetic)
    - [Floating-Point Arithmetic](#floating-point-arithmetic)
    - [Bitwise Operations](#bitwise-operations)
    - [Logic & Comparison](#logic--comparison)
    - [I/O](#io)
    - [VMCalls](#vmcalls)
- [Building](#building)
- [Usage](#usage)

---

#### Educational project.

- Goal: create a virtual machine that supports integer and real arithmetic, conditional branches, memory operations, I/O
  operations, and basic file I/O.
- Limitations: the stack supports only 8-byte numbers. Multithreading, networking, and advanced file system
  interaction (
  deleting or moving files and folders, creating and renaming folders) are not implemented.

---

## Architecture

### Execution Model

oVM is a hybrid machine: it has a value stack for general computation (like a stack machine) and a registers for boolean
operations and error handling, argument passing, return values, and temporary storage (like a register machine).
The two approaches complement each other — the stack handles expression evaluation naturally, while registers provide a
fast, named scratchpad without stack overhead.

Execution begins at byte offset `0` of the code segment. The **command pointer** (`CP`) advances through
variable-length instructions. Most opcodes are 1 byte; those that carry an immediate operand (jump targets, push
literals, register selectors) are followed by additional bytes as noted in the instruction tables.

The VM uses a **computed dispatch table** — a fixed array of 256 function pointers indexed by opcode byte — so
dispatch cost is a single indirect call per instruction regardless of the total number of opcodes.

### Registers

The VM exposes 32 user-visible 64-bit integer registers (`int64_t`). All registers can hold both integer and
floating-point values; float operations reinterpret the bits as `double` via `memcpy` (no UB, no precision loss from
casting).

| Code        | Name        | Purpose                           |
|-------------|-------------|-----------------------------------|
| `0x00`      | `FR`        | Function register (vmcall number) |
| `0x01–0x08` | `ARG1–ARG8` | Call arguments (caller-populated) |
| `0x09–0x0F` | —           | Reserved                          |
| `0x10`      | `CR`        | Counter register                  |
| `0x11–0x18` | `REG1–REG8` | General-purpose registers         |
| `0x19–0x1F` | —           | Reserved                          |

In addition to the user register file, the VM maintains three internal control registers that are not directly
addressable by user code:

| Register | Type       | Description                                                                  |
|----------|------------|------------------------------------------------------------------------------|
| `CP`     | `uint64_t` | Command pointer — current instruction offset in the code segment             |
| `BF`     | `bool`     | Boolean flag — set by comparison instructions, consumed by conditional jumps |
| `EF`     | `bool`     | Error flag — set by vmcalls and arithmetic ops that can fail                 |

### Stack

The value stack stores `int64_t` cells. Its capacity is set per-binary in the `.ovm` header (default: 512 cells).
Overflow and underflow are detected at runtime and raise exceptions. The stack pointer (`SP`) is read-only from user
code but visible in the debugger.

Pointer values are pushed and popped as raw `uintptr_t` reinterpreted as `int64_t` — this is safe on all supported
64-bit platforms.

### Call Stack

A separate call stack stores return addresses (`uint64_t` CP values) when `CALL` is executed. Its capacity is also set
per-binary in the header (default: 64 frames). Having a dedicated call stack means the return address is never mixed
with data values on the main stack, which simplifies calling conventions and prevents a class of stack-smashing bugs
present in single-stack designs.

`RET` pops the top of the call stack and sets `CP` to that address. `PUSHCP` / `POPCP` allow user code to manually
manipulate the call stack, but in general you should use `CALL` and `RET` instead.

### Data Segment

The data segment is a mutable byte array loaded from the `.ovm` binary. It serves as the primary storage for both
static data (string literals, constants) and program variables — essentially the equivalent of `.data` and `.bss`
sections combined.

There are three ways to access it:

- **`PUSHDATA offset`** — pushes a raw pointer to `data[offset]` onto the stack. Use this to pass a buffer address to
  vmcalls (`printstr`, `scanstr`, `readfile`, etc.). Requires `offset < data_size`.
- **`LOAD offset`** — reads 8 bytes at `data[offset]` and pushes the value onto the stack. Requires
  `offset + 8 ≤ data_size`.
- **`STORE offset`** — pops an `int64_t` from the stack and writes 8 bytes to `data[offset]`. Requires
  `offset + 8 ≤ data_size`.

All three instructions throw a runtime error if the offset is out of bounds.

### Heap

The heap provides dynamically allocated memory independent of the data segment.

- **`ALLOC`** — pops a size, allocates a heap block via `malloc`, pushes the pointer.
- **`FREE`** — pops a pointer and frees the block.
- **`WRITE`** — pops a value, then pops a pointer, writes 8 bytes to `*ptr`.
- **`READ`** — pops a pointer, reads 8 bytes from `*ptr`, pushes the value.
- **`WRITEB`** / **`READB`** — same as `WRITE` / `READ` but operate on a single byte.
- **`memcpy` vmcall** — copies a region of memory; arguments are passed via `ARG1` (dest), `ARG2` (src), `ARG3` (byte
  count).

Heap pointers are stored as `int64_t` on the value stack (raw `uintptr_t` reinterpreted as `int64_t`). There is no
garbage collector; the program is responsible for every `FREE`.

---

## Binary Format (.ovm)

Every `.ovm` file begins with a fixed-size header (packed, no padding):

```
Offset  Size  Field
──────────────────────────────────────────────────────
0        4    magic            — must equal 0x3E4D564F ("OVM>")
4        2    version_major
6        2    version_minor
8        8    data_size        — byte length of the data segment
16       8    code_size        — byte length of the code segment
24       8    stack_size       — value stack capacity (number of int64_t cells)
32       8    call_stack_size  — call stack capacity (number of frames)
──────────────────────────────────────────────────────
Total: 40 bytes
```

After the header the file contains two contiguous segments:

```
[ header 40B ][ data segment: data_size bytes ][ code segment: code_size bytes ]
```

The loader validates the magic number and rejects binaries whose `version_major.version_minor` is newer than the VM's
own version. Older binaries (lower version) load without error, enabling forward compatibility.

---

## Instruction Set

Instructions that carry an immediate 8-byte operand are marked **+8** (total size: 9 bytes). Instructions with one
1-byte register selector are marked **+1** (2 bytes total). Instructions with two 1-byte register selectors are marked *
*+1+1** (3 bytes total). Instructions with a 1-byte register selector followed by an 8-byte immediate are marked **+1+8
** (10 bytes total).

In the tables below, the **Stack** column visualizes the values popped from the value stack *before* the operation.
Values are shown top-to-bottom (e.g., `right<br/>left` means `right` is at the top of the stack, and `left` is below
it). The result of the operation is always pushed back onto the stack unless stated otherwise.

### Control Flow

| Opcode   | Byte   | Size       | Description                                                  |
|----------|--------|------------|--------------------------------------------------------------|
| `EXIT`   | `0x00` | 1          | Terminate execution                                          |
| `VMCALL` | `0x01` | 1          | Call a built-in vmcall; ID is read from the `FR` register    |
| `CALL`   | `0x02` | +8[`addr`] | Push `CP` to call stack, jump to `addr`                      |
| `RET`    | `0x03` | 1          | Pop call stack, jump to return address                       |
| `JMP`    | `0x04` | +8[`addr`] | Unconditional jump to `addr`                                 |
| `JIF`    | `0x05` | +8[`addr`] | Jump to `addr` if `BF == true`                               |
| `JNIF`   | `0x06` | +8[`addr`] | Jump to `addr` if `BF == false`                              |
| `JEF`    | `0x07` | +8[`addr`] | Jump to `addr` if `EF == true`                               |
| `JNEF`   | `0x08` | +8[`addr`] | Jump to `addr` if `EF == false`                              |
| `LOOP`   | `0x09` | +8[`addr`] | Decrement `CR`; if `CR != 0` after decrement, jump to `addr` |
| `DEBUG`  | `0x0A` | 1          | Enter interactive debug console                              |

### Stack Operations

| Opcode     | Byte   | Size         | Stack | Description                                 |
|------------|--------|--------------|-------|---------------------------------------------|
| `PUSH`     | `0x10` | +8[`val`]    | —     | Push 64-bit immediate `val`                 |
| `POP`      | `0x11` | 1            | val   | Discard `val`                               |
| `DUP`      | `0x12` | 1            | val   | Push a copy of `val`                        |
| `PUSHDATA` | `0x13` | +8[`offset`] | —     | Push pointer to `data[offset]`              |
| `LOAD`     | `0x14` | +8[`offset`] | —     | Push 8 bytes at `data[offset]` as `int64_t` |
| `STORE`    | `0x15` | +8[`offset`] | val   | Write `val` (8 bytes) to `data[offset]`     |

### Register Operations

| Opcode    | Byte   | Size               | Stack | Description                                  |
|-----------|--------|--------------------|-------|----------------------------------------------|
| `SETREG`  | `0x30` | +1[`reg`]+8[`val`] | —     | Set register `reg` to 64-bit immediate `val` |
| `PUSHREG` | `0x31` | +1[`reg`]          | —     | Push value of `reg`                          |
| `POPREG`  | `0x32` | +1[`reg`]          | val   | Write `val` to `reg`                         |
| `INCREG`  | `0x33` | +1[`reg`]          | —     | Increment `reg` by 1                         |
| `DECREG`  | `0x34` | +1[`reg`]          | —     | Decrement `reg` by 1                         |
| `CHECKEF` | `0x35` | 1                  | —     | `BF = EF`                                    |
| `CLEAREF` | `0x36` | 1                  | —     | `EF = false`                                 |
| `MOV`     | `0x37` | +1[`reg`]+1[`reg`] | —     | `dst = src`                                  |

### Memory Operations

| Opcode   | Byte   | Size | Stack       | Description                                                   |
|----------|--------|------|-------------|---------------------------------------------------------------|
| `ALLOC`  | `0x40` | 1    | size        | Push pointer to a new heap block of `size` bytes              |
| `FREE`   | `0x41` | 1    | ptr         | Free heap block at address `ptr`                              |
| `WRITE`  | `0x42` | 1    | val<br/>ptr | Write `val` (8 bytes) to address `ptr`                        |
| `READ`   | `0x43` | 1    | ptr         | Push 8 bytes read from address `ptr`                          |
| `WRITEB` | `0x44` | 1    | val<br/>ptr | Write low byte of `val` to address `ptr`                      |
| `READB`  | `0x45` | 1    | ptr         | Push byte read from address `ptr`, zero-extended to `int64_t` |

### Integer Arithmetic

| Opcode | Byte   | Size | Stack          | Description                                            |
|--------|--------|------|----------------|--------------------------------------------------------|
| `INC`  | `0x50` | 1    | val            | `val + 1`                                              |
| `DEC`  | `0x51` | 1    | val            | `val - 1`                                              |
| `ADD`  | `0x52` | 1    | right<br/>left | `left + right`                                         |
| `SUB`  | `0x53` | 1    | right<br/>left | `left - right`                                         |
| `MUL`  | `0x54` | 1    | right<br/>left | `left * right`                                         |
| `DIV`  | `0x55` | 1    | right<br/>left | `left / right`; if `right == 0`: sets `EF`, pushes `0` |
| `MOD`  | `0x56` | 1    | right<br/>left | `left % right`; if `right == 0`: sets `EF`, pushes `0` |
| `POW`  | `0x57` | 1    | exp<br/>base   | `pow(base, exp)` truncated to `int64_t`                |

### Floating-Point Arithmetic

| Opcode | Byte   | Size | Stack          | Description                                                   |
|--------|--------|------|----------------|---------------------------------------------------------------|
| `INCF` | `0x58` | 1    | val            | `val + 1.0`                                                   |
| `DECF` | `0x59` | 1    | val            | `val - 1.0`                                                   |
| `ADDF` | `0x5A` | 1    | right<br/>left | `left + right`                                                |
| `SUBF` | `0x5B` | 1    | right<br/>left | `left - right`                                                |
| `MULF` | `0x5C` | 1    | right<br/>left | `left * right`                                                |
| `DIVF` | `0x5D` | 1    | right<br/>left | `left / right`; if `right == 0.0`: sets `EF`, pushes `0`      |
| `MODF` | `0x5E` | 1    | right<br/>left | `fmod(left, right)`; if `right == 0.0`: sets `EF`, pushes `0` |
| `POWF` | `0x5F` | 1    | exp<br/>base   | `pow(base, exp)`                                              |

### Bitwise Operations

| Opcode | Byte   | Size | Stack          | Description     |
|--------|--------|------|----------------|-----------------|
| `AND`  | `0x60` | 1    | right<br/>left | `left & right`  |
| `OR`   | `0x61` | 1    | right<br/>left | `left \| right` |
| `XOR`  | `0x62` | 1    | right<br/>left | `left ^ right`  |
| `INV`  | `0x63` | 1    | val            | `~val`          |
| `SHL`  | `0x64` | 1    | right<br/>left | `left << right` |
| `SHR`  | `0x65` | 1    | right<br/>left | `left >> right` |

### Logic & Comparison

Comparison results are stored in `BF`, not pushed to the stack.

| Opcode   | Byte   | Size | Stack          | Description                      |
|----------|--------|------|----------------|----------------------------------|
| `NOT`    | `0x66` | 1    | —              | `BF = !BF`                       |
| `CMP`    | `0x67` | 1    | right<br/>left | `BF = (left == right)` (integer) |
| `CMPLT`  | `0x68` | 1    | right<br/>left | `BF = (left < right)` (integer)  |
| `CMPGT`  | `0x69` | 1    | right<br/>left | `BF = (left > right)` (integer)  |
| `CMPF`   | `0x6A` | 1    | right<br/>left | `BF = (left == right)` (float)   |
| `CMPLTF` | `0x6B` | 1    | right<br/>left | `BF = (left < right)` (float)    |
| `CMPGTF` | `0x6C` | 1    | right<br/>left | `BF = (left > right)` (float)    |

### I/O

| Opcode       | Byte   | Size | Stack | Description                                                                   |
|--------------|--------|------|-------|-------------------------------------------------------------------------------|
| `PRINTENDL`  | `0x70` | 1    | —     | Print a newline to stdout                                                     |
| `PRINTINT`   | `0x71` | 1    | val   | Print `val` as `int64_t`                                                      |
| `SCANINT`    | `0x72` | 1    | —     | Read `int64_t` from stdin and push; sets `EF` and pushes `0` on invalid input |
| `PRINTFLOAT` | `0x73` | 1    | val   | Print `val` as `double`                                                       |
| `SCANFLOAT`  | `0x74` | 1    | —     | Read `double` from stdin and push; sets `EF` and pushes `0` on invalid input  |
| `ITOF`       | `0x75` | 1    | val   | Convert `val` from `int64_t` to `double` via `static_cast`, push              |
| `FTOI`       | `0x76` | 1    | val   | Convert `val` from `double` to `int64_t` via `static_cast` (truncates), push  |

### VMCalls

VMCalls are invoked by setting `FR` to the vmcall ID and then executing `VMCALL`. All arguments are passed via the
`ARG1`–`ARG8` registers.

| ID     | Name         | ARGX                                                    | Stack result     | Notes                                                                                          |
|--------|--------------|---------------------------------------------------------|------------------|------------------------------------------------------------------------------------------------|
| `0x00` | `memcpy`     | 1: dest ptr<br/>2: src ptr<br/>3: byte count            | —                | Copies `ARG3` bytes from `ARG2` to `ARG1`                                                      |
| `0x01` | `printstr`   | 1: buffer ptr<br/>2: size (or 0)                        | —                | If `ARG2 == 0`: prints null-terminated string; if `ARG2 > 0`: prints exactly `ARG2` bytes      |
| `0x02` | `scanstr`    | 1: buffer ptr<br/>2: max size                           | —                | Reads one line from stdin into buffer (up to `ARG2 - 1` chars + null); sets `EF` on failure    |
| `0x10` | `readfile`   | 1: filename ptr<br/>2: buffer ptr<br/>3: bytes to read  | bytes read       | Sets `EF` on open failure or read error; always pushes bytes read (`0` on open failure)        |
| `0x11` | `writefile`  | 1: filename ptr<br/>2: buffer ptr<br/>3: bytes to write | —                | Overwrites file (truncates); sets `EF` on failure                                              |
| `0x12` | `appendfile` | 1: filename ptr<br/>2: buffer ptr<br/>3: bytes to write | —                | Appends to file; sets `EF` on failure                                                          |
| `0x20` | `randint`    | 1: min<br/>2: max                                       | random `int64_t` | If `ARG1 == 0 && ARG2 == 0`: full `int64_t` range; otherwise returns a value in `[ARG1, ARG2]` |

---

## Building

**Requirements:** CMake ≥ 3.6, a C++23-capable compiler (GCC 13+, Clang 17+, MSVC 19.38+).

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The resulting binary is `build/VM` (Linux/macOS) or `build/VM.exe` (Windows).

---

## Usage

```
VM <file.ovm> [options]
```

| Option       | Description                                                |
|--------------|------------------------------------------------------------|
| `--no-pause` | Exit immediately after execution (no "Press Enter" prompt) |
| `--debug`    | Reserved for future use                                    |

### Debugger

Inserting a `DEBUG` instruction into your program drops into an interactive console at that point in execution:

```
Debug mode
Commands:
0 - quit debug mode
1 - show registers
2 - show stack
```

Register output shows `CP`, `BF`, `EF`, `SP`, and all named user registers in both hex and decimal. Stack output lists
every value currently on the stack from top to bottom.