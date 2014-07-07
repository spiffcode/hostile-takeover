#ifndef __TOKENBUCKET_H__
#define __TOKENBUCKET_H__

#include "inc/basictypes.h"

namespace wi {

class TokenBucket {
public:
    TokenBucket(int token_count, int seconds_count);

    bool IsEmpty();

private:
    float token_count_;
    int token_count_limit_;
    dword ms_per_token_;
    long64 ms_last_token_;
};

} // namespace wi

#endif // __TOKENBUCKET_H__
