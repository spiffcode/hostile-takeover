#ifndef __EPOLLSERVER_H__
#define __EPOLLSERVER_H__

#include "inc/basictypes.h"
#include "base/dispatcher.h"
#include "base/eventer.h"
#include "base/socketserver.h"
#include <sys/epoll.h>
#include <map> // use std::unordered_map once standardized and stable

namespace base {

class EpollDispatcher;
class EpollDispatcher;
typedef std::map<dword, EpollDispatcher *> WaitMap;

// SocketServer that uses epoll() as the base primitive

class EpollServer : public SocketServer {
public:
    EpollServer();
    ~EpollServer();

    virtual bool Wait(long64 ctWait, bool fProcessIO = true);
    virtual void WakeUp();
    virtual Dispatcher *CreateDispatcher();

    void Add(int key, int descriptor, EpollDispatcher *dispatcher,
            uint32_t epoll_events);
    void Remove(int key, int descriptor);
    void Update(int key, int descriptor, uint32_t epoll_events);

protected:
    int GetEpollEventsList(epoll_event **epoll_event_list);

    int key_;
    WaitMap waitmap_;
    int ed_;
    bool wait_;
    int notifying_;
    Eventer *eventer_;
    epoll_event *epoll_events_;
    int epoll_events_count_;
};

// Dispatcher that is compatible with EpollServer

class EpollDispatcher : public Dispatcher {
public:
    EpollDispatcher(EpollServer *ss, int key) : Dispatcher(ss), key_(key) {}
    virtual ~EpollDispatcher() { SetDispatchee(NULL); }

    virtual void SetDispatchee(Dispatchee *dispatchee, int descriptor = -1);

protected:
    int key_;
};

} // namespace base

#endif // __EPOLLSERVER_H__
