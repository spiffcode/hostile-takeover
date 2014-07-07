#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "inc/basictypes.h"
#include "base/deletetracker.h"
#include "base/socketserver.h"

namespace base {

class Dispatchee;

class Dispatcher : public DeleteTracker {
public:
    Dispatcher(SocketServer *ss) : ss_(ss), dispatchee_(NULL), descriptor_(-1),
            events_(0) {}
    virtual ~Dispatcher() {}

    virtual void SetEvents(dword events) { events_ |= events; }
    virtual void ClearEvents(dword events = (dword)-1) { events_ &= ~events; }
    virtual dword GetEvents() { return events_; }
    virtual void OnEvent(dword ff);
    virtual void SetDispatchee(Dispatchee *dispatchee, int descriptor = -1) {}

    int descriptor() { return descriptor_; }
    SocketServer *ss() { return ss_; }

    // TODO: Some of these event flags have Socket semantics in them, that should be
    // owned by socket, not here.
    enum EventFlags {
        kfRead = 0x1, kfWrite = 0x2, kfConnect = 0x4, kfClose = 0x8,
        kfRemoteClose = 0x10
    };

protected:
    dword events_;
    int descriptor_;
    SocketServer *ss_;
    Dispatchee *dispatchee_;
};

class Dispatchee {
public:
    Dispatchee(SocketServer *ss) : dispatcher_(ss->CreateDispatcher()) {}
    virtual ~Dispatchee() { delete dispatcher_; }

    virtual void OnEvent(dword ff) = 0;

protected:
    Dispatcher *dispatcher_;
};


} // namespace base

#endif // __DISPATCHER_H__
