#ifndef __MACHTTPREQUEST_H__
#define __MACHTTPREQUEST_H__

#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include "game/httpservice.h"
#include "game/httprequest.h"
#include "base/thread.h"
#include "base/bytebuffer.h"

namespace wi {
class MacHttpRequest;
}

@interface ConnectionDelegate : NSObject {
    NSURLConnection *conn_;
    wi::MacHttpRequest *req_;
}
- (void)submit;
- (void)cancel;
- (void)connection:(NSURLConnection *)conn
        didFailWithError:(NSError *)error;
- (void)connection:(NSURLConnection *)conn didReceiveData:(NSData *)data;
- (void)connectionDidFinishLoading:(NSURLConnection *)conn;
@end

namespace wi {

class MacHttpRequest : public HttpRequest, base::MessageHandler {
public:
    MacHttpRequest(HttpResponseHandler *phandler);
    ~MacHttpRequest();

    void Submit();
    void Release();
    NSURLRequest *CreateNSURLRequest();
    void OnReceivedResponse(NSHTTPURLResponse *resp);
    void OnReceivedData(NSData *data);
    void OnFinishedLoading();
    void OnError(NSError *error);

private:
    HttpResponseHandler *handler_;
    ConnectionDelegate *delegate_;
};

} // namespace wi

#endif // __MACHTTPREQUEST_H__
