#ifndef GBX_DEBUG_HPP_
#define GBX_DEBUG_HPP_
#if defined(_DEBUG) || defined(DEBUG)
#include <stdio.h>
#include <stdarg.h>
#endif

namespace gbx {


#if defined(_DEBUG) || defined(DEBUG)
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
