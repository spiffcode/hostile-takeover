#ifndef __INPUT_H__
#define __INPUT_H__

#include "base/messagequeue.h"

const int kidmMouseDown = 1;
const int kidmMouseUp = 2;
const int kidmMouseMove = 3;
const int kidmMouseDown2 = 4;
const int kidmMouseUp2 = 5;
const int kidmMouseMove2 = 6;
const int kidmAppTerminate = 7;
const int kidmBreakEvent = 8;
//const int kidmNullEvent = 9; // now: base::kidmNullEvent
//const int kidmTransportEvent = 10; // now: base::kidmTransportEvent
const int kidmReceivedResponse = 11;
const int kidmReceivedData = 12;
const int kidmFinishedLoading = 13;
const int kidmError = 14;
const int kidmAnimationTimer = 15;
const int kidmDestroyAfterAnimation = 16;
const int kidmAppSetFocus = 17;
const int kidmAppKillFocus = 18;
const int kidmAskStringEvent = 19;
const int kidmKeyDown = 20;

extern base::MessageQueue gmq;

#endif // __INPUT_H__
