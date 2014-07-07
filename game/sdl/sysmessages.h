#ifndef __SYSMESSAGES_H__
#define __SYSMESSAGES_H__

namespace wi {

enum {
    kidmAppTerminate = 1,
    kidmBreakEvent,
    kidmNullEvent,
    kidmTransportEvent,
    kidmAnimationTimer,
    kidmDestroyAfterAnimation,
    kidmAppSetFocus,
    kidmAppKillFocus,
    kidmAskStringEvent,
    kidmSdlEvent,
    kidmUser = 0x400
};

} // namespace wi

#endif // __SYSMESSAGES_H__
