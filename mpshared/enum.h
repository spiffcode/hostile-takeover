#ifndef __ENUM_H__
#define __ENUM_H__

#include "inc/basictypes.h"

namespace wi {

// Enum class, used for enumeration

const dword kEnmFirst = (dword)-1;

class Enum // enm
{
public:
    Enum()
	{
        Reset();
    }
    void Reset()
	{
        m_pvNext = (void *)kEnmFirst;
		m_dwUser = kEnmFirst;
		m_wUser = (word)kEnmFirst;
    }

    void *m_pvNext;
    dword m_dwUser;
	word m_wUser;
};

} // namespace wi

#endif // __ENUM_H__
