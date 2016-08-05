#ifndef GBX_HWSTATE_HPP_
#define GBX_HWSTATE_HPP_
#include <Utix/Ints.h>

namespace gbx {


enum Interrupt : uint8_t
{
	INTERRUPT_VBLANK = 0x01,
	INTERRUPT_LCD_STAT = 0x02,
	INTERRUPT_TIMER = 0x04,
	INTERRUPT_SERIAL = 0x08,
	INTERRUPT_JOYPAD = 0x10
};


struct HWState
{

	enum HWFlags : uint8_t
	{
		INTERRUPT_MASTER_ENABLED = 0x01,
		INTERRUPT_MASTER_ACTIVE = 0x02
	};

	bool GetIntMaster() const;
	bool GetIntActive() const;
	void EnableIntMaster();
	void EnableIntActive();
	void DisableIntMaster();

	void EnableInterrupt(const Interrupt intrr);
	void DisableInterrupt(const Interrupt interr);
	void RequestInterrupt(const Interrupt interr);
	void ClearInterrupt(const Interrupt interr);

	uint8_t flags;
	uint8_t interrupt_enable;
	uint8_t interrupt_flags;
};




inline bool HWState::GetIntMaster() const 
{
	return (flags & HWFlags::INTERRUPT_MASTER_ENABLED) != 0;
}

inline bool HWState::GetIntActive() const 
{
	return (flags & HWFlags::INTERRUPT_MASTER_ACTIVE) != 0;
}


inline void HWState::EnableIntMaster() 
{
	flags |= HWFlags::INTERRUPT_MASTER_ENABLED;
}

inline void HWState::EnableIntActive()
{
	flags |= HWFlags::INTERRUPT_MASTER_ACTIVE;
}

inline void HWState::DisableIntMaster() 
{
	flags &= ~(HWFlags::INTERRUPT_MASTER_ENABLED 
	           | HWFlags::INTERRUPT_MASTER_ACTIVE);
}




inline void HWState::EnableInterrupt(const Interrupt inter)
{
	interrupt_enable |= static_cast<uint8_t>(inter);
}

inline void HWState::DisableInterrupt(const Interrupt inter)
{
	interrupt_enable &= ~static_cast<uint8_t>(inter);
}

inline void HWState::RequestInterrupt(const Interrupt inter)
{
	interrupt_flags |= static_cast<uint8_t>(inter);
}


inline void HWState::ClearInterrupt(const Interrupt inter)
{
	interrupt_flags &= ~static_cast<uint8_t>(inter);
}














}
#endif
