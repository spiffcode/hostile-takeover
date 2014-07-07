#include "game/sdl/mac/machttpservice.h"
#include "game/sdl/mac/machttprequest.h"

namespace wi {

HttpRequest *MacHttpService::NewRequest(HttpResponseHandler *phandler) {
    return new MacHttpRequest(phandler);
}

void MacHttpService::SubmitRequest(HttpRequest *preq) {
    MacHttpRequest *preqT = (MacHttpRequest *)preq;
    preqT->Submit();
}

void MacHttpService::ReleaseRequest(HttpRequest *preq) {
    MacHttpRequest *preqT = (MacHttpRequest *)preq;
    preqT->Release();
}

} // namespace wi
