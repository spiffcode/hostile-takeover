#ifndef __STATEFRAME_H__
#define __STATEFRAME_H__

#include "game/statetracker.h"
#include "base/bytebuffer.h"

namespace wi {

class StateFrame { // stf
private:
    const char *QuadToString(dword quad);

    struct Entry {
        dword quad;
        dword value;
    };
    Entry *entries_;
    int count_;
    int index_;
    StateTracker *tracker_;
    long cUpdates_;
    bool block_;

public:
    StateFrame(StateTracker *tracker);
    ~StateFrame();

    bool Init(long cUpdates, bool block);
    bool Grow();
    bool block() { return block_; }

    void AddValue(dword quad, dword value, int iCounter) {
        if (index_ >= count_) {
            if (!Grow()) {
                return;
            }
        }
        Entry *entry = &entries_[index_];
        entry->quad = quad;
        entry->value = value;
        index_++;
        if (iCounter >= 0 && iCounter < index_) {
            entries_[iCounter].value++;
        }
    }

    int AddCountedValue(dword quad) {
        if (index_ >= count_) {
            if (!Grow()) {
                return -1;
            }
        }
        Entry *entry = &entries_[index_];
        entry->quad = quad;
        entry->value = 0;
        index_++;
        return index_ - 1;
    }

    dword GetHash();
    base::ByteBuffer *ToJson();
    long updates() { return cUpdates_; }
};

} // namespace wi

#endif // __STATEFRAME_H__
