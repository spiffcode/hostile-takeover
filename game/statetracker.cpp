#include "game/statetracker.h"
#include "game/stateframe.h"
#include "mpshared/misc.h"
#include "inc/rip.h"
#include "game/ht.h"
#include <stdlib.h>
#include <string.h>

#define kcFramesMax 200

namespace wi { 

StateTracker::StateTracker(dword gameid) : gameid_(gameid) {
    frames_ = (StateFrame **)malloc(kcFramesMax * sizeof(StateFrame *));
    cFrames_ = 0;
    framesFree_ = (StateFrame **)malloc(kcFramesMax * sizeof(StateFrame *));
    cFramesFree_ = 0;
    cFramesAlloc_ = 200;
    nextblock_ = true;
}

StateTracker::~StateTracker() {
    MoveToFreeList(cFrames_);
    free(frames_);
    if (framesFree_ != NULL) {
        for (int i = 0; i < cFramesFree_; i++) {
            StateFrame *frame = framesFree_[i];
            delete frame;
        }
        free(framesFree_);
    }
}

StateFrame *StateTracker::AddFrame(long cUpdates) {
    // Move one frame to the free list if needed

    if (cFrames_ >= cFramesAlloc_) {
        MoveToFreeList(1);
    }

    // Alloc from the free list, otherwise alloc.

    StateFrame *frame = NULL;
    if (cFramesFree_ != 0) {
        frame = framesFree_[cFramesFree_ - 1];
        cFramesFree_--;
    } else {
        frame = new StateFrame(this);
    }
    if (frame == NULL) {
        return NULL;
    }
    frame->Init(cUpdates, nextblock_);
    nextblock_ = false;

    //Trace("add frame: %d block: %s", frame->updates(),
    //        frame->block() ? "yes" : "no");

    // Add to the alloced list

    frames_[cFrames_] = frame;
    cFrames_++;
    return frame;
}

void StateTracker::ExpireFrames(long cUpdates) {
    //Trace("ExpireFrames: update %d", cUpdates);

    int i;
    for (i = 0; i < cFrames_; i++) {
        // Don't expire cUpdates even though all clients have hit it,
        // because the client may not be at the next update yet, and it'll
        // need that frame to calculate the hash for the update result.
        StateFrame *frame = frames_[i];
        if (frame->updates() >= cUpdates) {
            break;
        }
        //Trace("expiring frame: %d, block: %s", frame->updates(),
        //        frame->block() ? "yes" : "no");
    }
    MoveToFreeList(i);
}

void StateTracker::MoveToFreeList(int count) {
    if (count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        StateFrame *frame = frames_[i];
        cFrames_--;
        framesFree_[cFramesFree_] = frame;
        cFramesFree_++;
    }

    memmove(&frames_[0], &frames_[count], ELEMENTSIZE(frames_) * cFrames_);
}

base::ByteBuffer *StateTracker::ToJson() {
    base::ByteBuffer *bb = new base::ByteBuffer(4096);
    if (bb == NULL) {
        return NULL;
    }

    bb->WriteString("{", false);
    bb->WriteString(base::Format::ToString("\"gameid\":\"%x\",", gameid_),
            false);

    for (int i = 0; i < cFrames_; i++) {
        StateFrame *frame = frames_[i];
        if (i != 0) {
            bb->WriteString(",", false);
        }
        bb->WriteString(base::Format::ToString("\"frame%d\":", i), false);
        base::ByteBuffer *bbFrame = frame->ToJson();
        if (bbFrame != NULL && bbFrame->Length() > 0) {
            bb->WriteBytes(bbFrame->Data(), bbFrame->Length() - 1);
        }
        delete bbFrame;
    }
    bb->WriteString("}", false);

    // Zero terminate
    bb->WriteString("");
    return bb;
}

dword StateTracker::GetHash() {
    // Calc the hash from the last "block" frame forward, since hashes
    // between block updates are what is compared.

    dword h = 0;
    for (int i = cFrames_ - 1; i >= 0; i--) {
        StateFrame *frame = frames_[i];
        dword t = frame->GetHash();
        h = (h << (h & 3)) ^ t;
        if (frame->block()) {
            //Trace("GetHash: start update %d", frame->updates());
            break;
        }
    }
    return h;
}

} // namespace wi
