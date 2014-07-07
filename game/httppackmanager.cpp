#include "game/httppackmanager.h"
#include "game/serviceurls.h"
#include "mpshared/mpht.h"
#include "base/format.h"
#include "base/md5.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

namespace wi {

HttpPackManager::HttpPackManager(HttpService *service, const char *cachedir,
        const char *tempdir) : PackManager(cachedir), service_(service) {
    haveHash_ = false;
    ppackidUpdate_ = NULL;
    tempdir_ = tempdir;
    req_ = NULL;
    tempfile_ = NULL;
    Cancel();
}

HttpPackManager::~HttpPackManager() {
    Cancel();
}

bool HttpPackManager::Install(const PackId *ppackid, void *ctx,
        ProgressCallback *callback) {
    // No service?
    if (service_ == NULL) {
        return false;
    }
    
    // Cancel any ongoing requests
    Cancel();

    // Remember what we're downloading
    temppackid_ = *ppackid;
    ppackidUpdate_ = NULL;
    haveHash_ = true;

    // Create a temp filename for the temp file
    char szTemp[256];
    strncpyz(szTemp,
            base::Format::ToString("%s/packdl.XXXXX", tempdir_.c_str()),
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

    
    // Start a new request
    ctx_ = ctx;
    callback_ = callback;
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        fclose(tempfile_);
        return false;
    }
    SetServiceUrl(req_);

    // Submit the request. This will asynchronously call back.
    service_->SubmitRequest(req_);
    return true;
}

bool HttpPackManager::Install(const char *pszURL, PackId *ppackidUpdate,
        void *ctx, ProgressCallback *callback) {
    // No service?
    if (service_ == NULL) {
        return false;
    }
    
    // Cancel any ongoing requests
    Cancel();

    // Remember what we're downloading - an url. Use a hash of the URL
    // for the pack id. We don't know the pack hash yet.
    memset(&temppackid_, 0, sizeof(temppackid_));
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (const byte *)pszURL, strlen(pszURL));
    byte hashURL[16];
    MD5Final(hashURL, &md5);
    temppackid_.id = *(dword *)(&hashURL[0]) ^ *(dword *)(&hashURL[4]) ^
            *(dword *)(&hashURL[8]) ^ *(dword *)(&hashURL[12]);

    // Since this is a custom pack, we don't have the pack hash yet.
    // Remember this so it can be calculated later.
    ppackidUpdate_ = ppackidUpdate;
    haveHash_ = false;

    // Create a temp filename for the temp file
    char szTemp[256];
    strncpyz(szTemp,
            base::Format::ToString("%s/packdl.XXXXX", tempdir_.c_str()),
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
    
    // Start a new request using the custom URL
    ctx_ = ctx;
    callback_ = callback;
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        fclose(tempfile_);
        return false;
    }
    req_->SetURL(pszURL);

    // Submit the request. This will asynchronously call back.
    service_->SubmitRequest(req_);
    return true;
}

void HttpPackManager::SetServiceUrl(HttpRequest *req) {
    std::string hash(base::Format::ToHex(temppackid_.hash,
            sizeof(temppackid_.hash)));
    req->SetURL(base::Format::ToString("%s/%08x-%s?c=%d&v=%d",
            kszPackUrl, temppackid_.id, hash.c_str(),
            kdwClientID, knVersionSimulation));
}

void HttpPackManager::Cancel() {
    ppackidUpdate_ = NULL;
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

void HttpPackManager::OnReceivedResponse(HttpRequest *preq, int code,
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

void HttpPackManager::OnReceivedData(HttpRequest *preq,
        const base::ByteBuffer *pbb) {
    if (error_) {
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        size_t cb = fwrite(pbb->Data(), 1, pbb->Length(), tempfile_);
        if (cb != pbb->Length()) {
            if (callback_ != NULL) {
                error_ = true;
                callback_->OnError(ctx_,
                        "Error saving Mission Pack. Out of space?");
            }
        } else {
            cbTotal_ += pbb->Length();
            if (callback_ != NULL) {
                callback_->OnProgress(ctx_, cbTotal_, cbLength_);
            }
        }
    }
}

void HttpPackManager::OnFinishedLoading(HttpRequest *preq) {
    if (error_) {
        Cancel();
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
                callback_->OnError(ctx_, "Downloaded mission pack is invalid!");
            }
        }
    }
    Cancel();
}

void HttpPackManager::OnError(HttpRequest *preq, const char *pszError) {
    if (error_) {
        return;
    }
    error_ = true;
    if (callback_ != NULL) {
        callback_->OnError(ctx_, pszError);
    }
}

bool HttpPackManager::FinishInstall() {
    if (tempfile_ == NULL) {
        return false;
    }

    // Make sure it is an addon pack
    char szType[5];
    szType[4] = 0;
    fseek(tempfile_, 0x3c, SEEK_SET);
    fread(szType, 4, 1, tempfile_);
    if (strcmp(szType, kszTypeAddon) != 0) {
        fclose(tempfile_);
        tempfile_ = NULL;
        unlink(tempfilename_.c_str());
        return false;
    }

    // Calculate the hash
    fseek(tempfile_, 0, SEEK_SET);
    MD5_CTX ctx;
    MD5Init(&ctx);
    while (true) {
        byte ab[256];
        size_t cb = fread(ab, 1, sizeof(ab), tempfile_);
        if (cb == 0) {
            break;
        }
        MD5Update(&ctx, ab, cb);
    }
    byte hash[16];
    MD5Final(hash, &ctx);
    fclose(tempfile_);
    tempfile_ = NULL;

    // Check that the hash matches, if we have it, otherwise use this
    // hash!
    if (haveHash_) {
        if (memcmp(hash, temppackid_.hash, sizeof(hash)) != 0) {
            unlink(tempfilename_.c_str());
            return false;
        }
    } else {
        memcpy(temppackid_.hash, hash, sizeof(temppackid_.hash));
        if (ppackidUpdate_ != NULL) {
            *ppackidUpdate_ = temppackid_;
        }
    }

    // Hash matches. Now remove the pack with the same id, if there is one.
    Remove(&temppackid_, false);

    // Install the pack into the cache. This better not fail since
    // the old pack has been removed already.
    if (rename(tempfilename_.c_str(), GetPackFilepath(&temppackid_)) < 0) {
        unlink(tempfilename_.c_str());
        return false;
    }
   
    // Add this to the map 
    map_.insert(PackMap::value_type(temppackid_.id, temppackid_));
    return true;
}

} // namespace wi
