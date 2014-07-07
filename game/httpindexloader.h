#ifndef __HTTPINDEXLOADER_H__
#define __HTTPINDEXLOADER_H__

#include "inc/basictypes.h"
#include "base/format.h"
#include "mpshared/indexloader.h"
#include "mpshared/packmanager.h"
#include "mpshared/mpht.h"
#include "game/map.h"
#include "game/httpservice.h"
#include "game/progresscallback.h"

namespace wi {

class HttpIndexLoader : public IndexLoader, HttpResponseHandler {
public:
    HttpIndexLoader(HttpService *service, PackManager *ppackm);
    ~HttpIndexLoader();

    bool Start(void *ctx, ProgressCallback *callback);
    void Reset();
    void AddIndexEntry(const PackId *ppackid, bool fDupCheck = true);
    void AddFakeEntry(const PackId *ppackid, const char *title,
            int cPlayersMin, int cPlayersMax, int cMissions);
    void MergeInstalled(PackManager *ppackm);
    bool OnRemoved(const PackId *ppackid);

    enum SortType {
        SORT_UNSORTED,
        SORT_INSTALLEDASCENDING,
        SORT_INSTALLEDDESCENDING,
        SORT_TITLEASCENDING,
        SORT_TITLEDESCENDING,
        SORT_PLAYERSASCENDING,
        SORT_PLAYERSDESCENDING,
        SORT_MISSIONSASCENDING,
        SORT_MISSIONSDESCENDING
    };
    void Sort(SortType sort);

private:
    void SetServiceUrl(HttpRequest *req);
    virtual void OnParseError();

    // HttpResponseHandler methods
    virtual void OnReceivedResponse(HttpRequest *preq, int code,
            const Map *pheaders);
    virtual void OnReceivedData(HttpRequest *preq,
            const base::ByteBuffer *pbb);
    virtual void OnFinishedLoading(HttpRequest *preq);
    virtual void OnError(HttpRequest *preq, const char *pszError);

    // ArrayItemsCallback
    virtual void OnObject(json::JsonObject *obj);

    HttpService *service_;
    void *ctx_;
    ProgressCallback *callback_;
    HttpRequest *req_;
    json::JsonBuilder builder_;
    int code_;
    bool error_;
    int cbTotal_;
    int cbLength_;
    SortType sort_;
    static PackManager *s_ppackm_;

    friend bool InstallAscendingSort(const IndexEntry& e1,
            const IndexEntry& e2);
    friend bool InstallDescendingSort(const IndexEntry& e1,
            const IndexEntry& e2);
};

} // namespace wi

#endif // __HTTPINDEXLOADER_H__

