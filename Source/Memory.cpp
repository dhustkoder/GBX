#include <stdio.h>
#include "Memory.hpp"

namespace gbx {




int8_t Memory::ReadS8(const uint16_t address) const {
	// TODO: this might not be totally portable 
	return static_cast<int8_t>(ReadU8(address));
}






uint8_t Memory::ReadU8(const uint16_t address) const {
	return data[address];
}





uint16_t Memory::ReadU16(const uint16_t address) const {
	return ConcatBytes(data[address + 1], data[address]);
}





void Memory::WriteU8(const uint16_t address, const uint8_t value) {
	data[address] = value;
}






void Memory::WriteU16(const uint16_t address, const uint16_t value) {
	Split16(value, &data[address + 1], &data[address]);
}















	

}
