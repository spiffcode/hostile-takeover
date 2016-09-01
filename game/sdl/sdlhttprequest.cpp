#include "game/sdl/sdlhttprequest.h"
#include "game/sdl/hosthelpers.h"
#include "base/thread.h"

#include <curl/curl.h>
#include <algorithm>

// Curl implementation of HttpRequest for SDL
namespace wi {

SdlHttpRequest::SdlHttpRequest(HttpResponseHandler *handler) : handler_(handler) {
}

SdlHttpRequest::~SdlHttpRequest() {
}

size_t SdlHttpRequest::WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	if (mem->what == CURLOPT_WRITEDATA)
	{
		ReceivedDataParams *pparams = new ReceivedDataParams;
		pparams->bb.WriteBytes((const byte*)contents, (int)realsize);
		thread_.Post(kidmReceivedData, this, pparams);
	}

    if (mem->what == CURLOPT_HEADERDATA)
	{
		ReceivedResponseParams *pparams = new ReceivedResponseParams;
		pparams->code = 200;

        // Stuff for delimiting
        size_t pos = 0;
        std::string delimiter = ":";
        std::string str((char *)contents);
        std::string key;
        std::string value;

        // Split the HTTP header into a key and value
        if ((pos = str.find(delimiter)) != std::string::npos) {
            key = str.substr(0, pos);
            str.erase(0, pos + delimiter.length());
        }
        value = str;

        // Remove newlines and spaces that sneak in sometimes
        value.erase(std::remove(value.begin(), value.end(), '\n'), value.end());
        value.erase(std::remove(value.begin(), value.end(), '\r'), value.end());
        value.erase(std::remove(value.begin(), value.end(), ' '), value.end());

        // handler_ doesn't read the Content-Length header properly when
        // other header(s) are posted. To be discovered why this happens.
        // The game only uses Content-Length so only post it for now.
        if (key == "Content-Length") {
            pparams->headers.SetValue(key.c_str(), value.c_str());
            thread_.Post(kidmReceivedResponse, this, pparams);
        }
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

void SdlHttpRequest::CreateCurlRequest() {
    /* init the curl session */
	curl_handle_ = curl_easy_init();

    chunk_.cls = this;
	chunk_.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunk_.size = 0;    /* no data at this point */
	chunk_.what = CURLOPT_WRITEDATA;

	chunkHeader_.cls = this;
	chunkHeader_.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
	chunkHeader_.what = CURLOPT_HEADERDATA;
	chunkHeader_.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

	/* specify URL to get */
	curl_easy_setopt(curl_handle_, CURLOPT_URL, url_.c_str());

    /* specify body to post */
    if (pbb_ != NULL) {
        int cb;
        void *pv = pbb_->Strip(&cb);
        curl_easy_setopt(curl_handle_, CURLOPT_POSTFIELDS, pv);
    }

	/* send all data to this function  */
	curl_easy_setopt(curl_handle_, CURLOPT_WRITEFUNCTION, SdlHttpRequest::WriteMemoryCallback_helper);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle_, CURLOPT_WRITEDATA, (void *)&chunk_);
	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle_,  CURLOPT_HEADERDATA, (void *)&chunkHeader_);

	curl_easy_setopt(curl_handle_, CURLOPT_CONNECTTIMEOUT,10);

	/* some servers don't like requests that are made without a user-agent
		      field, so we provide one */
	curl_easy_setopt(curl_handle_, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle_, CURLOPT_SSL_VERIFYHOST, 0L);
}

void SdlHttpRequest::SubmitCurlRequest(void *pv) {
    SdlHttpRequest *req = (SdlHttpRequest *)pv;

    // provide a buffer to store errors in
    char error[CURL_ERROR_SIZE];
    curl_easy_setopt(req->curl_handle_, CURLOPT_ERRORBUFFER, error);

    // submit
    CURLcode res = curl_easy_perform(req->curl_handle_);

    // check for errors
	if(res != CURLE_OK) {
		ErrorParams *pparams = new ErrorParams;

        if (strlen(error)) {
            // CURLOPT_ERRORBUFFER messages should be more helpful than curl_easy_strerror
            strncpyz(pparams->szError, error, CURL_ERROR_SIZE);
        } else {
            // Use curl_easy_strerror if CURLOPT_ERRORBUFFER not available
            strncpyz(pparams->szError, curl_easy_strerror(res), CURL_ERROR_SIZE);
        }

        req->thread().Post(kidmError, req, pparams);
    } else {
        req->thread().Post(kidmFinishedLoading, req);
    }
}

void SdlHttpRequest::Submit() {
    this->CreateCurlRequest();

    // SdlHttpRequest::Submit() is called on the game thread
    // so spin off a new thread to run the request on.
    base::Thread *thread = new base::Thread();
    thread->Start(SubmitCurlRequest, this);
}

void SdlHttpRequest::Release() {
	curl_easy_cleanup(curl_handle_);

	if (chunk_.memory)
		free(chunk_.memory);

	if (chunkHeader_.memory)
		free(chunkHeader_.memory);

	thread_.Clear(this);
}

void SdlHttpRequest::OnMessage(base::Message *pmsg) {
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
}

} // namespace wi
