#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include "base/dispatcher.h"
#include "inc/basictypes.h"

namespace base {

class SocketServer;
class SocketNotify;
class SocketAddress;

class Socket : Dispatchee {
public:
    Socket(SocketServer *ss);
    virtual ~Socket();
 
    virtual bool Attach(int s, SocketNotify *notify = NULL);
    virtual bool Create(int type, SocketNotify *notify = NULL);
    virtual int Bind(const SocketAddress& addr, bool reuse = true);
    virtual int Connect(const SocketAddress& addr);
    virtual int Send(const void *pv, size_t cb);
    virtual int SendTo(const void *pv, size_t cb, const SocketAddress& addr);
    virtual int Recv(void *pv, size_t cb);
    virtual int RecvFrom(void *pv, size_t cb, SocketAddress *paddr);
    virtual int Listen(int backlog = SOMAXCONN);
    virtual Socket *Accept(SocketAddress *paddr, SocketNotify *notify = NULL);
    virtual int Close();
    virtual int GetError() const;
    virtual void SetError(int error);
    virtual bool IsBlocking() const;
    virtual SocketAddress GetLocalAddress() const;
    virtual SocketAddress GetRemoteAddress() const;
    virtual void SetNotify(SocketNotify *notify);   
 
    enum ConnState {
        CS_CLOSED,
        CS_CONNECTING,
        CS_CONNECTED
    };
    virtual ConnState GetState() const;
    static const char *GetErrorString(int error);

protected:
    void UpdateLastError();

    // Dispatchee interface
    virtual void OnEvent(dword ff);
        
    int type_;
    int s_;
    int error_;
    ConnState state_;
    SocketNotify *notify_;
};

// Socket notification interface for async sockets.
class SocketNotify {
public:
    virtual void OnConnectEvent(Socket *socket) = 0;
    virtual void OnReadEvent(Socket *socket) = 0;
    virtual void OnWriteEvent(Socket *socket) = 0;
    virtual void OnCloseEvent(Socket *socket) = 0;
};

} // namespace base

#endif // __SOCKET_H__
