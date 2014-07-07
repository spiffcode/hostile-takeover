#ifndef __SOCKETSERVER_H__
#define __SOCKETSERVER_H__

#include "inc/basictypes.h"

namespace base {

const long64 kctForever = -1;

class Dispatcher;
class Socket;
class SocketNotify;

class SocketServer {
public:
    virtual ~SocketServer() {}
    virtual bool Wait(long64 ctWait, bool fProcessIO = true) = 0;
    virtual void WakeUp() = 0;
    virtual Dispatcher *CreateDispatcher() = 0;

    // Convenient helpers (not necessary, but nice to have)
    Socket *CreateSocket(int type, SocketNotify *notify);
    Socket *WrapSocket(int s, SocketNotify *notify);
    
    // Factory method
    static SocketServer *Create();
};

} // namespace base

#endif // __SOCKETSERVER_H__
