#ifndef __SDLHTTPREQUEST_H__
#define __SDLHTTPREQUEST_H__

#include "game/httpservice.h"
#include "game/httprequest.h"
#include "base/thread.h"
#include "base/bytebuffer.h"

#include <curl/curl.h>

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
    CURL *curl_handle_;
    struct MemoryStruct chunk_;
	struct MemoryStruct chunkHeader_;

    virtual void OnMessage(base::Message *pmsg);
    static void SubmitCurlRequest(void *pv);
    void CreateCurlRequest();

    size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
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
	char szError[CURL_ERROR_SIZE];
};

} // namespace wi

#endif // __SDLHTTPREQUEST_H__
