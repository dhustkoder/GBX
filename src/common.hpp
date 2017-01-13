#ifndef GBX_COMMON_HPP_
#define GBX_COMMON_HPP_
#include <stdint.h>
#include <stddef.h>

namespace gbx {

template<class T>
using owner = T;

constexpr size_t operator"" _Kib(unsigned long long kibs)
{
	return static_cast<size_t>(kibs * 1024);
}

constexpr size_t operator"" _Mib(unsigned long long mibs)
{
	return static_cast<size_t>(mibs * 1024 * 1024);
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

template<class T, size_t N>
constexpr size_t arr_size(const T(&)[N])
{
	return N;
}

template<class F>
class Finally {
public:
	Finally(const Finally&) = delete;
	Finally& operator=(const Finally&) = delete;
	Finally& operator=(Finally&&) = delete;

	constexpr Finally(F&& func) noexcept :
		m_func(static_cast<F&&>(func))
	{
	}

	Finally(Finally&& other) noexcept :
		m_func(static_cast<F&&>(other.m_func)),
		m_abort(other.m_abort)
	{
		other.abort();
	}

	~Finally() noexcept 
	{
		if (!m_abort)
			m_func();
	}

	void abort() noexcept
	{
		m_abort = true;
	}

private:
	F m_func;
	bool m_abort = false;
};


template<class F> 
constexpr Finally<F> finally(F&& f) 
{
	return {static_cast<F&&>(f)};
}


constexpr uint16_t concat_bytes(const uint8_t msb, const uint8_t lsb)
{
	return (msb << 8) | lsb;
}

template<class T>
constexpr bool test_bit(const int bit, const T value) 
{
	return (value & (static_cast<T>(0x01) << bit)) != 0;
}


template<class T>
constexpr T set_bit(const int bit, const T value) 
{
	return (value | (static_cast<T>(0x01) << bit));
}

template<class T>
constexpr T res_bit(const int bit, const T value) 
{
	return (value & ~(static_cast<T>(0x01) << bit));
}


template<class T>
constexpr uint8_t get_lsb(const T value) 
{
	return value & 0xFF;
}

template<class T>
constexpr uint8_t get_msb(const T value)
{
	static_assert(sizeof(T) > 1, "");
	return (value >> ((sizeof(T) - 1) * 8)) & 0xFF;
}

template<class T, const size_t ArrSize>
bool is_in_array(const T(&array)[ArrSize], const T& value)
{
	for (const auto& elem : array) {
		if (elem == value)
			return true;
	}
	return false;
}


} // namespace gbx
#endif

