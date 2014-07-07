#include "server/tokenbucket.h"
#include "base/tick.h"

namespace wi {

TokenBucket::TokenBucket(int token_count, int seconds_count) {
    token_count_ = token_count;
    token_count_limit_ = token_count;
    ms_per_token_ = seconds_count * 1000 / token_count;
    ms_last_token_ = 0;
}

bool TokenBucket::IsEmpty() {
    long64 msCurrent = base::GetMillisecondCount();
    token_count_ += (float)(msCurrent - ms_last_token_) / (float)ms_per_token_;
    if (token_count_ > token_count_limit_) {
        token_count_ = token_count_limit_;
    }
    ms_last_token_ = msCurrent;
    if (token_count_ < 1.0) {
        return true;
    }
    token_count_ -= 1.0f;
    return false;
}

} // namespace wi
