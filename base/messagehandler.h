#ifndef __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__

#include "inc/basictypes.h"

namespace base {

class Message;
class Thread;

class MessageHandler {
public:
    MessageHandler();
    MessageHandler(Thread& thread);
    virtual ~MessageHandler();
    virtual void OnMessage(Message *pmsg) {}
    Thread& thread() { return thread_; }
    void Dispose();

protected:
    Thread& thread_;
    bool disposed_;
};

} // namespace base

#endif // __MESSAGEHANDLER_H__
