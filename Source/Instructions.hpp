#ifndef GBX_INSTRUCTIONS_HPP_
#define GBX_INSTRUCTIONS_HPP_
#include <Utix/Assert.h>
#define ASSERT_INSTR_IMPL() puts(__func__); ASSERT_MSG(false, "Instruction Not Implemented!")

namespace gbx {

struct CPU;
struct Gameboy;

using main_instruction_t = void(* const)(Gameboy* const);
using cb_instruction_t = void(* const)(CPU* const);

extern const main_instruction_t main_table[256];
extern const cb_instruction_t cb_table[256];
















}
#endif
