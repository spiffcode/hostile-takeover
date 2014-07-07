#ifndef __EVENTER_H__
#define __EVENTER_H__

#include "inc/basictypes.h"
#include "base/dispatcher.h"
#include "base/socketserver.h"
#include "base/criticalsection.h"

namespace base {

class SocketServer;

class Eventer : Dispatchee {
public:
    Eventer(SocketServer *ss, bool *wait);
    virtual ~Eventer();

    virtual void Signal();
    Dispatcher *dispatcher() { return dispatcher_; }

private:
    // Dispatchee interface
    virtual void OnEvent(dword ff);

    bool *wait_;
    int afd_[2];
    bool signaled_;
    CriticalSection crit_;
};

} // namespace base

#endif // __EVENTER_H__
