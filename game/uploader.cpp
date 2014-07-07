#include "game/ht.h"
#include "game/uploader.h"
#include "base/thread.h"

namespace wi {

bool UploadByteBuffer(HttpService *service, base::ByteBuffer *bb,
        const char *url) {
    Uploader *uploader = new Uploader(service);
    if (uploader == NULL) {
        delete bb;
        return false;
    }
    if (!uploader->Upload(bb, url)) {
        delete uploader;
        return false;
    }
    delete uploader;
    return true;
}

Uploader::Uploader(HttpService *service) : service_(service), req_(NULL),
        done_(false), error_(false), ui_(NULL), cbLength_(0), cbTotal_(0) {
}

Uploader::~Uploader() {
    if (req_ != NULL) {
        service_->ReleaseRequest(req_);
        req_ = NULL;
    }
    delete ui_;
}

bool Uploader::Upload(base::ByteBuffer *bb, const char *url) {
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        delete bb;
        return false;
    }
    req_->SetURL(url);
    req_->SetBody(bb);
    req_->SetMethod("POST");
    service_->SubmitRequest(req_);

    SetMessage("Uploading game state...");

    // Sit in a modal loop until the upload either succeeds or fails

    while (!done_ && !error_) {
        Event evt;
        if (!gevm.GetEvent(&evt)) {
            continue;
        }
        if (ggame.FilterEvent(&evt)) {
            continue;
        }
        gevm.DispatchEvent(&evt);
    }

    service_->ReleaseRequest(req_);
    req_ = NULL;

    if (errorstr_.size() != 0) {
        HtMessageBox(kfMbWhiteBorder, "Upload Error", errorstr_.c_str());
    }

    return !error_;
}

void Uploader::SetMessage(const char *message) {
    delete ui_;
    message_ = message;
    ui_ = new TransportWaitingUI((char *)message_.c_str());
}

void Uploader::Wakeup() {
    // Wakes up the modal loop

    base::Thread::current().Post(base::kidmNullEvent, NULL);
}

void Uploader::OnReceivedResponse(HttpRequest *preq, int code,
        const Map *pheaders) {
    if (code >= 400) {
        errorstr_ = base::Format::ToString("error code %d", code);
        error_ = true;
        Wakeup();
        return;
    }
    if (code >= 200 && code < 300) {
        // Success! Get Content-Length and call back

        cbLength_ = -1;
        char szLength[32];
        if (pheaders->GetValue("Content-Length", szLength,
                sizeof(szLength))) {
            base::Format::ToInteger(szLength, 10, &cbLength_);
        }
        cbTotal_ = 0;
        SetMessage(base::Format::ToString("Uploaded %d of %d bytes...",
                cbTotal_, cbLength_));
        return;
    }
    // Ignore other status codes. If it's a redirect, OnReceivedResponse
    // will get called again.
    return;
}

void Uploader::OnReceivedData(HttpRequest *preq, const base::ByteBuffer *pbb) {
    if (error_) {
        return;
    }
}

void Uploader::OnFinishedLoading(HttpRequest *preq) {
    if (error_) {
        return;
    }
    done_ = true;
    Wakeup();
}

void Uploader::OnError(HttpRequest *preq, const char *pszError) {
    errorstr_ = pszError;
    error_ = true;
    Wakeup();
}

}
