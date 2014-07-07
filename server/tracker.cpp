#include "server/tracker.h"
#include "base/thread.h"
#include "base/tick.h"
#include <vector>

const dword TIMEOUT_CHECK_SECONDS = 2 * 60;

namespace wi {

Tracker::Tracker() {
    thread_.PostTimer(1, this,  TIMEOUT_CHECK_SECONDS * 100);
}

void Tracker::Add(const char *s, long64 tExpires) {
    // If it exists, just update the entry, otherwise add new
    std::string sT(s);
    TrackMap::iterator it = map_.find(sT);
    if (it != map_.end()) {
        it->second = tExpires;
        return;
    }
    map_.insert(TrackMap::value_type(sT, tExpires));
}


void Tracker::Remove(const char *s) {
    TrackMap::iterator it = map_.find(std::string(s));
    if (it != map_.end()) {
        map_.erase(it);
    }
}

bool Tracker::Find(const char *s, long64 *tExpires) {
    TrackMap::iterator it = map_.find(std::string(s));
    if (it == map_.end()) {
        return false;
    }
    long64 tCurrent = base::GetMillisecondCount();
    if (tCurrent > it->second) {
        map_.erase(it);
        return false;
    }
    if (tExpires != NULL) {
        *tExpires = it->second;
    }
    return true;
}

void Tracker::OnMessage(base::Message *pmsg) {
    // Remove stale entries
    long64 tCurrent = base::GetMillisecondCount();
    std::vector<std::string> v;
    for (TrackMap::iterator it = map_.begin(); it != map_.end(); it++) {
        if (tCurrent >= it->second) {
            v.push_back(it->first);
        }
    }
    std::vector<std::string>::iterator it = v.begin();
    for (; it != v.end(); it++) {
        TrackMap::iterator it_map = map_.find(*it);
        if (it_map != map_.end()) {
            map_.erase(it_map);
        }
    }
}

void Tracker::Clear() {
    map_.clear();
}

} // namespace wi
