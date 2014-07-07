#include "server/chatlimiter.h"
#include "base/tick.h"

namespace wi {

ChatLimiter::ChatLimiter(int chat_count, int seconds_count,
        int timeout_minutes, int repeat_limit) {
    ms_per_token_ = seconds_count * 1000 / chat_count;
    token_limit_ = chat_count;
    timeout_minutes_ = timeout_minutes;
    repeat_limit_ = repeat_limit;
}

void ChatLimiter::OnEndpointDelete(Endpoint *endpoint) {
    // Stop tracking this endpoint
    ChatterMap::iterator it = map_.find(endpoint->id());
    if (it == map_.end()) {
        return;
    }

    // If currently blocked, remember the ip so it can be blocked if there
    // is a reconnect.
    long64 tCurrent = base::GetMillisecondCount();
    if (tCurrent < it->second.tExpires) {
        endpoint->AddTracker(tracker_, it->second.tExpires);
    }
    map_.erase(it);
}

Chatter *ChatLimiter::CreateChatter(Endpoint *endpoint) {
    // Create a new chatter
    map_.insert(ChatterMap::value_type(endpoint->id(), Chatter(token_limit_)));
    endpoint->SignalOnDelete.connect(this, &ChatLimiter::OnEndpointDelete);
    Chatter *chatter = FindChatter(endpoint);
    if (chatter == NULL) {
        return NULL;
    }

    // See if this user is being tracked. If so, pick up the settings and
    // remove the tracker.
    if (endpoint->FindTracker(tracker_, &chatter->tExpires)) {
        endpoint->RemoveTracker(tracker_);
    }
    return chatter;
}

Chatter *ChatLimiter::FindChatter(Endpoint *endpoint) {
    // First see if this endpoint is already being tracked.
    ChatterMap::iterator it = map_.find(endpoint->id());
    if (it != map_.end()) {
        return &it->second;
    }
    return NULL;
}

void ChatLimiter::Mute(Endpoint *endpoint, int minutes) {
    // See if this endpoint is already being tracked. If not, add it.
    Chatter *chatter = FindChatter(endpoint);
    if (chatter == NULL) {
        chatter = CreateChatter(endpoint);
        if (chatter == NULL) {
            return;
        }
    }
    if (minutes < 0) {
        minutes = 0;
    }
    if (minutes > kcMinutesTimeoutMaximum) {
        minutes = kcMinutesTimeoutMaximum;
    }
    chatter->repeat_count = 0;
    chatter->tExpires = base::GetMillisecondCount() + minutes * 60000;
}

int ChatLimiter::Submit(Endpoint *endpoint, const char *chat,
        int *minutes_remaining) {
    // See if this endpoint is already being tracked. If not, add it.
    Chatter *chatter = FindChatter(endpoint);
    if (chatter == NULL) {
        chatter = CreateChatter(endpoint);
        if (chatter == NULL) {
            return knChatLimitResultNotLimited;
        }
    }

    // Calculate the # of new tokens to add to the bucket since last chat
    long64 tCurrent = base::GetMillisecondCount();
    chatter->token_count += (dword)(tCurrent - chatter->tLast) / ms_per_token_;
    if (chatter->token_count > token_limit_) {
        chatter->token_count = token_limit_;
    }
    chatter->tLast = tCurrent;

    // Is this chatter rate limited already? If so, wait until the limit
    // expires.
    if (tCurrent < chatter->tExpires) {
        *minutes_remaining = (int)(((chatter->tExpires - tCurrent + 500) / 1000
                + 59) / 60);
        return knChatLimitResultLimited;
    }

    // Is there a full token left? If not, this chatter has gone over
    // the rate limit.
    bool moderator = endpoint->IsModerator();
    if (chatter->token_count < 1.0 && !moderator) {
        chatter->repeat_count = 0;
        chatter->tExpires = tCurrent + timeout_minutes_ * 60000;
        *minutes_remaining = timeout_minutes_;
        return knChatLimitResultNewlyLimited;
    }
    chatter->token_count -= 1.0f;

    // Spamming the same string over and over?
    if ((strcmp(chatter->last_chat.c_str(), chat) == 0 || strlen(chat) > 80) &&
            !moderator) {
        chatter->repeat_count++;
        if (chatter->repeat_count >= repeat_limit_) {
            chatter->repeat_count = 0;
            chatter->tExpires = tCurrent + timeout_minutes_ * 60000;
            *minutes_remaining = timeout_minutes_;
            return knChatLimitResultNewlyLimited;
        }
    } else {
        chatter->last_chat = chat;
        chatter->repeat_count = 0;
    }

    // Looks good, let it go through
    return knChatLimitResultNotLimited;
}

} // namespace wi
