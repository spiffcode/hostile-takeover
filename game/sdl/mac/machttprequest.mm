#include "game/sdl/mac/machttprequest.h"
#include "base/thread.h"

@implementation ConnectionDelegate

- (id)initWithRequest:(wi::MacHttpRequest *)req {
    self = [super init];
    if (self != nil) {
        req_ = req;
        conn_ = nil;
    }
    return self;
}
    
- (void)dealloc {
    [conn_ release];
    [super dealloc];
}

- (void)submit {
    NSURLRequest *req = req_->CreateNSURLRequest();
    conn_ = [NSURLConnection
            connectionWithRequest:req
            delegate:self];
    [conn_ retain];
    [req release];
}

- (void)cancel {
    [conn_ cancel];
    [conn_ release];
    conn_ = nil;
}

- (void)connection:(NSURLConnection *)conn
        didReceiveResponse:(NSURLResponse *)resp {
    req_->OnReceivedResponse((NSHTTPURLResponse *)resp);
}

- (void)connection:(NSURLConnection *)conn didReceiveData:(NSData *)data {
    req_->OnReceivedData(data);
}

- (void)connectionDidFinishLoading:(NSURLConnection *)conn {
    req_->OnFinishedLoading();
}

- (void)connection:(NSURLConnection *)conn
        didFailWithError:(NSError *)error {
    NSLog(@"error: %@", error);
    req_->OnError(error);
}
@end

// C++ implementation of HttpRequest interface for Mac

namespace wi {

MacHttpRequest::MacHttpRequest(HttpResponseHandler *handler) :
        handler_(handler), delegate_(nil) {
}

MacHttpRequest::~MacHttpRequest() {
}

void MacHttpRequest::Submit() {
    delegate_ = [[ConnectionDelegate alloc] initWithRequest:this];
    [delegate_ submit];
}

void MacHttpRequest::Release() {
    [delegate_ cancel];
    [delegate_ release];
    delegate_ = nil;
    Dispose();
}

NSURLRequest *MacHttpRequest::CreateNSURLRequest() {
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

void MacHttpRequest::OnReceivedResponse(NSHTTPURLResponse *resp) {
    Map headers;
    NSDictionary *dict = [resp allHeaderFields];
    for (NSString *k in dict) {
        NSString *v = [dict objectForKey:k];
        headers.SetValue(
            [k cStringUsingEncoding:[NSString defaultCStringEncoding]],
            [v cStringUsingEncoding:[NSString defaultCStringEncoding]]);
    }
    int code = [resp statusCode];
    handler_->OnReceivedResponse(this, code, &headers);
}

void MacHttpRequest::OnReceivedData(NSData *data) {
    base::ByteBuffer bb;
    bb.WriteBytes((const byte *)[data bytes], [data length]);
    handler_->OnReceivedData(this, &bb);
}

void MacHttpRequest::OnFinishedLoading() {
    handler_->OnFinishedLoading(this);
}

void MacHttpRequest::OnError(NSError *error) {
    const char *psz = [[error localizedDescription] cStringUsingEncoding:
            [NSString defaultCStringEncoding]];
    char szError[80];
    strncpyz(szError, psz, sizeof(szError));
    handler_->OnError(this, szError);
}
            
} // namespace wi
