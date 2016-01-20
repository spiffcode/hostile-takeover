#include "game/iphone/iphonehttprequest.h"
#include "game/iphone/input.h"
#include "game/iphone/iphone.h"
#include "base/thread.h"

// Requests come in from the game thread; NS* api calls occur on main
// thread.


// C++ implementation of HttpRequest interface for iPhone

namespace wi {

IPhoneHttpRequest::IPhoneHttpRequest(HttpResponseHandler *handler) : handler_(handler) {
}

IPhoneHttpRequest::~IPhoneHttpRequest() {
}

void IPhoneHttpRequest::Dispose() {
    // Called on game thread
    thread_.Clear(this);
    MessageHandler::Dispose();
}

NSURLRequest *IPhoneHttpRequest::CreateNSURLRequest() {
    // Called on main thread
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSMutableURLRequest *req = [[NSMutableURLRequest alloc] init];

    // Set the url
    NSString *s = [NSString stringWithCString:url_.c_str()
            encoding:[NSString defaultCStringEncoding]];
    [req setURL:[NSURL URLWithString:s]];

    // Set the method
    s = [NSString stringWithCString:method_.c_str()
            encoding:[NSString defaultCStringEncoding]];
    [req setHTTPMethod:s];

    // Set the body
    if (pbb_ != NULL) {
        int cb;
        void *pv = pbb_->Strip(&cb);
        NSData *data = [NSData dataWithBytesNoCopy:(void *)pv length:cb];
        [req setHTTPBody:data];
    }

    // Set timeout
    [req setTimeoutInterval:timeout_];

    // Set cache policy
    [req setCachePolicy:NSURLRequestReloadIgnoringCacheData];

    // Set headers
    Enum enm;
    char szKey[128];
    while (headers_.EnumKeys(&enm, szKey, sizeof(szKey))) {
        char szValue[256];
        if (headers_.GetValue(szKey, szValue, sizeof(szValue))) {
            NSString *key = [NSString stringWithCString:szKey
                    encoding:[NSString defaultCStringEncoding]];
            NSString *value = [NSString stringWithCString:szValue
                    encoding:[NSString defaultCStringEncoding]];
            [req setValue:value forHTTPHeaderField:key];
        }
    }

    // Done
    [pool release];
    return req;
}

void IPhoneHttpRequest::OnMessage(base::Message *pmsg) {
    switch (pmsg->id) {
    case kidmReceivedResponse:
        {
            ReceivedResponseParams *pparams =
                    (ReceivedResponseParams *)pmsg->data;
            handler_->OnReceivedResponse(this, pparams->code,
                    &pparams->headers);
            delete pparams;
        }
        break;

    case kidmReceivedData:
        {
            ReceivedDataParams *pparams =
                    (ReceivedDataParams *)pmsg->data;
            handler_->OnReceivedData(this, &pparams->bb);
            delete pparams;
        }
        break;

    case kidmFinishedLoading:
        handler_->OnFinishedLoading(this);
        break;

    case kidmError:
        {
            ErrorParams *pparams = (ErrorParams *)pmsg->data;
            handler_->OnError(this, pparams->szError);
            delete pparams;
        }
        break;
    }
}

void IPhoneHttpRequest::OnReceivedResponse(NSHTTPURLResponse *resp) {
    // Called on main thread. Populate ReceivedResponseParams
    ReceivedResponseParams *pparams = new ReceivedResponseParams;
    NSDictionary *dict = [resp allHeaderFields];
    for (NSString *k in dict) {
        NSString *v = [dict objectForKey:k];
        pparams->headers.SetValue(
            [k cStringUsingEncoding:[NSString defaultCStringEncoding]],
            [v cStringUsingEncoding:[NSString defaultCStringEncoding]]);
    }
    pparams->code = (int)[resp statusCode];

    // Post this to the game thread
    thread_.Post(kidmReceivedResponse, this, pparams);
}

void IPhoneHttpRequest::OnReceivedData(NSData *data) {
    // Called on main thread. Populate ReceivedDataParams
    ReceivedDataParams *pparams = new ReceivedDataParams;
    pparams->bb.WriteBytes((const byte *)[data bytes], (int)[data length]);

     // Post this to the game thread
    thread_.Post(kidmReceivedData, this, pparams);
}

void IPhoneHttpRequest::OnFinishedLoading() {
    // Called on main thread. Post this to game thread.
    thread_.Post(kidmFinishedLoading, this);
}

void IPhoneHttpRequest::OnError(NSError *error) {
    // Called on main thread. Populate ErrorParams. Use
    // localizedDescription. Note there is also localizedFailureReason;
    // not sure which is better at the moment
    const char *psz = [[error localizedDescription] cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    ErrorParams *pparams = new ErrorParams;
    strncpyz(pparams->szError, psz, sizeof(pparams->szError));

    // Called on main thread. Post this to game thread.
    thread_.Post(kidmError, this, pparams);
}
            
} // namespace wi
