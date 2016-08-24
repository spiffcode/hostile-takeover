#ifndef __IPHONEHTTPSERVICE_H__
#define __IPHONEHTTPSERVICE_H__

#include "inc/basictypes.h"
#include "game/httpservice.h"

namespace wi {

class IPhoneHttpService : public HttpService {
public:
    // HttpService methods
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler);
    virtual void SubmitRequest(HttpRequest *preq);
    virtual void ReleaseRequest(HttpRequest *preq);
};

} // namespace wi

#endif // __IPHONEHTTPSERVICE_H__
