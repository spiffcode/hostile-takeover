#ifndef __REFMAP_H__
#define __REFMAP_H__

#include "game/map.h"

namespace wi {

class RefMap : public Map {
public:
    RefMap() : c_(0) {
    }

    void AddRef() {
        c_++;
    }

    bool Release() {
        c_--;
        if (c_ <= 0) {
            delete this;
            return true;
        }
        return false;
    }

    dword GetHash() {
        dword h = 0;
        KeyMap::iterator it = map_.begin();
        while (it != map_.end()) {
            h ^= HashString(it->first.c_str());
            h += HashString(it->second.c_str());
            it++;
        }
        return h;
    }

private:
    int c_;
};

} // namespace wi

#endif // __REFMAP_H__
