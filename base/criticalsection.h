#ifndef __CRITICALSECTION_H__
#define __CRITICALSECTION_H__

#include "inc/basictypes.h"
#include <pthread.h>

namespace base {

class CriticalSection {
public:
    CriticalSection() {
        pthread_mutexattr_t mutex_attribute;
        pthread_mutexattr_init(&mutex_attribute);
        pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mutex_, &mutex_attribute);
    }
    ~CriticalSection() {
        pthread_mutex_destroy(&mutex_);
    }
    void Enter() {
        pthread_mutex_lock(&mutex_);
    }
    void Leave() {
        pthread_mutex_unlock(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
};

// CritScope, for serializing exection through a scope

class CritScope {
public:
    CritScope(CriticalSection *pcrit) {
        pcrit_ = pcrit;
        pcrit_->Enter();
    }
    ~CritScope() {
        pcrit_->Leave();
    }
private:
    CriticalSection *pcrit_;
};

} // namespace base

#endif // __CRITICALSECTION_H__
