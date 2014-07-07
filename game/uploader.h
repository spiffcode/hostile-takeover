#ifndef __UPLOADER_H__
#define __UPLOADER_H__

#include "inc/basictypes.h"
#include "game/httpservice.h"
#include "game/httprequest.h"
#include "game/map.h"
#include "base/bytebuffer.h"
#include <string>

namespace wi {

bool UploadByteBuffer(HttpService *service, base::ByteBuffer *bb,
        const char *url);

class TransportWaitingUI;

class Uploader : HttpResponseHandler {
public:
    Uploader(HttpService *service);
    ~Uploader();

    bool Upload(base::ByteBuffer *bb, const char *url);

private:
    void SetMessage(const char *message);
    void Wakeup();

    // HttpResponseHandler
    virtual void OnReceivedResponse(HttpRequest *preq, int code,
            const Map *pheaders);
    virtual void OnReceivedData(HttpRequest *preq,
            const base::ByteBuffer *pbb);
    virtual void OnFinishedLoading(HttpRequest *preq);
    virtual void OnError(HttpRequest *preq, const char *pszError);

    TransportWaitingUI *ui_;
    std::string message_;
    std::string errorstr_;
    HttpService *service_;
    HttpRequest *req_;
    int cbLength_;
    int cbTotal_;
    bool done_;
    bool error_;
};

}

#endif // __UPLOADER_H__
