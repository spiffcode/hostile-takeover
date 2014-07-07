#include "game/sdl/sdlhttpservice.h"
#include "game/sdl/sdlhttprequest.h"
//#include "game/iphone/input.h"

// HttpService calls come in on the game thread. In order to use
// iPhone NS* Http apis, requests execute on the main thread.

namespace wi {

HttpRequest *SdlHttpService::NewRequest(HttpResponseHandler *phandler) {
    return new SdlHttpRequest(phandler);
}

void SdlHttpService::SubmitRequest(HttpRequest *preq) {
    SdlHttpRequest *preqT = (SdlHttpRequest *)preq;
    preqT->Submit();
}

void SdlHttpService::ReleaseRequest(HttpRequest *preq) {
    SdlHttpRequest *preqT = (SdlHttpRequest *)preq;
    preqT->Release();
}

} // namespace wi
