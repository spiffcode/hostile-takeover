#include "game/ht.h"
#include "game/httpindexloader.h"
#include "game/serviceurls.h"
#include <map>

namespace wi {

HttpIndexLoader::HttpIndexLoader(HttpService *service, PackManager *ppackm) {
    service_ = service;
    s_ppackm_ = ppackm;
    req_ = NULL;
    callback_ = NULL;
    ctx_ = NULL;
    Reset();
}

HttpIndexLoader::~HttpIndexLoader() {
    Reset();
}

void HttpIndexLoader::MergeInstalled(PackManager *ppackm) {
    // Merge in packs that are installed but not in the index. This way,
    // they show up in the mission list, can be deleted, etc.
    // Enumerate installed packs and do a lookup. To make this fast,
    // first create a map.

    std::map<dword,IndexEntry *> map;
    for (int i = 0; i < (int)index_.size(); i++) {
        IndexEntry *entry = &index_[i];
        map.insert(std::map<dword,IndexEntry *>::value_type(entry->packid.id,
                entry));
    }

    // Enumerate installed packs

    Enum enm;
    PackId packid;
    while (ppackm->EnumPacks(&enm, &packid)) {
        // If in the index, move on.
        std::map<dword,IndexEntry *>::const_iterator it = map.find(packid.id);
        if (it != map.end()) {
            continue;
        }

        // Add it to the index. These packs don't have index info nor
        // necessarily PackInfo, so it has to be accurately generated.

        AddIndexEntry(&packid, false);
    }
}

void HttpIndexLoader::AddIndexEntry(const PackId *ppackid, bool fDupCheck) {
    if (fDupCheck) {
        for (int i = 0; i < (int)index_.size(); i++) {
            IndexEntry *entry = &index_[i];
            if (memcmp(ppackid, &entry->packid, sizeof(*ppackid)) == 0) {
                return;
            }
        }
    }

    MissionList *pml = CreateMissionList(ppackid, kmltAll);
    if (pml == NULL) {
        return;
    }
    if (pml->GetCount() == 0) {
        delete pml;
        return;
    }

    int cPlayersMin = 999;
    int cPlayersMax = -1;
    MissionDescription md;
    for (int i = pml->GetCount() - 1; i >= 0; i--) {
        if (!pml->GetMissionDescription(i, &md)) {
            delete pml;
            return;
        }
        if (md.cPlayersMin < cPlayersMin) {
            cPlayersMin = md.cPlayersMin;
        }
        if (md.cPlayersMax > cPlayersMax) {
            cPlayersMax = md.cPlayersMax; 
        }
    }

    IndexEntry entry;
    entry.packid = *ppackid;
    entry.title = md.szPackName;
    entry.cMissions = pml->GetCount(); 
    entry.cPlayersMin = cPlayersMin;
    entry.cPlayersMax = cPlayersMax;
    entry.inIndex = false;
    index_.push_back(entry);
    delete pml;
    sort_ = SORT_UNSORTED;
}

bool HttpIndexLoader::OnRemoved(const PackId *ppackid) {
    // Find the pack in the list. If it's not part of the original index,
    // remove it.

    Index::iterator it = index_.begin();
    for (; it != index_.end(); it++) {
        if (it->packid.id != ppackid->id) {
            continue;
        }
        if (memcmp(&(it->packid), ppackid, sizeof(it->packid)) != 0) {
            continue;
        }
        if (it->inIndex) {
            break;
        }
        index_.erase(it);
        return true;
    }
    return false;
}        

void HttpIndexLoader::AddFakeEntry(const PackId *ppackid, const char *title,
        int cPlayersMin, int cPlayersMax, int cMissions) {
    IndexEntry entry;
    entry.packid = *ppackid;
    entry.title = title;
    entry.cPlayersMin = cPlayersMin;
    entry.cPlayersMax = cPlayersMax;
    entry.cMissions = cMissions;
    entry.inIndex = false;
    index_.push_back(entry);
    sort_ = SORT_UNSORTED;
}

bool HttpIndexLoader::Start(void *ctx, ProgressCallback *callback) {
    if (service_ == NULL) {
        return false;
    }

    // Cancel any ongoing requests
    Reset();

    // Start a new request
    ctx_ = ctx;
    callback_ = callback;
    req_ = service_->NewRequest(this);
    if (req_ == NULL) {
        return false;
    }
    SetServiceUrl(req_);

    // Start the json builder. Tell it to call back with array items.
    builder_.Start(this);

    // Submit the request. This will asynchronously call back.
    service_->SubmitRequest(req_);
    return true;
}

void HttpIndexLoader::SetServiceUrl(HttpRequest *req) {
    req->SetURL(base::Format::ToString("%s?c=%d&v=%d", kszIndexUrl,
            kdwClientID, knVersionSimulation));
}

void HttpIndexLoader::Reset() {
    code_ = 0;
    error_ = false;
    if (req_ != NULL) {
        if (service_ != NULL) {
            service_->ReleaseRequest(req_);
        }
        req_ = NULL;
    }
    builder_.Reset();
    index_.clear();
    sort_ = SORT_UNSORTED;
    cbTotal_ = 0;
    cbLength_ = 0;
}

void HttpIndexLoader::OnReceivedResponse(HttpRequest *preq, int code,
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

void HttpIndexLoader::OnReceivedData(HttpRequest *preq,
        const base::ByteBuffer *pbb) {
    if (error_) {
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        if (!builder_.Update((const char *)pbb->Data(), pbb->Length())) {
            if (callback_ != NULL) {
                error_ = true;
                callback_->OnError(ctx_, "Error parsing Mission Pack list");
            }
        } else {
            cbTotal_ += pbb->Length();
            if (callback_ != NULL) {
                callback_->OnProgress(ctx_, cbTotal_, cbLength_);
            }
        }
    }
}

void HttpIndexLoader::OnFinishedLoading(HttpRequest *preq) {
    if (error_) {
        return;
    }
    if (code_ >= 200 && code_ < 300) {
        if (callback_ != NULL) {
            callback_->OnFinished(ctx_);
        }
    }
}

void HttpIndexLoader::OnError(HttpRequest *preq, const char *pszError) {
    if (error_) {
        return;
    }
    error_ = true;
    if (callback_ != NULL) {
        callback_->OnError(ctx_, pszError);
    }
}

void HttpIndexLoader::OnObject(json::JsonObject *obj) {
    if (error_) {
        delete obj;
        return;
    }
    IndexLoader::OnObject(obj);
}

void HttpIndexLoader::OnParseError() {
    error_ = true;
    if (callback_ != NULL) {
        callback_->OnError(ctx_, "Error parsing Mission Pack list");
    }
}

bool InstallAscendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    PackManager *s_ppackm = HttpIndexLoader::s_ppackm_;
    return s_ppackm->IsInstalled(&e1.packid) <
            s_ppackm->IsInstalled(&e2.packid);
}

bool InstallDescendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    PackManager *s_ppackm = HttpIndexLoader::s_ppackm_;
    return s_ppackm->IsInstalled(&e1.packid) >
            s_ppackm->IsInstalled(&e2.packid);
}

bool TitleAscendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    return stricmp(e1.title.c_str(), e2.title.c_str()) < 0;
}

bool TitleDescendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    return stricmp(e1.title.c_str(), e2.title.c_str()) > 0;
}

bool PlayersAscendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    if (e1.cPlayersMin < e2.cPlayersMin) {
        return true;
    }
    if (e1.cPlayersMin == e2.cPlayersMin) {
        if (e1.cPlayersMax < e2.cPlayersMax) {
            return true;
        }
    }
    return false;
}

bool PlayersDescendingSort(const IndexEntry& e1, const IndexEntry& e2) {
    if (e1.cPlayersMin > e2.cPlayersMin) {
        return true;
    }
    if (e1.cPlayersMin == e2.cPlayersMin) {
        if (e1.cPlayersMax > e2.cPlayersMax) {
            return true;
        }
    }
    return false;
}

bool MissionsAscendingSort(const IndexEntry& e1,
        const IndexEntry& e2) {
    return e1.cMissions < e2.cMissions;
}

bool MissionsDescendingSort(const IndexEntry& e1,
        const IndexEntry& e2) {
    return e1.cMissions > e2.cMissions;
}

typedef bool (* SortFunction)(const IndexEntry& e1, const IndexEntry& e2);

static SortFunction s_apfnSort[] = {
    NULL,
    InstallAscendingSort,
    InstallDescendingSort,
    TitleAscendingSort,
    TitleDescendingSort,
    PlayersAscendingSort,
    PlayersDescendingSort,
    MissionsAscendingSort,
    MissionsDescendingSort
};

PackManager *HttpIndexLoader::s_ppackm_;
void HttpIndexLoader::Sort(SortType sort) {
    if (sort == sort_) {
        return;
    }
    sort_ = sort;
    std::stable_sort(index_.begin(), index_.end(), s_apfnSort[sort_]);
}

} // namespace wi
