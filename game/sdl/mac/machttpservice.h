#ifndef __MACHTTPSERVICE_H__
#define __MACHTTPSERVICE_H__

#include "inc/basictypes.h"
#include "game/httpservice.h"

namespace wi {

class MacHttpService : public HttpService {
public:
    MacHttpService();

    // HttpService methods
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler);
    virtual void SubmitRequest(HttpRequest *preq);
    virtual void ReleaseRequest(HttpRequest *preq);

private:
    void *delegate_;
};

} // namespace wi

#endif // __MACHTTPSERVICE_H__
