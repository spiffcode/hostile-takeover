#ifndef __HTTPPOST_H__
#define __HTTPPOST_H__

#include "base/sigslot.h"
#include "inc/basictypes.h"
#include "base/bytebuffer.h"
#include "base/messagehandler.h"
#include "base/thread.h"
#include "base/misc.h"
#include "base/socket.h"
#include "base/socketaddress.h"
#include "base/bytebuffer.h"
#include <string>

namespace wi {

struct CompleteParams : base::MessageData {
    CompleteParams(int status_code, int error) : status_code(status_code),
            error(error) { }
    int status_code;
    int error;
};

class HttpPost : base::MessageHandler, base::SocketNotify {
public:
    HttpPost(base::SocketAddress& address, const std::string& path,
            base::ByteBuffer *body);
    ~HttpPost();

    void Submit(int seconds_timeout = 60);
    int attempts() { return attempts_; }

    // this, status code, error code
    base::signal4<HttpPost *, int, int,
            const base::ByteBuffer&> SignalOnComplete;

    enum State {
        HS_CLOSED, HS_CONNECTING, HS_CONNECTED, HS_WRITE, HS_SENT, HS_READ
    };

private:
    void SetState(State state);
    void WrapBody(const std::string& path, base::ByteBuffer *body);
    bool FindStatusCode(int *status_code);
    void ProcessRead();
    void ProcessWrite();
    void PostComplete(int status_code, int error);
    const char *GetLine(const char *current, const char *end, char *line,
            int cb);

    // SocketNotify
    virtual void OnConnectEvent(base::Socket *socket);
    virtual void OnReadEvent(base::Socket *socket);
    virtual void OnWriteEvent(base::Socket *socket);
    virtual void OnCloseEvent(base::Socket *socket);

    // MessageHandler interface
    virtual void OnMessage(base::Message *pmsg);

    State state_;
    base::Socket *s_;
    base::SocketAddress address_;
    base::ByteBuffer write_;
    base::ByteBuffer read_;
    base::ByteBuffer result_;
    bool completed_;
    int attempts_;
    int written_;
};

STARTLABEL(HsLabels)
    LABEL(HttpPost::HS_CLOSED)
    LABEL(HttpPost::HS_CONNECTING)
    LABEL(HttpPost::HS_CONNECTED)
    LABEL(HttpPost::HS_WRITE)
    LABEL(HttpPost::HS_SENT)
    LABEL(HttpPost::HS_READ)
ENDLABEL(HsLabels)

} // namespace wi

#endif // __HTTPPOST_H__

