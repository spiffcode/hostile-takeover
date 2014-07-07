#include "game/simplerequest.h"
#include "game/ht.h"

namespace wi {

SimpleRequest::SimpleRequest(HttpService *service) : service_(service),
        req_(NULL), code_(0), error_(false), errorstr_(NULL),
        errorstrsize_(0), done_(false), timeout_(-1) {
    if (service_ == NULL) {
        service_ = gphttp;
    }
}

SimpleRequest::~SimpleRequest() {
    Reset();
}

void SimpleRequest::Reset() {
    if (req_ != NULL && service_ != NULL) {
        service_->ReleaseRequest(req_);
        req_ = NULL;
    }
    bb_.Rewind();
    errorstr_ = NULL;
    errorstrsize_ = 0;
}

bool SimpleRequest::Get(const char *url, char *result, int resultsize,
        char *errorstr, int errorstrsize) {
    // Stop anything in progress (there shouldn't be anything)
    Reset();
    errorstr_ = errorstr;
    errorstrsize_ = errorstrsize;
    if (errorstr_ != NULL && errorstrsize_ > 0) {
        *errorstr_ = 0;
    }

    // Make a new request and submit it
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        return false;
    }
    req_->SetURL(url);
    if (timeout_ > 0) {
        req_->SetTimeout(timeout_);
    }
    service_->SubmitRequest(req_);

    // Sit in a loop waiting for the response. Note the http service
    // has a timeout, so no timeout required here.
    done_ = false;
    gtimm.Enable(false);
    while (!done_ && !error_) {
        Event evt;
        if (gevm.GetEvent(&evt, 50, false)) {
            if (gevm.IsAppStopping()) {
                break;
            }
            ggame.FilterEvent(&evt);
        }
    }
    gtimm.Enable(true);

    // Return result if there is one
    if (!error_) {
        if (result != NULL) {
            int cb = _min(bb_.Length(), resultsize - 1);
            memcpy(result, bb_.Data(), cb);
            result[cb] = 0;
        }
        Reset();
        return true;
    }
    Reset();
    return false;
}

void SimpleRequest::OnReceivedResponse(HttpRequest *preq, int code,
        const Map *pheaders) {
    code_ = code;
    if (code >= 400) {
        // Http error
        error_ = true;
        if (errorstr_ != NULL) {
            strncpyz(errorstr_, base::Format::ToString(
                    "Server returned error %d", code), errorstrsize_);
        }
    }
}

void SimpleRequest::OnReceivedData(HttpRequest *preq,
        const base::ByteBuffer *pbb) {
    if (error_) {
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        bb_.WriteBytes(pbb->Data(), pbb->Length());
    }
}

void SimpleRequest::OnFinishedLoading(HttpRequest *preq) {
    if (error_) {
        return;
    }
    done_ = true;
}    

void SimpleRequest::OnError(HttpRequest *preq, const char *pszError) {
    if (errorstr_ != NULL) {
        strncpyz(errorstr_, pszError, errorstrsize_);
    }
    error_ = true;
}

} // namespace wi
