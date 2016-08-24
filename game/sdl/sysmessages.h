#ifndef __SYSMESSAGES_H__
#define __SYSMESSAGES_H__

namespace wi {

enum {
    kidmAppTerminate = 1,
    kidmBreakEvent,
    kidmAnimationTimer,
    kidmDestroyAfterAnimation,
    kidmAppSetFocus,
    kidmAppKillFocus,
    kidmAskStringEvent,
    kidmSdlEvent,
    kidmReceivedResponse,
    kidmReceivedData,
    kidmFinishedLoading,
    kidmError,
    kidmUser = 0x400
};

} // namespace wi

#endif // __SYSMESSAGES_H__
