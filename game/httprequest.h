#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include "inc/basictypes.h"
#include "game/map.h"
#include "base/bytebuffer.h"
#include <string>

namespace wi {

class HttpRequest {
public:
    HttpRequest();
    virtual ~HttpRequest();

    void SetURL(const char *pszUrl);
    void GetURL(char *pszUrl, int cb);
    void SetMethod(const char *pszMethod);
    void GetMethod(char *pszMethod, int cb);
    void SetBody(base::ByteBuffer *pbb);
    base::ByteBuffer *GetBody();
    void SetTimeout(int cSeconds);
    int GetTimeout();
    void SetHeaders(const Map *pheaders);
    const Map *GetHeaders();

protected:
    std::string url_;
    std::string method_;
    base::ByteBuffer *pbb_;
    int timeout_;
    Map headers_;
};

} // namespace wi

#endif // __HTTPREQUEST_H__
