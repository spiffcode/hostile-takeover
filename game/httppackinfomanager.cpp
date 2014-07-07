#include "game/httppackinfomanager.h"
#include "game/serviceurls.h"
#include "base/format.h"
#include <unistd.h>

namespace wi {

HttpPackInfoManager::HttpPackInfoManager(HttpService *service,
        const char *cachedir, const char *tempdir) : PackInfoManager(cachedir),
        service_(service) {
    tempdir_ = tempdir;
    req_ = NULL;
    callback_ = NULL;
    ctx_ = NULL;
    tempfile_ = NULL;
    Reset();
}

HttpPackInfoManager::~HttpPackInfoManager() {
    Reset();
}

bool HttpPackInfoManager::Start(const PackId *ppackid, void *ctx,
        ProgressCallback *callback) {
    if (service_ == NULL) {
        return false;
    }

    // Initialize
    Reset();
    temppackid_ = *ppackid;
    ctx_ = ctx;
    callback_ = callback;

    // Create a temp filename for the temp file
    char szTemp[256];
    strncpyz(szTemp,
            base::Format::ToString("%s/packinfodl.XXXXX", tempdir_.c_str()),
            sizeof(szTemp));
    if (mktemp(szTemp) == NULL) {
        return false;
    }
    tempfilename_ = szTemp;

    // Open the temp file that will receive the bytes
    tempfile_ = fopen(tempfilename_.c_str(), "w+b");
    if (tempfile_ == NULL) {
        return false;
    }

    // Build the request
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        return false;
    }
    SetServiceUrl(req_);

    // Submit the request. This will asynchronously call back.
    service_->SubmitRequest(req_);
    return true;

}

void HttpPackInfoManager::SetServiceUrl(HttpRequest *req) {
    std::string hash(base::Format::ToHex(temppackid_.hash,
            sizeof(temppackid_.hash)));
    req->SetURL(base::Format::ToString("%s/%08x-%s?c=%d&v=%d",
            kszPackInfoUrl, temppackid_.id, hash.c_str(),
            kdwClientID, knVersionSimulation));
}

void HttpPackInfoManager::Reset() {
    callback_ = NULL;
    ctx_ = NULL;
    code_ = 0;
    error_ = false;
    if (req_ != NULL) {
        if (service_ != NULL) {
            service_->ReleaseRequest(req_);
        }
        req_ = NULL;
    }
    cbTotal_ = 0;
    cbLength_ = 0;
    if (tempfile_ != NULL) {
        fclose(tempfile_);
        tempfile_ = NULL;
        unlink(tempfilename_.c_str());
    }
}

void HttpPackInfoManager::OnReceivedResponse(HttpRequest *preq, int code,
        const Map *pheaders) {
    code_ = code;
    if (code >= 400) {
        // Http error
        error_ = true;
        if (callback_ != NULL) {
            callback_->OnError(ctx_,
                    base::Format::ToString("Server returned error %d", code));
        }
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
        if (callback_ != NULL) {
            callback_->OnBegin(ctx_, cbLength_);
        }
        cbTotal_ = 0;
        return;
    }
    // Ignore other status codes. If it's a redirect, OnReceivedResponse
    // will get called again.
    return;
}

void HttpPackInfoManager::OnReceivedData(HttpRequest *preq,
        const base::ByteBuffer *pbb) {
    if (error_) {
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        size_t cb = fwrite(pbb->Data(), 1, pbb->Length(), tempfile_);
        if (cb != pbb->Length()) {
            if (callback_ != NULL) {
                error_ = true;
                callback_->OnError(ctx_, "Error saving Mission Pack info!");
            }
        } else {
            cbTotal_ += pbb->Length();
            if (callback_ != NULL) {
                callback_->OnProgress(ctx_, cbTotal_, cbLength_);
            }
        }
    }
}

void HttpPackInfoManager::OnFinishedLoading(HttpRequest *preq) {
    if (error_) {
        Reset();
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        if (FinishInstall()) {
            if (callback_ != NULL) {
                callback_->OnFinished(ctx_);
            }
        } else {
            if (callback_ != NULL) {
                error_ = true;
                callback_->OnError(ctx_, "Error parsing Mission Pack info!");
            }
        }
    }
    Reset();
}

void HttpPackInfoManager::OnError(HttpRequest *preq, const char *pszError) {
    if (error_) {
        return;
    }
    error_ = true;
    if (callback_ != NULL) {
        callback_->OnError(ctx_, pszError);
    }
}

bool HttpPackInfoManager::FinishInstall() {    
    if (tempfile_ == NULL) {
        return false;
    }

    // Build a json object to make sure it can be parsed

    json::JsonMap *map = LoadInfoMap(tempfile_);
    fclose(tempfile_);
    tempfile_ = NULL;
    if (map == NULL) {
        unlink(tempfilename_.c_str());
        return false;
    }

    // Check that the id and hash matches
    PackId packid;
    if (!GetPackId(map, &packid)  ||
            packid.id != temppackid_.id ||
            memcmp(packid.hash, temppackid_.hash, 16) != 0) {
        delete map;
        unlink(tempfilename_.c_str());
        return false;
    }
    delete map;

    // Move from temp directory to cache directory. Don't bother to delete
    // old info's with the same id (but different hash).

    if (rename(tempfilename_.c_str(), GetInfoFilename(&temppackid_)) < 0) {
        unlink(tempfilename_.c_str());
        return false;
    }
    return true;
}

} // namespace wi
