#ifndef __SDLHTTPREQUEST_H__
#define __SDLHTTPREQUEST_H__

#include "game/httpservice.h"
#include "game/httprequest.h"
#include "base/thread.h"
#include "base/bytebuffer.h"

namespace wi {

class SdlHttpRequest;

struct MemoryStruct {
	SdlHttpRequest *cls;
	int what;
	char *memory;
	size_t size;
};


class SdlHttpRequest : public HttpRequest, base::MessageHandler {
public:
	SdlHttpRequest(HttpResponseHandler *phandler);
	~SdlHttpRequest();

	void Submit();
	void Release();
	void OnFinishedLoading();

private:
    HttpResponseHandler *handler_;

    void *doAccess(void *arg);
    virtual void OnMessage(base::Message *pmsg);
    size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

	static void *doAccess_helper(void *context) {
		return ((SdlHttpRequest *)context)->doAccess(0);
	}

	static size_t WriteMemoryCallback_helper(void *contents, size_t size, size_t nmemb, void *userp)
	{
		return (((MemoryStruct *)userp)->cls)->WriteMemoryCallback(contents,size,nmemb,userp);
	}
};

struct ReceivedResponseParams : base::MessageData {
	int code;
	Map headers;
};

struct ReceivedDataParams : base::MessageData {
	base::ByteBuffer bb;
};

struct ErrorParams : base::MessageData {
	char szError[80];
};

} // namespace wi

#endif // __SDLHTTPREQUEST_H__
