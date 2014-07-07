#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#include "base/criticalsection.h"
#include "base/socketserver.h"
#include "base/messagehandler.h"
#include "base/log.h"
#include "inc/basictypes.h"
#include <string.h>

namespace base {

class MessageData {
public:
    MessageData() {}
    virtual ~MessageData() {}
};

template<class T>
class DisposeData : public MessageData {
public:
    DisposeData(T *data) : data_(data) { }
    virtual ~DisposeData() { delete data_; }
    T *data_;
};

struct Message;

struct Message {
    Message() { memset(this, 0, sizeof(*this)); }
    int id;
    int x;
    int y;
    long ms;
    dword ff;
    long64 ctRate;
    long64 tTrigger;
    MessageData *data;
    MessageHandler *handler;
    Message *pmsgNext;
};
#define kfMsgCoalesce 0x1
#define kfMsgCancelMode 0x2
#define kfMsgTimer 0x4
const int kidmDispose = 0x7f00;

// New base messages are negative to avoid colliding with game idm's defined in input.h.
const int kidmNone = -1;
const int kidmNullEvent = -2;
const int kidmTransportEvent = -3;

class MessageQueue {
public:
    MessageQueue(SocketServer *ss = NULL);
    virtual ~MessageQueue();

    SocketServer& ss() { return *ss_; }
    void set_ss(SocketServer *ss) { delete ss_; ss_ = ss; }

    virtual void Stop();
    virtual bool IsStopping();
    virtual bool Get(Message *pmsg, long64 ctWait = kctForever);
    virtual void Post(Message *pmsg, int idCoalesce = -1);
    virtual void Post(int id, MessageHandler *handler,
            MessageData *data = NULL) {
        Message msg;
        msg.id = id;
        msg.handler = handler;
        msg.data = data;
        Post(&msg);
    }
    virtual void PostDelayed(Message *pmsg, long ct, long ctBoost = 0);
    virtual void PostDelayed(int id, MessageHandler *handler, long ct,
                long ctBoost = 0) {
        Message msg;
        msg.id = id;
        msg.handler = handler;
        PostDelayed(&msg, ct, ctBoost);
    }
    virtual void PostTimer(int id, MessageHandler *handler, long ct,
            long ctBoost = 0) {
        Message msg;
        msg.id = id;
        msg.handler = handler;
        msg.ff = kfMsgTimer;
        PostDelayed(&msg, ct, ctBoost);
    }
    virtual void BoostTimer(int id, MessageHandler *handler, long ctBoost);
    virtual void Clear(MessageHandler *handler = NULL, int id = kidmNone);
    virtual void ClearDispose(MessageHandler *handler = NULL);
    virtual void Dispatch(Message *pmsg);

    // Nice helper to async-delete objects
    template<class T> void Dispose(T *obj) {
        if (FindDispose(obj)) {
            LOG() << "error - disposing twice!";
            return;
        }
        if (obj != NULL) {
            Post(kidmDispose, NULL, new DisposeData<T>(obj));
        }
    }

private:
    bool FindDispose(void *pv);
    void ClearChain(Message **ppmsg, MessageHandler *handler, int id);
    void InsertDelayedMessage(Message *pmsg);

    SocketServer *ss_;
    bool stop_;
    Message *pmsg_;
    Message *pmsgDelayed_;
    CriticalSection crit_;
};

} // namespace base

#endif // __MESSAGEQUEUE_H__
