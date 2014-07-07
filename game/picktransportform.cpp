#include "picktransportform.h"

namespace wi {
    
PickTransportForm::PickTransportForm()
{
    m_ptra = NULL;
}

bool PickTransportForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
    if (!ShellForm::Init(pfrmm, pini, idf))
        return false;
    
    m_ctrad = gtram.GetTransportDescriptions(m_atrad, kctradMax);
    LabelControl *plbl =
            (LabelControl *)GetControlPtr(kidcNoTransportsAvailable);
    if (m_ctrad != 0)
        plbl->Show(false);
    
    for (int i = kidcTransport1; i <= kidcTransport6; i++) {
        ButtonControl *pbtn = (ButtonControl *)GetControlPtr(i);
        if ((i - kidcTransport1) < m_ctrad) {
            pbtn->SetText(m_atrad[i - kidcTransport1].szName);
        } else {
            pbtn->Show(false);
        }
    }
    
    return true;
}

bool PickTransportForm::DoModal(int *pnResult, bool fAnimate, bool fShowSound)
{
    // If there is only one valid transport, don't waste the user's time by
    // making them choose it.
    
    if (m_ctrad != 1)
        return ShellForm::DoModal(pnResult, fAnimate, fShowSound);

    dword result;
    m_ptra = OpenTransport(&m_atrad[0], &result);
    return m_ptra != NULL;
}

Transport *PickTransportForm::OpenTransport(TransportDescription *ptrad,
        dword *result) {
    *result = knTransportOpenResultFail;    
    Transport *ptra = NULL;
    {
        char szT[64];
        sprintf(szT, "OPENING %s...", ptrad->szName);
        TransportWaitingUI twui(szT);
        *result = ptrad->pfnOpen(ptrad, &ptra);
    }

    const char *message = NULL;
    switch (*result) {
    case knTransportOpenResultSuccess:
        return ptra;

    default:
    case knTransportOpenResultFail: 
        message = "Failure accessing network.";
        break;

    case knTransportOpenResultNoNetwork:
        message = "Please check for network connectivity.";
        break;

    case knTransportOpenResultCantConnect: 
        message = "Could not connect to game server.";
        break;

    case knTransportOpenResultNotResponding:
        message = "Timed out waiting for Game server to respond.";
        break;

    case knTransportOpenResultProtocolMismatch: 
        message = "Please upgrade to the latest version of Hostile Takeover before playing multiplayer.";
        //message = "Please upgrade to the latest version of Hostile Takeover.";
        break;

    case knTransportOpenResultServerFull:
        message = "This server is full. Please try another server.";
        break;
    }

    if (message != NULL) {
        HtMessageBox(kfMbWhiteBorder, "Error!", message);
    }
    return NULL;
}

void PickTransportForm::OnControlSelected(word idc)
{
    if (idc == kidcCancel) {
        EndForm(idc);
        return;
    }

    dword result;
    m_ptra = OpenTransport(&m_atrad[idc - kidcTransport1], &result);
    if (m_ptra != NULL) {
        EndForm(idc);
    }
}

bool PickTransport(Transport **pptra)
{
    *pptra = NULL;
    
    PickTransportForm *pfrm = (PickTransportForm *)gpmfrmm->LoadForm(
            gpiniForms, kidfPickTransport, new PickTransportForm());
    Assert(pfrm != NULL);
    if (pfrm == NULL)
        return true;
    int idc;
    pfrm->DoModal(&idc);
    *pptra = pfrm->GetTransport();
    delete pfrm;
    
    if (gevm.IsAppStopping())
        return false;
    
    if (idc == kidcCancel)
        return true;
    
    return true;
}
    
} // namespace wi

