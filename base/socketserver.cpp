#include "base/socketserver.h"
#include "base/socket.h"

namespace base {

// Convenient helpers (not necessary, but nice to have)

Socket *SocketServer::CreateSocket(int type, SocketNotify *notify) {
    Socket *socket = new Socket(this);
    if (socket->Create(type, notify)) {
        return socket;
    }
    delete socket;
    return NULL;
}

Socket *SocketServer::WrapSocket(int s, SocketNotify *notify) {
    Socket *socket = new Socket(this);
    if (socket->Attach(s, notify)) {
        return socket;
    }
    delete socket;
    return NULL;
}

// Can't put this in the include file because of class definition ordering issues

void Dispatcher::OnEvent(dword ff) {
    if (dispatchee_ != NULL) {
        dispatchee_->OnEvent(ff);
    }
}


} // namespace base
