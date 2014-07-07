#ifndef __PROGRESSCALLBACK_H__
#define __PROGRESSCALLBACK_H__

namespace wi {

class ProgressCallback {
public:
    virtual void OnBegin(void *ctx, int cbLength) = 0;
    virtual void OnProgress(void *ctx, int cbTotal, int cbLength) = 0;
    virtual void OnFinished(void *ctx) = 0;
    virtual void OnError(void *ctx, const char *pszError) = 0;
};

} // namespace wi

#endif // __PROGRESSCALLBACK_H__
