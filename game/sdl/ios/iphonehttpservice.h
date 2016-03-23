#ifndef __IPHONEHTTPSERVICE_H__
#define __IPHONEHTTPSERVICE_H__

#include "game/sdl/ios/iphonehttprequest.h"
#include "inc/basictypes.h"
#include "game/httpservice.h"
#include <map>

typedef std::map<NSURLSessionDataTask *, wi::IPhoneHttpRequest *> TaskMap;

@interface SessionDelegate : NSObject<NSURLSessionDataDelegate> {
    NSURLSession *session_;
    TaskMap taskmap_;
}
@end

namespace wi {

class IPhoneHttpService : public HttpService {
public:
    IPhoneHttpService();

    // HttpService methods
    virtual HttpRequest *NewRequest(HttpResponseHandler *phandler);
    virtual void SubmitRequest(HttpRequest *preq);
    virtual void ReleaseRequest(HttpRequest *preq);

private:
    SessionDelegate *delegate_;
};

} // namespace wi

#endif // __IPHONEHTTPSERVICE_H__
