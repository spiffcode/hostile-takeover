#ifndef __SIMPLEREQUEST_H__
#define __SIMPLEREQUEST_H__

#include "game/httpservice.h"
#include "base/bytebuffer.h"

namespace wi {

class SimpleRequest : HttpResponseHandler {
public:
    SimpleRequest(HttpService *service = NULL);
    ~SimpleRequest();

    const base::ByteBuffer *body() { return &bb_; }

    bool Get(const char *url, char *result = NULL, int resultsize = 0,
            char *error = NULL, int errorsize = 0);
    void SetTimeout(int seconds) { timeout_ = seconds; }
    void Reset();

private:
    // HttpResponseHandler methods
    virtual void OnReceivedResponse(HttpRequest *preq, int code,
            const Map *pheaders);
    virtual void OnReceivedData(HttpRequest *preq,
            const base::ByteBuffer *pbb);
    virtual void OnFinishedLoading(HttpRequest *preq);
    virtual void OnError(HttpRequest *preq, const char *pszError);

    HttpService *service_;
    HttpRequest *req_;
    int code_;
    bool error_;
    base::ByteBuffer bb_;
    char *errorstr_;
    int errorstrsize_;
    bool done_;
    int timeout_;
};

}
#endif // __SIMPLEREQUEST_H__
