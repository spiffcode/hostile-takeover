#ifndef __IDS_H__
#define __IDS_H__

namespace wi {

const int kidmGameTimer = 1; 
const int kidmPreGameTimer = 2;

STARTLABEL(MsgLabels)
    LABEL(kidmGameTimer)
    LABEL(kidmPreGameTimer)
ENDLABEL(MsgLabels)

} // namespace wi

#endif // __IDS_H__
