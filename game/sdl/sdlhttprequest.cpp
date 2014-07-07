#include "game/sdl/sdlhttprequest.h"
#include "game/sdl/hosthelpers.h"
#include "base/thread.h"

// C++ implementation of HttpRequest interface for SDL

namespace wi {

#if 0 // TODO(darrinm)
SdlHttpRequest::SdlHttpRequest(HttpResponseHandler *handler) :
        handler_(handler), delegate_(nil) {
#else
SdlHttpRequest::SdlHttpRequest(HttpResponseHandler *handler) {
    LOG() << "SdlHttpRequest constructor not implemented yet";
#endif
}

SdlHttpRequest::~SdlHttpRequest() {
}

void SdlHttpRequest::Submit() {
#if 0
    delegate_ = [[ConnectionDelegate alloc] initWithRequest:this];
    [delegate_ performSelectorOnMainThread:@selector(submit)
            withObject:nil waitUntilDone: NO];
#else
    LOG() << "SdlHttpRequest::Submit not implemented yet";
#endif
}

void SdlHttpRequest::Release() {
#if 0
    // This can cause a deadlock when exiting because of how the main thread
    // is synchronizing with the game thread to exit before it does

    if (!Sdl::IsExiting()) {
        [delegate_ performSelectorOnMainThread:@selector(cancel)
                withObject:nil waitUntilDone: YES];
        [delegate_ release];
    }
    delegate_ = nil;
    thread_.Clear(this);
    Dispose();
#else
    LOG() << "SdlHttpRequest::Release not implemented yet";
#endif
}

#if 0 // TODO(darrinm)
NSURLRequest *SdlHttpRequest::CreateNSURLRequest() {
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
}
#endif

void SdlHttpRequest::OnMessage(base::Message *pmsg) {
#if 0 // TODO(darrinm)
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
#endif
}

#if 0 // TODO(darrinm)
void SdlHttpRequest::OnReceivedResponse(NSHTTPURLResponse *resp) {
    // Called on main thread. Populate ReceivedResponseParams
    ReceivedResponseParams *pparams = new ReceivedResponseParams;
    NSDictionary *dict = [resp allHeaderFields];
    for (NSString *k in dict) {
        NSString *v = [dict objectForKey:k];
        pparams->headers.SetValue(
            [k cStringUsingEncoding:[NSString defaultCStringEncoding]],
            [v cStringUsingEncoding:[NSString defaultCStringEncoding]]);
    }
    pparams->code = [resp statusCode];

    // Post this to the game thread
    thread_.Post(kidmReceivedResponse, this, pparams);
}

void SdlHttpRequest::OnReceivedData(NSData *data) {
    // Called on main thread. Populate ReceivedDataParams
    ReceivedDataParams *pparams = new ReceivedDataParams;
    pparams->bb.WriteBytes((const byte *)[data bytes], [data length]);

     // Post this to the game thread
    thread_.Post(kidmReceivedData, this, pparams);
}

void SdlHttpRequest::OnFinishedLoading() {
    // Called on main thread. Post this to game thread.
    thread_.Post(kidmFinishedLoading, this);
}

void SdlHttpRequest::OnError(NSError *error) {
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
#endif

} // namespace wi
