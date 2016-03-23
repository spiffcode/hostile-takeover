#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include "game/sdl/mac/machttpservice.h"
#include "game/sdl/mac/machttprequest.h"
#include "game/sdl/sysmessages.h"
#include "game/sdl/hosthelpers.h"
#include <map>

typedef std::map<NSURLSessionDataTask *, wi::MacHttpRequest *> TaskMap;

// HttpService calls come in on the game thread. In order to use
// iPhone NS* Http apis, requests execute on the main thread.

@interface MacHttpRequestWrapper : NSObject {
    wi::MacHttpRequest *req_;
}
@end

@implementation MacHttpRequestWrapper
- (id)initWithRequest:(wi::MacHttpRequest *)req {
    self = [super init];
    if (self != nil) {
        req_ = req;
    }
    return self;
}

- (wi::MacHttpRequest *)request {
    return req_;
}
@end

@interface SessionDelegate : NSObject<NSURLSessionDataDelegate> {
    NSURLSession *session_;
    TaskMap taskmap_;
}
@end

@implementation SessionDelegate
- (id)init {
    self = [super init];
    if (self != nil) {
        session_ = nil;
    }
    return self;
}
    
- (void)dealloc {
    [session_ release];
    [super dealloc];
}

- (void)cancel:(MacHttpRequestWrapper *)wrapper {
    wi::MacHttpRequest *req = [wrapper request];
    NSURLSessionDataTask *task = nil;
    TaskMap::iterator it = taskmap_.begin();
    for (; it != taskmap_.end(); it++) {
        if (it->second == req) {
            task = it->first;
            taskmap_.erase(it);
            break;
        }
    }
    if (task != nil) {
        [task cancel];
        [task release];
    }
    [wrapper release];
}

- (void)submit:(MacHttpRequestWrapper *)wrapper {
    if (session_ == nil) {
        session_ = [NSURLSession
                sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]
                delegate:self
                delegateQueue:[NSOperationQueue mainQueue]];
    }
    wi::MacHttpRequest *req = [wrapper request];
    NSURLRequest *url_req = req->CreateNSURLRequest();
    NSURLSessionDataTask *task = [session_ dataTaskWithRequest:url_req];
    [url_req release];
    [task retain];
    taskmap_.insert(TaskMap::value_type(task, req));
    [task resume];
    [wrapper release];
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)task
        didReceiveResponse:(NSURLResponse *)resp
        completionHandler:(void (^)(NSURLSessionResponseDisposition disp))handler {
    TaskMap::iterator it = taskmap_.find(task);
    if (it != taskmap_.end()) {
        it->second->OnReceivedResponse((NSHTTPURLResponse *)resp);
    }
    handler(NSURLSessionResponseAllow);
}

- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)task
        didReceiveData:(NSData *)data {
    TaskMap::iterator it = taskmap_.find(task);
    if (it != taskmap_.end()) {
        it->second->OnReceivedData(data);
    }
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
        didCompleteWithError:(NSError *)error {
    TaskMap::iterator it = taskmap_.find((NSURLSessionDataTask *)task);
    if (it != taskmap_.end()) {
        if (error == nil) {
            it->second->OnFinishedLoading();
        } else {
            NSLog(@"error: %@", error);
            it->second->OnError(error);
        }
    }
}
@end

namespace wi {

MacHttpService::MacHttpService() {
    delegate_ = NULL;
}

HttpRequest *MacHttpService::NewRequest(HttpResponseHandler *phandler) {
    return new MacHttpRequest(phandler);
}

void MacHttpService::SubmitRequest(HttpRequest *preq) {
    if (delegate_ == NULL) {
        delegate_ = [[SessionDelegate alloc] init];
    }
    MacHttpRequestWrapper *wrapper = [[MacHttpRequestWrapper alloc]
            initWithRequest:(MacHttpRequest *)preq];
    SessionDelegate *delegate = (SessionDelegate *)delegate_;
    [delegate performSelectorOnMainThread:@selector(submit:)
            withObject:wrapper waitUntilDone: NO];
}

void MacHttpService::ReleaseRequest(HttpRequest *preq) {
    // This can cause a deadlock when exiting because of how the main thread
    // is synchronizing with the game thread to exit before it does
    MacHttpRequest *req = (MacHttpRequest *)preq;
    if (!HostHelpers::IsExiting()) {
        MacHttpRequestWrapper *wrapper = [[MacHttpRequestWrapper alloc]
                initWithRequest:req];
        SessionDelegate *delegate = (SessionDelegate *)delegate_;
        [delegate performSelectorOnMainThread:@selector(cancel:)
                withObject:wrapper waitUntilDone: YES];
    }
    req->Dispose();
}

} // namespace wi
