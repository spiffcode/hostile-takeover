#ifndef __HTTPPACKMANAGER_H__
#define __HTTPPACKMANAGER_H__

#include "mpshared/packmanager.h"
#include "game/httpservice.h"
#include "game/progresscallback.h"
#include <string>

namespace wi {

class HttpPackManager : public PackManager, HttpResponseHandler {
public:
    HttpPackManager(HttpService *service, const char *cachedir,
            const char *tempdir);
    ~HttpPackManager();

    bool Install(const PackId *ppackid, void *ctx, ProgressCallback *callback);
    bool Install(const char *pszURL, PackId *ppackid, void *ctx,
            ProgressCallback *callback);
    void Cancel();

private:
    void SetServiceUrl(HttpRequest *req);
    bool FinishInstall();

    // HttpResponseHandler methods
    virtual void OnReceivedResponse(HttpRequest *preq, int code,
            const Map *pheaders);
    virtual void OnReceivedData(HttpRequest *preq,
            const base::ByteBuffer *pbb);
    virtual void OnFinishedLoading(HttpRequest *preq);
    virtual void OnError(HttpRequest *preq, const char *pszError);

    HttpService *service_;
    void *ctx_;
    ProgressCallback *callback_;
    HttpRequest *req_;
    int code_;
    bool error_;
    int cbTotal_;
    int cbLength_;
    std::string tempfilename_;
    FILE *tempfile_;
    PackId temppackid_;
    PackId *ppackidUpdate_;
    std::string tempdir_;
    bool haveHash_;
};
extern HttpPackManager *gppackm;

} // namespace wi

#endif // __HTTPPACKMANAGER_H__

