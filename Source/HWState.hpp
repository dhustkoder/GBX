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
	uint8_t GetPendentInts() const;
	void EnableIntMaster();
	void EnableIntActive();
	void DisableIntMaster();

	void EnableInt(const Interrupt intrr);
	void DisableInt(const Interrupt interr);
	void RequestInt(const Interrupt interr);
	void ClearInt(const Interrupt interr);



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


inline uint8_t HWState::GetPendentInts() const
{
	return 0x1f & interrupt_enable & interrupt_flags;
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




inline void HWState::EnableInt(const Interrupt inter)
{
	interrupt_enable |= inter;
}

inline void HWState::DisableInt(const Interrupt inter)
{
	interrupt_enable &= ~inter;
}

inline void HWState::RequestInt(const Interrupt inter)
{
	interrupt_flags |= inter;
}


inline void HWState::ClearInt(const Interrupt inter)
{
	interrupt_flags &= ~inter;
}














}
#endif
