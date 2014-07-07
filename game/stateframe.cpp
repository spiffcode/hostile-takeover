#include "game/stateframe.h"
#include "game/statetracker.h"
#include <stdlib.h>
#include <string.h>

#define kcEntriesInit 2500
#define kcEntriesGrow 500

namespace wi {

StateFrame::StateFrame(StateTracker *tracker) :
        tracker_(tracker), cUpdates_(0), entries_(NULL),
        count_(0), index_(0), block_(false) {
}

StateFrame::~StateFrame() {
    free(entries_);
}

bool StateFrame::Init(long cUpdates, bool block) {
    if (entries_ == NULL) {
        entries_ = (Entry *)malloc(sizeof(Entry) * kcEntriesInit);
        if (entries_ == NULL) {
            return false;
        }
        count_ = kcEntriesInit;
    }
    index_ = 0;
    cUpdates_ = cUpdates;
    block_ = block;
    return true;
}

bool StateFrame::Grow() {
    Entry *entriesnew = (Entry *)malloc(sizeof(Entry) *
            (kcEntriesGrow + count_));
    if (entriesnew == NULL) {
        return false;
    }
    memcpy(entriesnew, entries_, sizeof(Entry) * count_);
    free(entries_);
    entries_ = entriesnew;
    count_ += kcEntriesGrow;
    return true;
}

dword StateFrame::GetHash() {
    dword h = 0;
    Entry *entryMax = &entries_[index_];
    Entry *entry = entries_;
    for (Entry *entry = entries_; entry < entryMax; entry++) {
        h += entry->quad;
        h ^= entry->value;
    }
    return h;
}

const char *StateFrame::QuadToString(dword quad) {
    static char s_achQuad[5];
    char *ach = (char *)&quad;
    s_achQuad[0] = ach[3];
    s_achQuad[1] = ach[2] != ' ' ? ach[2] : 0;
    s_achQuad[2] = ach[1] != ' ' ? ach[1] : 0;
    s_achQuad[3] = ach[0] != ' ' ? ach[0] : 0;
    s_achQuad[4] = 0;
    return s_achQuad;
}

base::ByteBuffer *StateFrame::ToJson() {
    base::ByteBuffer *bb = new base::ByteBuffer;
    if (bb == NULL) {
        return NULL;
    }
    bb->WriteString("{", false);
    bb->WriteString(base::Format::ToString("\"updates\":\"%d\"", cUpdates_),
            false);
    bb->WriteString(base::Format::ToString(",\"hash\":\"%08x\"", GetHash()),
            false);
    bb->WriteString(",\"dict\":{", false);

    bool first = true;
    for (int i = 0; i < index_;) {
        Entry *parent = &entries_[i];
        int count = parent->value;

        if (!first) {
            bb->WriteString(",", false);
        }
        first = false;

        bb->WriteString(base::Format::ToString("\"%s\":{",
                QuadToString(parent->quad)), false);

        i = i + 1;
        int start = i;

        bool firstchild = true;
        for (; i < start + count && i < index_; i++) {
            if (!firstchild) {
                bb->WriteString(",", false);
            }
            firstchild = false;
            Entry *entry = &entries_[i];
            bb->WriteString(base::Format::ToString("\"%s\":\"%x\"",
                    QuadToString(entry->quad), entry->value), false);
        }

        bb->WriteString("}", false);
    }
    bb->WriteString("}}", false);
    bb->WriteString("");
    return bb;
}

} // namespace wi
