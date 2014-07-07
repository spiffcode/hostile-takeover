#include "eventer.h"
#include <unistd.h>
#include <signal.h>

namespace base {

Eventer::Eventer(SocketServer *ss, bool *wait) : Dispatchee(ss), wait_(wait),
        signaled_(false) {
    pipe(afd_);
    signal(SIGPIPE, SIG_IGN);
    dispatcher_->SetDispatchee(this, afd_[0]);
    dispatcher_->SetEvents(Dispatcher::kfRead);
}

Eventer::~Eventer()
{
    close(afd_[0]);
    close(afd_[1]);
}

void Eventer::Signal()
{
    CritScope cs(&crit_);
    if (!signaled_) {
        byte b = 0;
        write(afd_[1], &b, sizeof(b));
        signaled_ = true;
    }
}

void Eventer::OnEvent(dword ff)
{
    CritScope cs(&crit_);
    if (signaled_) {
        byte b;
        read(afd_[0], &b, sizeof(b));
        signaled_ = false;
    }
    if (wait_ != NULL) {
        *wait_ = false;
    }
}

} // namespace base 
