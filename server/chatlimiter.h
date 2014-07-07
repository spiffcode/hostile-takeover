#ifndef __CHATLIMITER_H__
#define __CHATLIMITER_H__

#include "server/endpoint.h"
#include "server/tracker.h"
#include "base/sigslot.h"
#include <map>

namespace wi {

// Uses a token bucket algorithm. Imagine a bucket being filled up with
// tokens at a constant rate. Each chat takes one token out of the bucket.
// If it's empty, the chat doesn't get sent.

struct Chatter {
    Chatter(int token_count) : token_count(token_count), tLast(0),
            tExpires(0), repeat_count(0) {}
    float token_count;
    long64 tLast;
    long64 tExpires;
    std::string last_chat;
    int repeat_count;
};

const int knChatLimitResultNotLimited = 0;
const int knChatLimitResultNewlyLimited  = 1;
const int knChatLimitResultLimited = 2;
const int kcChatLimitResultRepeatLimit = 3;

const int kcChatsPerDefault = 5;
const int kcSecondsPerDefault = 10;
const int kcMinutesTimeoutDefault = 3;
const int kcRepeatLimitDefault = 4;

const int kcMinutesTimeoutMaximum = 2 * 60;

class ChatLimiter : public base::has_slots<> {
public:
    ChatLimiter(int chat_count = kcChatsPerDefault,
            int seconds_count = kcSecondsPerDefault,
            int timeout_minutes = kcMinutesTimeoutDefault,
            int repeat_limit = kcRepeatLimitDefault);

    int Submit(Endpoint *endpoint, const char *chat, int *minutes_remaining);
    void Mute(Endpoint *endpoint, int minutes = kcMinutesTimeoutDefault);
    int timeout_minutes() { return timeout_minutes_ ; }
    Tracker& tracker() { return tracker_; }

private:
    Chatter *FindChatter(Endpoint *endpoint);
    Chatter *CreateChatter(Endpoint *endpoint);
    void OnEndpointDelete(Endpoint *endpoint);

    int timeout_minutes_;
    int token_limit_;
    int repeat_limit_;
    dword ms_per_token_;

    typedef std::map<dword, Chatter> ChatterMap;
    ChatterMap map_;
    Tracker tracker_;
};

} // namespace wi

#endif // __CHATLIMITER_H__
