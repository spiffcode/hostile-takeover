#include "game/ht.h"
#include "game/Multiplayer.h"

namespace wi {
    
Transport *gptra;
char gszUsername[kcbPlayerName];
char gszPassword[kcbPlayerName];
char gszToken[kcbTokenMax];
bool gfAnonymous;

// Waiting UI helper

TransportWaitingUI::TransportWaitingUI(int nWaitStr, bool fShow)
{
    m_psz = "";
    switch (nWaitStr) {
    case knWaitStrConnectToHost:
        m_psz = "CONTACTING HOST...";
        break;

    case knWaitStrClientDisconnecting:
        m_psz = "CLIENT DISCONNECTING...";
        break;

    case knWaitStrDisconnectingClient:
        m_psz = "DISCONNECTING CLIENT...";
        break;

    case knWaitStrDisconnectingClients:
        m_psz = "DISCONNECTING CLIENTS...";
        break;

    case knWaitStrClosingTransport:
        m_psz = "CLOSING TRANSPORT...";
        break;

    case knWaitStrBeginGameSearch:
        m_psz = "BEGIN GAME SEARCH...";
        break;

    case knWaitStrAdvertisingGame:
        m_psz = "ADVERTISING GAME...";
        break;
    }
    m_pfrm = NULL;
    if (fShow) {
        Show();
    }
}

TransportWaitingUI::TransportWaitingUI(char *psz, bool fShow)
{
    m_pfrm = NULL;
    m_psz = psz;
    if (fShow) {
        Show();
    }
}

TransportWaitingUI::~TransportWaitingUI()
{
    Hide();
}

void TransportWaitingUI::Show()
{
    if (m_psz != NULL) {
        m_pfrm = gpmfrmm->LoadForm(gpiniForms, kidfWaiting, new WaitForm(m_psz, false));
        m_pfrm->SetFlags(m_pfrm->GetFlags() & ~kfFrmTopMost);
        gpmfrmm->DrawFrame(false);
    }
}

void TransportWaitingUI::Hide()
{
    delete m_pfrm;
    m_pfrm = NULL;
    gpmfrmm->DrawFrame(false);
}
        
} // namespace wi
