#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>

#include "game/iphone/iphonehttpservice.h"
#include "game/iphone/iphonehttprequest.h"
#include "game/iphone/iphone.h"
#include "game/iphone/input.h"
#include <map>

typedef std::map<NSURLSessionDataTask *, wi::IPhoneHttpRequest *> TaskMap;

// HttpService calls come in on the game thread. In order to use
// iPhone NS* Http apis, requests execute on the main thread.

@interface IPhoneHttpRequestWrapper : NSObject {
    wi::IPhoneHttpRequest *req_;
}
@end

@implementation IPhoneHttpRequestWrapper
- (id)initWithRequest:(wi::IPhoneHttpRequest *)req {
    self = [super init];
    if (self != nil) {
        req_ = req;
    }
    return self;
}

- (wi::IPhoneHttpRequest *)request {
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

- (void)cancel:(IPhoneHttpRequestWrapper *)wrapper {
    wi::IPhoneHttpRequest *req = [wrapper request];
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

- (void)submit:(IPhoneHttpRequestWrapper *)wrapper {
    if (session_ == nil) {
        session_ = [NSURLSession
                sessionWithConfiguration:[NSURLSessionConfiguration defaultSessionConfiguration]
                delegate:self
                delegateQueue:[NSOperationQueue mainQueue]];
    }
    wi::IPhoneHttpRequest *req = [wrapper request];
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

IPhoneHttpService::IPhoneHttpService() {
    delegate_ = NULL;
}

HttpRequest *IPhoneHttpService::NewRequest(HttpResponseHandler *phandler) {
    return new IPhoneHttpRequest(phandler);
}

void IPhoneHttpService::SubmitRequest(HttpRequest *preq) {
    if (delegate_ == NULL) {
        delegate_ = [[SessionDelegate alloc] init];
    }
    IPhoneHttpRequestWrapper *wrapper = [[IPhoneHttpRequestWrapper alloc]
            initWithRequest:(IPhoneHttpRequest *)preq];
    SessionDelegate *delegate = (SessionDelegate *)delegate_;
    [delegate performSelectorOnMainThread:@selector(submit:)
            withObject:wrapper waitUntilDone: NO];
}

void IPhoneHttpService::ReleaseRequest(HttpRequest *preq) {
    // This can cause a deadlock when exiting because of how the main thread
    // is synchronizing with the game thread to exit before it does
    IPhoneHttpRequest *req = (IPhoneHttpRequest *)preq;
    if (!IPhone::IsExiting()) {
        IPhoneHttpRequestWrapper *wrapper = [[IPhoneHttpRequestWrapper alloc]
                initWithRequest:req];
        SessionDelegate *delegate = (SessionDelegate *)delegate_;
        [delegate performSelectorOnMainThread:@selector(cancel:)
                withObject:wrapper waitUntilDone: YES];
    }
    req->Dispose();
}

} // namespace wi
