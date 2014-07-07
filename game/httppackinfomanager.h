#ifndef __HTTPPACKINFOMANAGER_H__
#define __HTTPPACKINFOMANAGER_H__

#include "inc/basictypes.h"
#include "mpshared/packinfomanager.h"
#include "mpshared/mpht.h"
#include "game/httpservice.h"
#include "game/httprequest.h"
#include "game/progresscallback.h"
#include <string>

namespace wi {

class HttpPackInfoManager : public PackInfoManager, HttpResponseHandler {
public:
    HttpPackInfoManager(HttpService *service, const char *cachedir,
            const char *tempdir);
    ~HttpPackInfoManager();

    bool Start(const PackId *ppackid, void *ctx, ProgressCallback *callback);
    void Reset();

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
    std::string tempdir_;
};
extern HttpPackInfoManager *gppim;

} // namespace wi

#endif // __HTTPPACKINFOMANAGER_H__
