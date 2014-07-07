#ifndef __MIXER_H__
#define __MIXER_H__

#include "basictypes.h"

namespace wi {
    
struct Channel // chnl
{
    byte *pb;
    byte *pbEnd;
    byte bSampleLast;
    int nStepIndex;
};

void MixChannels(Channel *achnl, int cchnl, byte *pb, word cb);

} // namespace wi

#endif // __MIXER_H__
