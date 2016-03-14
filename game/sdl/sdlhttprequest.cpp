#include "game/sdl/sdlhttprequest.h"
#include "game/sdl/hosthelpers.h"
#include "base/thread.h"

#include <curl/curl.h>
#include <pthread.h>

// Curl implementation of HttpRequest for SDL
namespace wi {

size_t SdlHttpRequest::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	LOG() << "WriteMemoryCallback";

	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	if (mem->what == CURLOPT_WRITEDATA)
	{
		LOG() << "Data size: " << realsize;
		ReceivedDataParams *pparams = new ReceivedDataParams;
		pparams->bb.WriteBytes((const byte*)contents,realsize);
		thread_.Post(kidmReceivedData, this, pparams);
	}
	else if (mem->what == CURLOPT_HEADERDATA)
	{
		LOG() << "Header";
		ReceivedResponseParams *pparams = new ReceivedResponseParams;
		pparams->code = 200;
		thread_.Post(kidmReceivedResponse, this, pparams);
	}

	mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		LOG() << "not enough memory (realloc returned NULL)";
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

SdlHttpRequest::SdlHttpRequest(HttpResponseHandler *handler) : handler_(handler) {
	LOG() << "SdlHttpRequest::SdlHttpRequest";
}

SdlHttpRequest::~SdlHttpRequest() {
}

void *SdlHttpRequest::doAccess(void *arg)
{

	CURL *curl_handle;
	CURLcode res;
	/* init the curl session */
	curl_handle = curl_easy_init();

	struct MemoryStruct chunk;
	struct MemoryStruct chunkHeader;

	chunk.cls = this;
	chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk.size = 0;    /* no data at this point */
	chunk.what = CURLOPT_WRITEDATA;

	chunkHeader.cls = this;
	chunkHeader.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunkHeader.what = CURLOPT_HEADERDATA;
	chunkHeader.size = 0;    /* no data at this point */

	curl_global_init(CURL_GLOBAL_ALL);

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, url_.c_str());

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, SdlHttpRequest::WriteMemoryCallback_helper);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle,  CURLOPT_HEADERDATA, (void *)&chunkHeader);

	curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT,10);

	/* some servers don't like requests that are made without a user-agent
		      field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

	res = curl_easy_perform(curl_handle);

	long http_code = 0;

	/* check for errors */
	if(res != CURLE_OK) {
		LOG() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
		ErrorParams *pparams = new ErrorParams;
		strncpyz(pparams->szError, curl_easy_strerror(res), sizeof(pparams->szError));

		// Called on main thread. Post this to game thread.
		thread_.Post(kidmError, this, pparams);
	}
	else {
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

		LOG() << "Resp code = " << http_code;

		//base::ByteBuffer bb;
		//bb.WriteBytes((const byte*)chunkHeader.memory,chunkHeader.size);

		//ReceivedResponseParams *pparams = new ReceivedResponseParams;
		//pparams->code = http_code;
		//thread_.Post(kidmReceivedResponse, this, pparams);

		if (http_code == 200)
		{
			LOG() << "Data length: " << chunk.size;
			//ReceivedDataParams *pparams = new ReceivedDataParams;
			//pparams->bb.WriteBytes((const byte*)chunk.memory,chunk.size);
			//thread_.Post(kidmReceivedData, this, pparams);
		}

		LOG() << "%lu bytes retrieved" << (long)chunk.size;
		LOG() << "got: " << chunk.memory;
	}

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if(chunk.memory)
		free(chunk.memory);

	if(chunkHeader.memory)
		free(chunkHeader.memory);

	thread_.Post(kidmFinishedLoading, this);

	return 0;
}

void SdlHttpRequest::Submit() {
	LOG() << "SdlHttpRequest::SdlHttpRequest, URL = " << url_.c_str();

	pthread_t pth; // this is our thread identifier
	pthread_create(&pth,NULL, SdlHttpRequest::doAccess_helper,this);

}

void SdlHttpRequest::Release() {
	LOG() << "SdlHttpRequest::Release";
	// TODO: need to abort thread..
	thread_.Clear(this);
}

void SdlHttpRequest::OnMessage(base::Message *pmsg) {
	LOG() << "SdlHttpRequest::OnMessage";

	switch (pmsg->id) {
	case kidmReceivedResponse:
	{
		ReceivedResponseParams *pparams =
				(ReceivedResponseParams *)pmsg->data;
		handler_->OnReceivedResponse(this, pparams->code,
				&pparams->headers);
		delete pparams;
        break;
	}

	case kidmReceivedData:
	{
		ReceivedDataParams *pparams =
				(ReceivedDataParams *)pmsg->data;
		handler_->OnReceivedData(this, &pparams->bb);
		delete pparams;
        break;
	}

	case kidmFinishedLoading:
		handler_->OnFinishedLoading(this);
		break;

	case kidmError:
	{
		ErrorParams *pparams = (ErrorParams *)pmsg->data;
		handler_->OnError(this, pparams->szError);
		delete pparams;
        break;
	}

	} // switch
}

} // namespace wi
