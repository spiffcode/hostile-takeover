#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>
#include "inc/basictypes.h"
#include "base/messagequeue.h"

namespace base {

class ICaller {
public:
	virtual ~ICaller() {}
    virtual void Call() = 0;
};

template<class T>
class Caller : public ICaller {
public:
    Caller(T *obj, void (T::*method)(void *), void *pv) : obj_(obj),
            method_(method), pv_(pv) {}
    void Call() { (obj_->*method_)(pv_); }
private:
    T *obj_;
    void (T::*method_)(void *);
    void *pv_;
};

class Thread : public MessageQueue {
public:
    Thread(SocketServer *ss = NULL);
    virtual ~Thread();

    static Thread& current() { return *(Thread *)pthread_getspecific(s_key_); }
    bool is_current() const { return &current() == this; }

    template<class T>
    void Start(T *obj, void (T::*method)(void *), void *pv = NULL) {
        Start(CallerStart, new Caller<T>(obj, method, pv));
    }
    virtual void Start(void (*start_routine)(void *) = NULL,
            void *start_pv = NULL);
    virtual void Stop(bool wait = true);
    static void RunLoop(long64 ct = -1);

private:
    static void *PreRun(void *pv);
    static void CallerStart(void *pv);

    static pthread_key_t s_key_;
    pthread_t thread_;
    void (*start_routine_)(void *);
    void *start_pv_;
};

} // namespace base

#endif // __THREAD_H__

