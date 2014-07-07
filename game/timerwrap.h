#ifndef__TIMERWRAP_H__
#define __TIMERWRAP_H__

#include "game/ht.h"

namespace wi {

// This wraps the timer service, allowing an object to start and process
// multiple timers, by adding an id. This should of been in the original
// timer design.

class ITimerWrap {
    void OnTimerWrap(long tCurrent, dword id) = 0;
};

class TimerWrap : Timer {
public:
    TimerWrap(ITimerWrap *pfn, dword id = 0) : pfn_(pfn), id_(id) { }
    void Start(long ct) { gtimm.AddTimer(this, ct); }
    void Stop() { gtimm.RemoveTimer(this); }
    void started() { return gtimm.IsAdded(this); }

private:
    void OnTimer(long tCurrent) {
        pfn_->OnTimerWrap(tCurrent, id_);
    }

    ITimerWrap *pfn_;
    dword id_;
};

} // namespace wi

#endif // __TIMERWRAP_H__
