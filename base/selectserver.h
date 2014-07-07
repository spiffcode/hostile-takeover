#ifndef __SELECTSERVER_H__
#define __SELECTSERVER_H__

#include "inc/basictypes.h"
#include "base/criticalsection.h"
#include "base/dispatcher.h"
#include "base/eventer.h"
#include "base/socketserver.h"

namespace base {

class SelectDispatcher;

// SocketServer that uses select() as the base primitive

class SelectServer : public SocketServer {
public:
    SelectServer();
    ~SelectServer();

    virtual bool Wait(long64 ctWait, bool fProcessIO = true);
    virtual void WakeUp();
    virtual Dispatcher *CreateDispatcher();

    void Add(SelectDispatcher *pdisp);
    void Remove(SelectDispatcher *pdisp);

protected:
    bool wait_;
    int notifying_;
    SelectDispatcher *pdispFirst_;
    Eventer *eventer_;
    CriticalSection crit_;
    SelectDispatcher *pdispNotifyNext_;
};

// Dispatcher that is compatible with SelectServer

class SelectDispatcher : public Dispatcher {
public:
    SelectDispatcher(SelectServer *ss) : Dispatcher(ss), pdispNext_(NULL) {}
    ~SelectDispatcher() { SetDispatchee(NULL); }

    virtual void SetDispatchee(Dispatchee *dispatchee, int descriptor = -1) {
        if (dispatchee_ != NULL) {
            ((SelectServer *)ss_)->Remove(this);
            descriptor_ = -1;
        }
        dispatchee_ = dispatchee;
        if (dispatchee_ != NULL) {
            descriptor_ = descriptor;
            ((SelectServer *)ss_)->Add(this);
        }
    }

protected:
    SelectDispatcher *pdispNext_;
    friend class SelectServer;
};

} // namespace base

#endif // __SELECTSERVER_H__
