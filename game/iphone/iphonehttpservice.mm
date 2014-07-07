#include "game/iphone/iphonehttpservice.h"
#include "game/iphone/iphonehttprequest.h"
#include "game/iphone/input.h"

// HttpService calls come in on the game thread. In order to use
// iPhone NS* Http apis, requests execute on the main thread.

namespace wi {

HttpRequest *IPhoneHttpService::NewRequest(HttpResponseHandler *phandler) {
    return new IPhoneHttpRequest(phandler);
}

void IPhoneHttpService::SubmitRequest(HttpRequest *preq) {
    IPhoneHttpRequest *preqT = (IPhoneHttpRequest *)preq;
    preqT->Submit();
}

void IPhoneHttpService::ReleaseRequest(HttpRequest *preq) {
    IPhoneHttpRequest *preqT = (IPhoneHttpRequest *)preq;
    preqT->Release();
}

} // namespace wi
