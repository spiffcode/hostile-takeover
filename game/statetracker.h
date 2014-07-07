#ifndef __STATETRACKER_H__
#define __STATETRACKER_H__

#include "inc/basictypes.h"
#include "mpshared/mpht.h"
#include "base/bytebuffer.h"

namespace wi {

class StateFrame;
class StateTracker {
public:
    StateTracker(dword gameid);
    ~StateTracker();

    StateFrame *AddFrame(long cUpdates);
    void ExpireFrames(long cUpdates);
    base::ByteBuffer *ToJson();
    dword GetHash();
    void SetNextBlock() { nextblock_ = true; }

private:
    void MoveToFreeList(int count);

    StateFrame **frames_;
    int cFrames_;
    StateFrame **framesFree_;
    int cFramesFree_;
    int cFramesAlloc_;
    dword gameid_;
    bool nextblock_;
};

} // namespace wi

#endif // __STATETRACKER_H__
