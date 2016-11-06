#ifndef GBX_DEBUG_HPP_
#define GBX_DEBUG_HPP_
#if defined(_DEBUG) || defined(DEBUG)
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <assert.h>

#define GBX_DEBUG

#if defined(__linux__) || defined(__APPLE__)
#define GBX_DEBUG_BREAK() raise(SIGTRAP)
#elif defined(_WIN32)
#define GBX_DEBUG_BREAK() __debugbreak()
#else
#error "Could not define GBX_DEBUG_BREAK"
#endif

#else
#define GBX_DEBUG_BREAK()
#define assert(...)
#endif

namespace gbx {


#ifdef GBX_DEBUG

inline void debug_puts(const char* str) 
{
	puts(str);
}

inline void debug_printf(const char* fmt, ...) 
{
	va_list arg_list;
	va_start(arg_list, fmt);
	vprintf(fmt, arg_list);
	va_end(arg_list);
}

#else

inline void debug_puts(const char*) {}
inline void debug_printf(const char*, ...) {}

#endif



} // namespace gbx
#endif

