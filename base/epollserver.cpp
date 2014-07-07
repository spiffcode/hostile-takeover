#include "base/epollserver.h"

#include "base/log.h"
#include "base/tick.h"
#include "base/socket.h"
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

namespace base {

EpollServer::EpollServer() : notifying_(0), wait_(false), key_(0),
        epoll_events_(NULL), epoll_events_count_(0) {
    // The reserved socket size dynamically grows, so just use size 1
    ed_ = ::epoll_create(1);
    eventer_ = new Eventer(this, &wait_);
}

EpollServer::~EpollServer() {
    delete eventer_;
    ::close(ed_);
#ifdef DEBUG
    if (waitmap_.size() > 0) {
        LOG() << "EpollServer deleting, with waiting dispatchers!";
    }
#endif
    delete epoll_events_;
}

void EpollServer::WakeUp() {
    eventer_->Signal();
}

int EpollServer::GetEpollEventsList(epoll_event **epoll_event_list) {
    // If the current list is big enough, use it
    if (waitmap_.size() > epoll_events_count_) {
        // Free it and make a bigger one
        delete epoll_events_;
        epoll_events_ = NULL;
        epoll_events_count_ = 0;

        // Should probably check for error here but this is unlikely to be
        // a problem        
        int new_count = waitmap_.size() + waitmap_.size() / 2;
        epoll_events_ = new epoll_event[new_count];
        epoll_events_count_ = new_count;
    }

    *epoll_event_list = epoll_events_;
    return epoll_events_count_;
}

bool EpollServer::Wait(long64 ctWait, bool fProcessIO) {
    if (notifying_ != 0) {
        LOG() << "Error - re-entering!";
        return false;
    }

    // Calculate timing information
    long64 cmsNext = -1;
    long64 msStop;
    if (ctWait != kctForever) {
        cmsNext = ctWait * 10;
        msStop = base::GetMillisecondCount() + cmsNext;
    }

    // Wait for and dispatch events
    wait_ = true;
    while (wait_) {
        epoll_event *epoll_events;
        int count = GetEpollEventsList(&epoll_events);
        count = epoll_wait(ed_, epoll_events, count, (int)cmsNext);

        // Check for error
        if (count < 0) {
            LOG() << "epoll_wait returns < 0, errno: " << errno << ", "
                    << Socket::GetErrorString(errno);
            return false;
        }

        // If timeout, success
        if (count == 0) {
            return true;
        }

        // Enumerate and dispatch events
        notifying_++;
        for (int i = 0; i < count; i++) {
            // Get the dispatcher. Check to see if the dispatcher has been
            // removed. This lookup must be fast.
            epoll_event& e = epoll_events[i];
            WaitMap::iterator it = waitmap_.find(e.data.fd);
            if (it == waitmap_.end()) {
                continue;
            }
            EpollDispatcher *dispatcher = it->second;

            // Notify of the event(s)
            dword ff = 0;
            if (e.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                ff |= Dispatcher::kfClose;
            }
            if (e.events & EPOLLIN) {
                if (dispatcher->GetEvents() & Dispatcher::kfRemoteClose) {
                    ff |= Dispatcher::kfClose;
                } else {
                    ff |= Dispatcher::kfRead;
                }
            }
            if (e.events & EPOLLOUT) {
                if (dispatcher->GetEvents() & Dispatcher::kfConnect) {
                    ff |= Dispatcher::kfConnect;
                } else {
                    ff |= Dispatcher::kfWrite;
                }
            }
            if (ff != 0) {
                dispatcher->OnEvent(ff);
            }
        }
        notifying_--;

        // Recalc the time remaining.
        if (ctWait != kctForever) {
            long64 msCurrent = base::GetMillisecondCount();
            if (msCurrent >= msStop) {
                return true;
            }
            cmsNext = msStop - msCurrent;
        }
    }
    return true;
}

Dispatcher *EpollServer::CreateDispatcher() {
    return new EpollDispatcher(this, key_++);
}

void EpollServer::Add(int key, int descriptor, EpollDispatcher *disp,
        uint32_t epoll_events) {
#ifdef DEBUG
    WaitMap::iterator it = waitmap_.find(key);
    if (it != waitmap_.end()) {
        LOG() << key << " ALREADY ADDED!";
    }
#endif
    waitmap_.insert(WaitMap::value_type(key, disp));
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = epoll_events | EPOLLET | EPOLLRDHUP;
    e.data.fd = key;
    epoll_ctl(ed_, EPOLL_CTL_ADD, descriptor, &e);
}

void EpollServer::Remove(int key, int descriptor) {
    WaitMap::iterator it = waitmap_.find(key);
    if (it == waitmap_.end()) {
        LOG() << key << " NOT FOUND!";
        return;
    }
    waitmap_.erase(it);
    epoll_event e; // ignored but must be passed on kernels < 2.6.9
    memset(&e, 0, sizeof(e));
    epoll_ctl(ed_, EPOLL_CTL_DEL, descriptor, &e);
}

void EpollServer::Update(int key, int descriptor, uint32_t epoll_events) {
    epoll_event e;
    memset(&e, 0, sizeof(e));
    e.events = epoll_events | EPOLLET | EPOLLRDHUP;
    e.data.fd = key;
    epoll_ctl(ed_, EPOLL_CTL_MOD, descriptor, &e);
}

// Static factory method for EpollServer
SocketServer *SocketServer::Create() {
    return new EpollServer;
}

// EpollDispatcher methods

void EpollDispatcher::SetDispatchee(Dispatchee *dispatchee, int descriptor) {
    // If no dispatchee currently, and now setting one
    if (dispatchee_ == NULL && dispatchee != NULL) {
        ((EpollServer *)ss_)->Add(key_, descriptor, this, EPOLLIN | EPOLLOUT);
    }

    // If have dispatchee, and now clearing it
    if (dispatchee_ != NULL && dispatchee == NULL) {
        ((EpollServer *)ss_)->Remove(key_, descriptor_);
    }

    // Switching dispatchees likely won't happen, but cover the cases...
    if (dispatchee_ != NULL && dispatchee != NULL) {
        ((EpollServer *)ss_)->Remove(key_, descriptor_);
        ((EpollServer *)ss_)->Add(key_, descriptor, this, EPOLLIN | EPOLLOUT);
    }

    descriptor_ = descriptor;
    dispatchee_ = dispatchee;
}

} // namespace base
