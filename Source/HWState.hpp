#ifndef GBX_HWSTATE_HPP_
#define GBX_HWSTATE_HPP_
#include <stdint.h>

namespace gbx {

enum IntMask : uint8_t {
	IntVBlank = 0x01,
	IntLcdStat = 0x02,
	IntTimer = 0x04,
	IntSerial = 0x08,
	IntJoypad = 0x10
};


struct HWState 
{
	enum Flags : uint8_t {
		IntMasterEnable = 0x01,
		IntMasterActive = 0x02,
		CpuHalt = 0x04,
		TimerStop = 0x08
	};

	bool GetIntMaster() const;
	bool GetIntActive() const;
	Flags GetFlags(Flags hwflags) const;
	uint8_t GetPendentInts() const;
	void EnableIntMaster();
	void EnableIntActive();
	void DisableIntMaster();

	void SetFlags(Flags hwflags);
	void ClearFlags(Flags hwflags);

	void EnableInt(IntMask inter);
	void DisableInt(IntMask inter);
	void RequestInt(IntMask inter);
	void ClearInt(IntMask inter);

	uint16_t tima_clock;
	uint16_t tima_clock_limit;
	uint16_t div_clock;
	uint8_t flags;
	uint8_t div;
	uint8_t tima;
	uint8_t tma;
	uint8_t tac;
	uint8_t int_enable;
	uint8_t int_flags;
};

inline bool HWState::GetIntMaster() const 
{
	return (flags & IntMasterEnable) != 0;
}

inline bool HWState::GetIntActive() const 
{
	return (flags & IntMasterActive) != 0;
}


inline HWState::Flags HWState::GetFlags(const Flags hwflags) const
{
	return static_cast<Flags>(flags & hwflags);
}


inline uint8_t HWState::GetPendentInts() const
{
	return 0x1f & int_enable & int_flags;
}



inline void HWState::EnableIntMaster() 
{
	flags |= IntMasterEnable;
}

inline void HWState::EnableIntActive()
{
	flags |= IntMasterActive;
}

inline void HWState::DisableIntMaster() 
{
	flags &= ~(IntMasterEnable 
	           | IntMasterActive);
}


inline void HWState::SetFlags(const Flags hwflags)
{
	flags |= hwflags;
}

inline void HWState::ClearFlags(const Flags hwflags)
{
	flags &= ~hwflags;
}



inline void HWState::EnableInt(const IntMask inter)
{
	int_enable |= inter;
}

inline void HWState::DisableInt(const IntMask inter)
{
	int_enable &= ~inter;
}

inline void HWState::RequestInt(const IntMask inter)
{
	int_flags |= inter;
}


inline void HWState::ClearInt(const IntMask inter)
{
	int_flags &= ~inter;
}



} // namespace gbx
#endif

