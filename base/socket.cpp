#include "base/socket.h"
#include "base/socketserver.h"
#include "base/socketaddress.h"
#include "base/misc.h"
#include <unistd.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <stdio.h>

namespace base {

Socket::Socket(SocketServer *ss) : Dispatchee(ss), s_(-1), notify_(NULL),
        error_(0), state_(CS_CLOSED), type_(0) {
}

Socket::~Socket() {
    Close();
}

bool Socket::Attach(int s, SocketNotify *notify) {
    Close();
    s_ = s;
    if (s_ == -1) {
        return false;
    }
    state_ = CS_CONNECTED;        
    SetNotify(notify);
    return true;
}

bool Socket::Create(int type, SocketNotify *notify) {
    Close();
    s_ = ::socket(AF_INET, type, 0);
    UpdateLastError();
    SetNotify(notify);

    // Turn off nagle
    int value = 1;
    setsockopt(s_, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));
    type_ = type;

    // Return EPIPE instead of raising SIGPIPE
#ifdef SO_NOSIGPIPE
    value = 1;
    setsockopt(s_, SOL_SOCKET, SO_NOSIGPIPE, (void *)&value, sizeof(value));
#endif
    
    return s_ != -1;
}

SocketAddress Socket::GetLocalAddress() const {
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getsockname(s_, (sockaddr*)&addr, &addrlen);
    SocketAddress address;
    if (result >= 0) {
        address.FromSockAddr(addr);
    }
    return address;
}

SocketAddress Socket::GetRemoteAddress() const {
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int result = ::getpeername(s_, (sockaddr*)&addr, &addrlen);
    SocketAddress address;
    if (result >= 0) {
        address.FromSockAddr(addr);
    }
    return address;
}

int Socket::Bind(const SocketAddress& addr, bool reuse) {
    if (reuse) {
        int n = 1;
        int err = setsockopt(s_, SOL_SOCKET, SO_REUSEADDR, (void *)&n,
                sizeof(n));
        if (err < 0) {
            UpdateLastError();
            return err;
        }
    }

    sockaddr_in saddr;
    addr.ToSockAddr(&saddr);
    int err = ::bind(s_, (sockaddr*)&saddr, sizeof(saddr));
    UpdateLastError();
    return err;
}

int Socket::Connect(const SocketAddress& addr) {
    if (s_ == -1 && !Create(SOCK_STREAM)) {
        return -1;
    }
    SocketAddress addr2(addr);
    if (addr2.IsUnresolved()) {
        addr2.Resolve();
    }
    sockaddr_in saddr;
    addr2.ToSockAddr(&saddr);
    int err = ::connect(s_, (sockaddr*)&saddr, sizeof(saddr));
    UpdateLastError();
    if (err == 0) {
        state_ = CS_CONNECTED;
    } else if (IsBlocking()) {
        state_ = CS_CONNECTING;
        dispatcher_->SetEvents(Dispatcher::kfConnect);
    }
    dispatcher_->SetEvents(Dispatcher::kfRead | Dispatcher::kfWrite);
    return err;
}

int Socket::GetError() const {
    return error_;
}

void Socket::SetError(int error) {
    error_ = error;
}

Socket::ConnState Socket::GetState() const {
    return state_;
}

int Socket::Send(const void *pv, size_t cb) {
    int sent = ::send(s_, reinterpret_cast<const char *>(pv), (int)cb, 0);
    UpdateLastError();
    if ((sent < 0) && IsBlocking()) {
        dispatcher_->SetEvents(Dispatcher::kfWrite);
    }
    return sent;
}

int Socket::SendTo(const void *pv, size_t cb, const SocketAddress& addr) {
    sockaddr_in saddr;
    addr.ToSockAddr(&saddr);
    int sent = ::sendto(
            s_, (const char *)pv, (int)cb, 0, (sockaddr*)&saddr,
            sizeof(saddr));
    UpdateLastError();
    if ((sent < 0) && IsBlocking()) {
        dispatcher_->SetEvents(Dispatcher::kfWrite);
    }
    return sent;
}

int Socket::Recv(void *pv, size_t cb) {
    int received = ::recv(s_, (char *)pv, (int)cb, 0);
    if ((received == 0) && (cb != 0)) {
        dispatcher_->SetEvents(Dispatcher::kfRead | Dispatcher::kfRemoteClose);
        error_ = EWOULDBLOCK;
        return -1;
    }
    UpdateLastError();
    if ((received >= 0) || IsBlocking()) {
        dispatcher_->SetEvents(Dispatcher::kfRead);
    }
    return received;
}

int Socket::RecvFrom(void *pv, size_t cb, SocketAddress *paddr) {
    sockaddr_in saddr;
    socklen_t cbAddr = sizeof(saddr);
    int received = ::recvfrom(s_, (char *)pv, (int)cb, 0, (sockaddr*)&saddr,
            &cbAddr);
    UpdateLastError();
    if (received >= 0 && paddr != NULL) {
        paddr->FromSockAddr(saddr);
    }
    if (received >= 0 || IsBlocking()) {
        dispatcher_->SetEvents(Dispatcher::kfRead);
    }
    return received;
}

int Socket::Listen(int backlog) {
    int err = ::listen(s_, backlog);
    UpdateLastError();
    if (err == 0)
        state_ = CS_CONNECTING;
    dispatcher_->SetEvents(Dispatcher::kfRead);
    return err;
}

Socket *Socket::Accept(SocketAddress *paddr, SocketNotify *notify) {
    sockaddr_in saddr;
    socklen_t cbAddr = sizeof(saddr);
    int s = ::accept(s_, (sockaddr*)&saddr, &cbAddr);
    UpdateLastError();
    if (s == -1) {
        return NULL;
    }
    if (paddr != NULL) {
        paddr->FromSockAddr(saddr);
    }
    dispatcher_->SetEvents(Dispatcher::kfRead | Dispatcher::kfWrite);
    return dispatcher_->ss()->WrapSocket(s, notify);
}

int Socket::Close() {
    if (s_ == -1) {
        return 0;
    }
    int err = close(s_);
    UpdateLastError();
    s_ = -1;
    state_ = CS_CLOSED;
    SetNotify(NULL);
    return err;
}

void Socket::UpdateLastError() {
    error_ = errno;
}
    
bool Socket::IsBlocking() const {
    return error_ == EWOULDBLOCK || error_ == EAGAIN || error_ == EINPROGRESS;        
}

void Socket::OnEvent(dword ff) {
    if (ff & Dispatcher::kfConnect) {
        state_ = CS_CONNECTED;
    }

    if (ff & Dispatcher::kfRead) {
        dispatcher_->ClearEvents(Dispatcher::kfRead);
        if (notify_ != NULL) {
            DeleteRecord dr(dispatcher_);
            notify_->OnReadEvent(this);
            if (dr.deleted()) {
                return;
            }
        }
    }
    if (ff & Dispatcher::kfWrite) {
        dispatcher_->ClearEvents(Dispatcher::kfWrite);
        if (notify_ != NULL) {
            DeleteRecord dr(dispatcher_);
            notify_->OnWriteEvent(this);
            if (dr.deleted()) {
                return;
            }
        }
    }
    if (ff & Dispatcher::kfConnect) {
        dispatcher_->ClearEvents(Dispatcher::kfConnect);
        if (notify_ != NULL) {
            DeleteRecord dr(dispatcher_);
            notify_->OnConnectEvent(this);
            if (dr.deleted()) {
                return;
            }
        }
    }
    if (ff & Dispatcher::kfClose) {
        state_ = CS_CLOSED;
        if (notify_ != NULL) {
            DeleteRecord dr(dispatcher_);
            notify_->OnCloseEvent(this);
            if (dr.deleted()) {
                return;
            }
        }
    }
}

void Socket::SetNotify(SocketNotify *notify) {
    if (notify_ != NULL) {
        dispatcher_->SetDispatchee(NULL);
        dispatcher_->ClearEvents();
        fcntl(s_, F_SETFL, fcntl(s_, F_GETFL, 0) & ~O_NONBLOCK);
    }
    notify_ = notify;
    if (notify_ != NULL) {
        fcntl(s_, F_SETFL, fcntl(s_, F_GETFL, 0) | O_NONBLOCK);
        dispatcher_->SetDispatchee(this, s_);
        if (state_ == CS_CONNECTED || type_ != SOCK_STREAM) {
            dispatcher_->SetEvents(Dispatcher::kfRead | Dispatcher::kfWrite);
        }
    }
}

STARTLABEL(SocErrors)
    LABEL(EINTR)
    LABEL(EBADF)
    LABEL(EACCES)
    LABEL(EFAULT)
    LABEL(EINVAL)
    LABEL(EMFILE)
    LABEL(EWOULDBLOCK)
    LABEL(EINPROGRESS)
    LABEL(EALREADY)
    LABEL(ENOTSOCK)
    LABEL(EDESTADDRREQ)
    LABEL(EMSGSIZE)
    LABEL(EPROTOTYPE)
    LABEL(ENOPROTOOPT)
    LABEL(EPROTONOSUPPORT)
    LABEL(ESOCKTNOSUPPORT)
    LABEL(EOPNOTSUPP)
    LABEL(EPFNOSUPPORT)
    LABEL(EAFNOSUPPORT)
    LABEL(EADDRINUSE)
    LABEL(EADDRNOTAVAIL)
    LABEL(ENETDOWN)
    LABEL(ENETUNREACH)
    LABEL(ENETRESET)
    LABEL(ECONNABORTED)
    LABEL(ECONNRESET)
    LABEL(ENOBUFS)
    LABEL(EISCONN)
    LABEL(ENOTCONN)
    LABEL(ESHUTDOWN)
    LABEL(ETOOMANYREFS)
    LABEL(ETIMEDOUT)
    LABEL(ECONNREFUSED)
    LABEL(ELOOP)
    LABEL(ENAMETOOLONG)
    LABEL(EHOSTDOWN)
    LABEL(EHOSTUNREACH)
    LABEL(ENOTEMPTY)
    LABEL(EUSERS)
    LABEL(EDQUOT)
    LABEL(ESTALE)
    LABEL(EREMOTE)
ENDLABEL(SocErrors)

const char *Socket::GetErrorString(int error) {
    return SocErrors.Find(error);
}

} // namespace base
