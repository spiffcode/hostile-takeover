#include "base/thread.h"
#include "base/tick.h"

namespace base {

Thread g_main_thread;
pthread_key_t Thread::s_key_;

Thread::Thread(SocketServer *ss) : start_routine_(NULL), start_pv_(NULL),
        MessageQueue(ss) {
    if (&g_main_thread == this) {
        pthread_key_create(&s_key_, NULL);
        pthread_setspecific(s_key_, this);
    }
}

Thread::~Thread() {
    Stop();
}

void Thread::Start(void (start_routine)(void *), void *start_pv) {
    start_routine_ = start_routine;
    start_pv_ = start_pv;
    pthread_create(&thread_, NULL, PreRun, this);
}

void *Thread::PreRun(void *pv) {
    Thread *thread = (Thread *)pv;
    pthread_setspecific(s_key_, thread);
    if (thread->start_routine_ != NULL) {
        thread->start_routine_(thread->start_pv_);
    } else {
        thread->RunLoop();
    }
    return NULL;
}

void Thread::CallerStart(void *pv) {
    ICaller *caller = (ICaller *)pv;
    caller->Call();
    delete caller;
}

void Thread::Stop(bool wait) {
    MessageQueue::Stop();
    if (wait && &current() != this) {
        void *pv;
        pthread_join(thread_, &pv);
    }
}

void Thread::RunLoop(long64 ct) {
    Thread& thread = current();

    long64 tEnd;
    if (ct != kctForever) {
        tEnd = base::GetTickCount() + ct;
    }
    long64 ctNext = ct;

    while (true) {
        Message msg;
        if (!thread.Get(&msg, ctNext)) {
            return;
        }
        thread.Dispatch(&msg);

        if (ct != -1) {
            long64 tCur = base::GetTickCount();
            if (tCur >= tEnd) {
                return;
            }
            ctNext = tEnd - tCur;
        }
    }
}

} // namespace base
