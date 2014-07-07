#ifndef __SDLHTTPREQUEST_H__
#define __SDLHTTPREQUEST_H__

#include "game/httpservice.h"
#include "game/httprequest.h"
#include "base/thread.h"
#include "base/bytebuffer.h"

namespace wi {

class SdlHttpRequest : public HttpRequest, base::MessageHandler {
public:
    SdlHttpRequest(HttpResponseHandler *phandler);
    ~SdlHttpRequest();

    void Submit();
    void Release();
// TODO(darrinm):
//    NSURLRequest *CreateNSURLRequest();
//    void OnReceivedResponse(NSHTTPURLResponse *resp);
//    void OnReceivedData(NSData *data);
    void OnFinishedLoading();
// TODO(darrinm):
//    void OnError(NSError *error);

private:
    virtual void OnMessage(base::Message *pmsg);

    HttpResponseHandler *handler_;
// TODO(darrinm):
//    ConnectionDelegate *delegate_;
};

struct ReceivedResponseParams : base::MessageData {
    int code;
    Map headers;
};

struct ReceivedDataParams : base::MessageData {
    base::ByteBuffer bb;
};

struct ErrorParams : base::MessageData {
    char szError[80];
};

} // namespace wi

#endif // __SDLHTTPREQUEST_H__
