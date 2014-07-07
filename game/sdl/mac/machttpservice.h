#ifndef __MACHTTPSERVICE_H__
#define __MACHTTPSERVICE_H__

#include "inc/basictypes.h"
#include "game/httpservice.h"

namespace wi {

class MacHttpService : public HttpService {
public:
    // HttpService methods
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler);
    virtual void SubmitRequest(HttpRequest *preq);
    virtual void ReleaseRequest(HttpRequest *preq);
};

} // namespace wi

#endif // __MACHTTPSERVICE_H__
