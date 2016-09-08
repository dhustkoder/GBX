#ifndef GBX_INSTRUCTIONS_HPP_
#define GBX_INSTRUCTIONS_HPP_
#include <Utix/Ints.h>
#include <Utix/Assert.h>
#include "Debug.hpp"
#define ASSERT_INSTR_IMPL() debug_puts(__func__); ASSERT_MSG(false, "Instruction Not Implemented!")

namespace gbx {

struct CPU;
struct Gameboy;

using instruction_table_t = void(*)(Gameboy*);

extern const instruction_table_t main_instructions[256];
extern const instruction_table_t cb_instruction[256];

extern const uint8_t clock_table[256];











}
#endif
