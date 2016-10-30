#ifndef GBX_INSTRUCTIONS_HPP_
#define GBX_INSTRUCTIONS_HPP_
#include <stdint.h>
#include "debug.hpp"

namespace gbx {

struct Cpu;
struct Gameboy;

using InstructionPtr = void(*)(Gameboy*);
extern const InstructionPtr main_instructions[256];
extern const InstructionPtr cb_instructions[256];
extern const uint8_t clock_table[256];


} // namespace gbx
#endif

