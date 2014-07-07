#include "game/httprequest.h"

namespace wi {

HttpRequest::HttpRequest() {
    method_ = "GET";
    timeout_ = 30;
    pbb_ = NULL;
}

HttpRequest::~HttpRequest() {
    delete pbb_;
}

void HttpRequest::SetURL(const char *pszUrl) {
    url_ = pszUrl;
}

void HttpRequest::GetURL(char *pszUrl, int cb) {
    strncpyz(pszUrl, url_.c_str(), cb);
}

void HttpRequest::SetMethod(const char *pszMethod) {
    method_ = pszMethod;
}

void HttpRequest::GetMethod(char *pszMethod, int cb) {
    strncpyz(pszMethod, method_.c_str(), cb);
}

void HttpRequest::SetBody(base::ByteBuffer *pbb) {
    delete pbb_;
    pbb_ = pbb;
}

base::ByteBuffer *HttpRequest::GetBody() {
    return pbb_;
}

void HttpRequest::SetTimeout(int cSeconds) {
    timeout_ = cSeconds;
}

int HttpRequest::GetTimeout() {
    return timeout_;
}

void HttpRequest::SetHeaders(const Map *pheaders) {
    headers_ = *pheaders;
}

const Map *HttpRequest::GetHeaders() {
    return &headers_;
}

} // namespace wi
