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



constexpr bool TestBit(const uint8_t bit, const uint16_t value) {
	return (value & (0x01 << bit)) != 0;
}


template<class T>
constexpr T SetBit(const uint8_t bit, const T value) {
	return (value | (0x01 << bit));
}

template<class T>
constexpr T ResBit(const uint8_t bit, const T value) {
	return (value & ~(0x01 << bit));
}




constexpr uint8_t GetLowByte(const uint16_t value) {
	return value & 0x00FF;
}


constexpr uint8_t GetHighByte(const uint16_t value) {
	return (value & 0xFF00) >> 8;
}



constexpr uint8_t GetLowNibble(const uint8_t byte) {
	return byte & 0x0F;
}



constexpr uint8_t GetHighNibble(const uint8_t byte) {
	return byte & 0xF0;
}




















}
#endif
