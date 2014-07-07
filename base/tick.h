#ifndef __TICK_H__
#define __TICK_H__

#include "inc/basictypes.h"

namespace base {

extern long64 GetTickCount();
extern long64 GetMillisecondCount();
extern dword GetSecondsUnixEpocUTC();

} // namespace base

#endif // __TICK_H__
