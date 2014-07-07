#ifndef __HTTPSERVICE_H__
#define __HTTPSERVICE_H__

#include "inc/basictypes.h"
#include "game/httprequest.h"
#include "game/map.h"
#include "base/bytebuffer.h"

namespace wi {

class HttpResponseHandler {
public:
    virtual void OnReceivedResponse(HttpRequest *preq, int code,
            const Map *pheaders) = 0;
    virtual void OnReceivedData(HttpRequest *preq,
            const base::ByteBuffer *pbb) = 0;
    virtual void OnFinishedLoading(HttpRequest *preq) = 0;
    virtual void OnError(HttpRequest *preq, const char *pszError) = 0;
};

class HttpService {
public:
    virtual ~HttpService() {}
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler) = 0;
    virtual void SubmitRequest(HttpRequest *preq) = 0;
    virtual void ReleaseRequest(HttpRequest *preq) = 0;
};
extern HttpService *gphttp;

} // namespace wi

#endif // __HTTPSERVICE_H__
