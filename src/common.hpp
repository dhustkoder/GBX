#ifndef GBX_COMMON_HPP_
#define GBX_COMMON_HPP_
#include <stdint.h>
#include <stddef.h>

namespace gbx {

template<class T>
using owner = T;

constexpr int32_t operator""_Kib(unsigned long long kibs)
{
	return static_cast<int32_t>(kibs * 1024);
}

constexpr int32_t operator""_Mib(unsigned long long mibs)
{
	return static_cast<int32_t>(mibs * 1024 * 1024);
}

template<class T>
constexpr T max(const T x, const T y)
{
	return x > y ? x : y;
}

template<class T>
constexpr T min(const T x, const T y)
{
	return x < y ? x : y;
}

template<class F>
struct Finally {
	Finally(const Finally&)=delete;
	Finally& operator=(const Finally&)=delete;
	constexpr explicit Finally(F&& f) noexcept : m_f(static_cast<F&&>(f)) {}
	constexpr Finally(Finally&& other) noexcept = default;
	~Finally() noexcept { m_f(); }
private:
	const F m_f;
};

template<class F> 
constexpr Finally<F> finally(F&& f) 
{
	return Finally<F>(static_cast<F&&>(f));
}


constexpr uint16_t concat_bytes(const uint8_t high_byte, const uint8_t low_byte)
{
	return (high_byte << 8) | low_byte;
}

template<class T>
constexpr bool test_bit(const uint8_t bit, const T value) 
{
	return (value & (static_cast<T>(0x01) << bit)) != 0;
}


template<class T>
constexpr T set_bit(const uint8_t bit, const T value) 
{
	return (value | (static_cast<T>(0x01) << bit));
}

template<class T>
constexpr T res_bit(const uint8_t bit, const T value) 
{
	return (value & ~(static_cast<T>(0x01) << bit));
}



constexpr uint8_t get_low_byte(const uint16_t value) 
{
	return value & 0x00FF;
}

constexpr uint8_t get_high_byte(const uint16_t value) 
{
	return (value & 0xFF00) >> 8;
}




} // namespace gbx
#endif
