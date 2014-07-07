#include "server/httppost.h"
#include "mpshared/misc.h"

// A "simple" asynchronous http POST helper. Asynchronous except for the
// dns resolution of the address.
//
// Example protocol sniff using this class to Google app engine
//
// Client to server:
//
// POST /api/posttest HTTP/1.0\r\n
// Accept-Encoding: identity\r\n
// Content-Length: 11\r\n
// Host: 127.0.0.1:8080\r\n
// Content-Type: binary/octet-stream\r\n
// Connection: close\r\n
// User-Agent: httppost/1.0\r\n
// \r\n
// hello world
//
// Server to client:
//
// HTTP/1.0 200 OK\r\n
// Server: Development/1.0\r\n
// Date: Tue, 16 Jun 2009 22:53:17 GMT\r\n
// Cache-Control: no-cache\r\n
// Content-Type: binary/octet-stream\r\n
// Content-Length: 11\r\n
// \r\n
// hello world

namespace wi {

const int MSG_POSTCOMPLETE = 1;
const int MSG_TIMEOUT = 2;

HttpPost::HttpPost(base::SocketAddress& address, const std::string& path,
        base::ByteBuffer *body) : address_(address), s_(NULL),
        state_(HS_CLOSED), completed_(false), attempts_(0), written_(0) {
    WrapBody(path, body);
}

HttpPost::~HttpPost() {
    delete s_;
}

void HttpPost::SetState(State state) {
#if 0
    LOG() << base::Log::Format("0x%08lx ", this)
            << "From: " << HsLabels.Find(state_)
            << " To: " << HsLabels.Find(state);
#endif
    state_ = state;
}

void HttpPost::WrapBody(const std::string& path, base::ByteBuffer *body) {
    write_.WriteString(base::Format::ToString("POST %s HTTP/1.0\r\n",
            path.c_str()), false);
    write_.WriteString("Accept-Encoding: identity\r\n", false);
    write_.WriteString(base::Format::ToString("Content-Length: %d\r\n",
            body->Length()), false);
    write_.WriteString(base::Format::ToString("Host: %s\r\n",
            address_.ToString()), false);
    write_.WriteString("Content-Type: binary/octet-stream\r\n", false);
    write_.WriteString("Connection: close\r\n", false);
    write_.WriteString("User-Agent: httppost/1.0\r\n", false);
    write_.WriteString("\r\n", false);
    write_.WriteBytes(body->Data(), body->Length());
    delete body;
}

void HttpPost::Submit(int seconds_timeout) {
    if (!thread_.is_current()) {
        return;
    }
    if (state_ != HS_CLOSED) {
        return;
    }

    written_ = 0;
    completed_ = false;
    attempts_++;

    base::Socket *s = thread_.ss().CreateSocket(SOCK_STREAM, this); 
    if (s == NULL) {
        PostComplete(0, 0);
        return;
    }
    s_ = s;

    // Force dns resolution. If this this request has been around for awhile
    // we don't want to use stale ip
    address_.Resolve(true);
    
    if (s->Connect(address_) < 0) {
        if (!s->IsBlocking()) {
            LOG() << "Connect(" << address_.ToString() << ") failed error: " <<
                    base::Socket::GetErrorString(s->GetError());
            PostComplete(0, s->GetError());
            return;
        }
    }

    // Set timeout
    thread_.PostDelayed(MSG_TIMEOUT, this, seconds_timeout * 100);

    SetState(HS_CONNECTING);
}

const char *HttpPost::GetLine(const char *current, const char *end,
        char *line, int cb) {
    for (const char *pch = current; pch < end - 1; pch++) {
        if (pch[0] == '\r' && pch[1] == '\n') {
            int cbT = pch - current + 1;
            if (cbT > cb) {
                cbT = cb;
            }
            strncpyz(line, current, cbT);
            return pch + 2;
        }
    }
    return NULL;
}

bool HttpPost::FindStatusCode(int *status_code) {
    // Look for an empty line with \r\n - this marks the end of the headers
    const char *current = (const char *)read_.Data();
    const char *end = (const char *)read_.Data() + read_.Length();
    bool found = false;
    int cbBody = -1;
    char line[512];
    while ((current = GetLine(current, end, line, sizeof(line))) != NULL) {
        int cch = strlen(line);
        if (cch >= 15) {
            if (strncmp(line, "Content-Length:", 15) == 0) {
                base::Format::ToInteger(&line[15], 10, &cbBody);
            }
        }
        if (strlen(line) == 0) {
            found = true;
            break;
        }
    }

    // If not at end of headers, continue reading
    if (!found) {
        return false;
    }

    // If there is a Content-Length, see if that many body bytes have been
    // read yet. If not, continue reading.
    if (cbBody != -1) {
        if (((const char *)(read_.Data() + read_.Length()) - current) <
                cbBody) {
            return false;
        } else {
            if (result_.Length() == 0 && cbBody != 0) {
                result_.WriteBytes((const byte *)current, cbBody);
            }
        }
    }

    // Extract the status code
    if (GetLine((const char *)read_.Data(), end, line, sizeof(line)) == NULL) {
        return false;
    }

    // Find the status code
    end = &line[strlen(line)];
    for (current = line; current < end; current++) {
        if (*current == ' ' && *(current + 1) >= '0' && *(current + 1) <= '9') {
            current++;
            break;
        }
    }

    char szT[32];
    szT[0] = 0;
    for (int i = 0; i < sizeof(szT) - 1; i++) {
        if (current[i] >= '0' && current[i] <= '9') {
            szT[i] = current[i];
            szT[i + 1] = 0;
        }
    }

    *status_code = 0;
    base::Format::ToInteger(szT, 10, status_code);

    return strlen(szT) != 0;
}

void HttpPost::ProcessRead() {
    if (state_ != HS_SENT && state_ != HS_READ) {
        return;
    }
    SetState(HS_READ);

    while (true) {
        byte buff[4096];
        int cb = s_->Recv(buff, sizeof(buff));
        if (cb < 0) {
            if (s_->IsBlocking()) {
                break;
            }
            PostComplete(0, s_->GetError());
            return;
        }
        read_.WriteBytes(buff, cb);
    }

    // Check for status code. If present, close the connection.
    int status_code;
    if (FindStatusCode(&status_code)) {
        PostComplete(status_code, 0);
    }
}

void HttpPost::ProcessWrite() {
    if (state_ != HS_CONNECTED && state_ != HS_WRITE) {
        return;
    }
    SetState(HS_WRITE);

    while (written_ < write_.Length()) {
        int remaining = write_.Length() - written_;
        int cb = s_->Send(write_.Data() + written_, remaining);
        if (cb < 0) {
            if (s_->IsBlocking()) {
                return;
            }
            PostComplete(0, s_->GetError());
            return;
        }
        written_ += cb;
    }

    SetState(HS_SENT);
    ProcessRead();
}

void HttpPost::OnConnectEvent(base::Socket *socket) {
    SetState(HS_CONNECTED);
    ProcessWrite();
}

void HttpPost::OnReadEvent(base::Socket *socket) {
    ProcessRead();
}

void HttpPost::OnWriteEvent(base::Socket *socket) {
    ProcessWrite();
}

void HttpPost::OnCloseEvent(base::Socket *socket) {
    PostComplete(0, 0);
}

void HttpPost::PostComplete(int status_code, int error) {
    base::Message msg;
    msg.id = MSG_POSTCOMPLETE;
    msg.handler = this;
    msg.data = new CompleteParams(status_code, error);
    thread_.Post(&msg);
}

void HttpPost::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case MSG_POSTCOMPLETE:
        thread_.Clear(this, MSG_TIMEOUT);
        if (!completed_) {
            completed_ = true;
            delete s_;
            s_ = NULL;
            SetState(HS_CLOSED);
            CompleteParams *params = (CompleteParams *)pmsg->data;
            if (params->status_code != 200) {
                RLOG() << "post: " << (dword)this << " status_code: "
                        << params->status_code << " error: " << params->error;
            }
            SignalOnComplete(this, params->status_code, params->error, result_);
            delete params;
        }
        break;

    case MSG_TIMEOUT:
        PostComplete(0, 0);
        break;
    }
}

} // namespace wi

