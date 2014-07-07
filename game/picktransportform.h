#ifndef __MPPICKTRANSPORTFORM_H__
#define __MPPICKTRANSPORTFORM_H__

#include "ht.h"

namespace wi {
    
class PickTransportForm : public ShellForm {
public:
    PickTransportForm() secMultiplayer;
    Transport *GetTransport() {
        return m_ptra;
    }
    
    // ShellForm overrides
    
    virtual bool DoModal(int *pnResult = NULL, bool fAnimate = true,
            bool fShowSound = true);
    
    // Form overrides
    
    virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf);
    virtual void OnControlSelected(word idc);
    
private:
    Transport *OpenTransport(TransportDescription *ptrad, dword *result);

    int m_ctrad;
    TransportDescription m_atrad[kctradMax];
    Transport *m_ptra;
};

} // namespace wi

#endif // __MPPICKTRANSPORTFORM_H__
