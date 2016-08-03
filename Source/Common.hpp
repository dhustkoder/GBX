#ifndef GBX_COMMON_HPP_
#define GBX_COMMON_HPP_
#include <Utix/Ints.h>




namespace gbx {

constexpr size_t operator""_Kib(unsigned long long kibs) {
	return static_cast<size_t>(sizeof(uint8_t) * kibs * 1024);
}

constexpr size_t operator""_Mib(unsigned long long mibs) {
	return static_cast<size_t>(sizeof(uint8_t) * mibs * 1024 * 1024);
}



constexpr uint16_t ConcatBytes(const uint8_t high_byte, const uint8_t low_byte) {
	return (high_byte << 8) | low_byte;
}




constexpr bool GetBit(const uint16_t value, const uint8_t bit) {
	return (value & ( 1 << bit )) != 0;
}


constexpr uint8_t GetLowByte(const uint16_t value) {
	return value & 0x00FF;
}


constexpr uint8_t GetHighByte(const uint16_t value) {
	return (value & 0xFF00) >> 8;
}



constexpr uint8_t GetLowNibble(uint8_t byte) {
	return byte & 0x0F;
}



constexpr uint8_t GetHighNibble(uint8_t byte) {
	return byte & 0xF0;
}





inline void Split16(const uint16_t value, uint8_t* const high_byte, uint8_t* const low_byte) {
	*high_byte = GetHighByte(value);
	*low_byte = GetLowByte(value);
}



inline void Add16(const uint16_t val, uint8_t* const high_byte, uint8_t* const low_byte) {
	const uint16_t result = ConcatBytes(*high_byte, *low_byte) + val;
	Split16(result, high_byte, low_byte);
}



inline void Sub16(const uint16_t val, uint8_t* const high_byte, uint8_t* const low_byte) {
	const uint16_t result = ConcatBytes(*high_byte, *low_byte) - val;
	Split16(result, high_byte, low_byte);
}




















}
#endif
