#ifndef __SDLHTTPSERVICE_H__
#define __SDLHTTPSERVICE_H__

#include "inc/basictypes.h"
#include "game/httpservice.h"

namespace wi {

class SdlHttpService : public HttpService {
public:
    // HttpService methods
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler);
    virtual void SubmitRequest(HttpRequest *preq);
    virtual void ReleaseRequest(HttpRequest *preq);
};

} // namespace wi

#endif // __SDLHTTPSERVICE_H__
