#include "game/ht.h"
#include "mpshared/netmessage.h"
#include "game/wistrings.h"
#include "game/statetracker.h"
#include "game/stateframe.h"
#include "game/uploader.h"
#include "game/serviceurls.h"
#include "game/chatter.h"
#include "game/gameform.h"
#include "base/sigslot.h"
#include <string>

namespace wi {

// Global representing the contiguous rect opaquing the map, in map relative tile coords

TRect *gptrcMapOpaque;
TRect gtrcMapOpaque;

// Handy dandy global flags used for enabling various test features

#ifdef STATS_DISPLAY
int gcBitmapsDrawn;
extern bool gfShowStats;
int gcbCommandsQueued;
#endif
int gcMessagesPerUpdate;

extern bool gfShowFPS;
extern bool gfSuspendUpdates;
extern bool gfSingleStep;

short gcPaint = 0;
#ifdef STATS_DISPLAY
static short s_cUpdateRects = 0;
#endif
static fix s_cfxFPS = 0;
static long s_tLastFPSUpdate = 0;

// Misc SimUI management variables

bool gfDragSelecting;
WPoint s_wptSelect1, s_wptSelect2;
WRect gwrcSelection;
WPoint s_awptSelection[250];
int s_cwptSelection;
bool gfLassoSelection = false;
#ifdef STATS_DISPLAY
long gcPathScoresCalced = 0;
#endif

bool PtInPolygon(WPoint *awpt, int cwpt, WCoord wx, WCoord wy) secSimUIForm;

//
// Form used for waiting
//

WaitForm::WaitForm(char *pszTitle, bool fFullScreen)
{
	m_pszTitle = pszTitle;
	m_fFullScreen = fFullScreen;
}

bool WaitForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	if (!Form::Init(pfrmm, pini, idf))
		return false;

	// Size the form to be full screen

	DibBitmap *pbm = m_pfrmm->GetDib();
	Size siz;
	pbm->GetSize(&siz);

	// Remember old bounds

	m_rcOrig = m_rc;

	// Set text if passed

	if (m_pszTitle != NULL) {
		LabelControl *plbl = (LabelControl *)GetControlPtr(kidcTitle);
		plbl->SetText(m_pszTitle);
	}

	int xNew = (siz.cx - m_rc.Width()) / 2;
	int yNew = (siz.cy - m_rc.Height()) / 2;

	if (m_fFullScreen) {
		// Reposition the controls

		for (int n = 0; n < m_cctl; n++) {
			Control *pctl = m_apctl[n];
			Rect rcCtl;
			pctl->GetRect(&rcCtl);
			int xNewCtl = rcCtl.left + xNew;
			int yNewCtl = rcCtl.top + yNew;
			pctl->SetPosition(xNewCtl, yNewCtl);
		}
		Rect rcNew;
		rcNew.Set(0, 0, siz.cx, siz.cy);
		SetRect(&rcNew);
	}

	return true;
}

void WaitForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	Form::OnPaintBackground(pbm, pupd);

	DrawBorder(pbm, &m_rcOrig, gcxyBorder, GetColor(kiclrCyanSideFirst), pupd);
}

// This is a hack so that sigslot.h doesn't need to be included from ht.h.

class SlotHack : public base::has_slots<> {
public:
    void Connect(SimUIForm *sui, Chatter *chatter) {
        if (chatter != NULL) {
            chatter->SignalOnBlink.connect(this, &SlotHack::OnChatButtonBlink);
            chatter->SignalOnPlayers.connect(this, &SlotHack::OnPlayersButton);
        }
        sui_ = sui;
        chatter_ = chatter;
    }
    void Disconnect() {
        if (chatter_ != NULL) {
            chatter_->SignalOnBlink.disconnect(this);
            chatter_->SignalOnPlayers.disconnect(this);
        }
    }
    void OnChatButtonBlink(bool fOn) {
        if (sui_ != NULL) {
            sui_->OnChatButtonBlink(fOn);
        }
    }
    void OnPlayersButton() {
        if (sui_ != NULL) {
            sui_->OnPlayersButton();
        }
    }
    SimUIForm *sui_;
    Chatter *chatter_;
};
SlotHack g_slothack;

//
// SimUIForm implementation
//

bool SimUIForm::s_fReadyForPaint;
    
SimUIForm::SimUIForm(word wfRole, dword gameid, Chatter *chatter)
{
	s_fReadyForPaint = false;
	gfDragSelecting = false;
	s_cwptSelection = 0;
    
	m_fTimerAdded = false;

	m_wfRole = wfRole;
	m_pcmdqServer = NULL;
	m_pfrmWaitingForAllPlayers = NULL;
	m_tLastCommunication = 0;

	m_nStateMoveTarget = 0;

    m_ppenh = NULL;
    gfLassoSelection ? SetUIType(kuitStylus) : SetUIType(kuitFinger);
    m_punmrFirst = NULL;

    m_gameid = gameid;
    m_pchatter = chatter;
    if (m_pchatter != NULL) {
        m_pchatter->SetChatTitle("Game Chat");
    }
#ifdef TRACKSTATE
    m_ptracker = new StateTracker(m_gameid);
    m_fSyncError = false;
#endif

    g_slothack.Connect(this, chatter);
}

SimUIForm::~SimUIForm()
{
    delete m_pfrmWaitingForAllPlayers;
    delete m_pfrmLag;
	if (m_fTimerAdded)
		gtimm.RemoveTimer(this);
	if (m_wfRole & kfRoleMultiplayer) {
		gptra->SetCallback(NULL);
        gptra->SetGameCallback(NULL);
	}
		
    if (m_pcmdqServer != NULL) {
        m_pcmdqServer->Exit();
        delete m_pcmdqServer;
    }

    delete m_ppenh;

#ifdef TRACKSTATE
    delete m_ptracker;
#endif

    g_slothack.Disconnect();
    if (m_pchatter != NULL) {
        m_pchatter->HideChat();
    }
}

void SimUIForm::SetUIType(UIType uit)
{
    switch (uit) {
    case kuitFinger:
        delete m_ppenh;
        m_ppenh = new FingerHandler(this);
        break;

    case kuitStylus:
        delete m_ppenh;
        m_ppenh = new StylusHandler(this);
        break;
    }
}

void SimUIForm::OnChatButtonBlink(bool fOn) {
// Annoying since the button is visible during observe, and you can already
// see the chat on screen.
#if 0
    if (gpplrLocal->GetFlags() & kfPlrObserver) {
        GetControlPtr(kidcChat)->Show(fOn);
    }
#endif
}

void SimUIForm::OnPlayersButton() {
    std::string s = GameForm::GetPlayersString();
    if (m_pchatter != NULL) {
        m_pchatter->AddChat("", s.c_str(), true);
    }
}

static SimUIForm *gpsui;

const long kctStartTimeout = 1500;	// 15 seconds

bool SimUIForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	gpsui = this;

	m_tStartTimeout = HostGetTickCount() + kctStartTimeout;

	// Init move target

	m_nStateMoveTarget = 0;
	m_aniMoveTarget.Init(g_panidMoveTarget);

	if (m_wfRole & kfRoleMultiplayer) {
		m_pcmdqServer = new CommandQueue();
		Assert(m_pcmdqServer != NULL, "out of memory!");
		if (m_pcmdqServer == NULL)
			return false;
		if (!m_pcmdqServer->Init(kcmsgCommandQueueMax)) {
			return false;
        }

		// Transport callback

		gptra->SetCallback(this);
        gptra->SetGameCallback(this);

        // Create a big form to cover everything and show "Waiting for all
        // players"

		m_pfrmWaitingForAllPlayers = gpmfrmm->LoadForm(gpiniForms, kidfWaiting, new WaitForm("WAITING FOR ALL PLAYERS...", true));
		if (m_pfrmWaitingForAllPlayers != NULL) {
			gpmfrmm->DrawFrame(false);
		}
	}

    // New multiplayer design: client maintains its own update timer. 
    // Client messages are sent "just in time" by server, scheduled to be
    // executed on a given update. Clients know what update to expect the
    // next message from the server. Every so often the client sends feedback
    // to the server on latency (time of actual update - time message received)
    // so that the server can tune when client messages are sent, in real
    // time. This approach works better over the internet. The previous
    // approach worked fine over local networks.

    gtimm.AddTimer(this, gtGameSpeed);
    m_fTimerAdded = true;

    while (m_punmrFirst != NULL) {
        UpdateNetMessageRecord *punmrT = m_punmrFirst;
        m_punmrFirst = punmrT->punmrNext;
        delete punmrT->punm;
        delete punmrT;
    }

	// Let the base Form initialize the controls

	bool fSuccess = Form::Init(pfrmm, pini, idf);

	if (m_wfRole & kfRoleMultiplayer) {
        m_cUpdatesBlock = 0;
        m_msUpdatesBlock = 0;
        SendUpdateResult(0, 0, 0);
	}

	// Size the rect to the dib

	DibBitmap *pbmSim = m_pfrmm->GetDib();
	Size sizSim;
	pbmSim->GetSize(&sizSim);
	Rect rcSim;
	rcSim.Set(0, 0, sizSim.cx, sizSim.cy);
	SetRect(&rcSim);

	// Set control positions
	
	Control *pctlT;
	Rect rcT;

	// Objective is at upper-left

	pctlT = GetControlPtr(kidcObjective);
	Assert(pctlT != NULL);
	pctlT->SetPosition(0, 0);

	// Countdown timer is at upper-right

	pctlT = GetControlPtr(kidcCountdown);
	Assert(pctlT != NULL);
	pctlT->GetRect(&rcT);
	pctlT->SetPosition(sizSim.cx - rcT.Width(), 0);

	// FPS is at upper right, down 1 line to leave room for countdown timer

	pctlT = GetControlPtr(kidcFps);
	Assert(pctlT != NULL);
	pctlT->GetRect(&rcT);
	pctlT->SetPosition(sizSim.cx - rcT.Width(), rcT.Height());

    // Chat is at upper right. Hide chat button for now. It only appears
    // in observer mode.

    pctlT = GetControlPtr(kidcChat);
    pctlT->GetRect(&rcT);
    pctlT->SetPosition(sizSim.cx - rcT.Width() - rcT.top, rcT.top);
    pctlT->Show(false);

	// Alert is at bottom left

	pctlT = GetControlPtr(kidcAlert);
	Assert(pctlT != NULL);
	pctlT->GetRect(&rcT);
	pctlT->SetPosition(0, sizSim.cy - rcT.Height());

    // Reset lag state

    m_pfrmLag = NULL;
    m_pidLagging = kpidNeutral;

	// Show / hide these controls

	GetControlPtr(kidcFps)->Show(gfShowFPS);
	GetControlPtr(kidcAlert)->Show(false);
	Assert(gsim.GetLevel() != NULL);
	Assert(gsim.GetLevel()->GetTriggerMgr() != NULL);
	GetControlPtr(kidcCountdown)->Show(gsim.GetLevel()->GetTriggerMgr()->GetCountdownTimer()->GetFlags() & kfCtVisibleAtStart);

    // This form wants pen2 events
    m_wf |= kfFrmDemandPen2;

	return fSuccess;
}

//
// IGameCallback implementation
//

void SimUIForm::OnNetMessage(NetMessage **ppnm)
{
#ifdef TRACKSTATE
    // If there has been a sync error, do not respond to any messages
    if (m_fSyncError) {
        return;
    }
#endif

    // Setting *ppnm = NULL allows the callee to keep the NetMessage
    // rather than the caller deleting it
    NetMessage  *pnm = *ppnm;

	// Track the last time communication was received on this device

	m_tLastCommunication = HostGetTickCount();

	switch (pnm->nmid) {
	case knmidScPlayerDisconnect:
        {
            // This must run at the same point on all clients, since
            // it sets the kfPlrComputer flag.
            PlayerDisconnectNetMessage *pdm = (PlayerDisconnectNetMessage*)pnm;
            OnPlayerDisconnectNotify(pdm->pid, pdm->nReason);

            // Asynchronously, check for game over
            Event evt;
            evt.eType = checkGameOverEvent;
            evt.dw = pdm->pid;
            gevm.PostEvent(&evt);
        }
        break;

	case knmidScLagNotify:
		// This player is being notified by the server of another player
        // that is lagging.

		OnLagNotify(((LagNotifyNetMessage *)pnm)->pidLagging,
                ((LagNotifyNetMessage *)pnm)->cSeconds);
		break;

	case knmidScUpdate:
        QueueUpdateMessage((UpdateNetMessage *)*ppnm);

        // Tell the caller to not delete
        *ppnm = NULL;
        break;

    case knmidScCheckWin:
        OnCheckWin(((CheckWinNetMessage *)pnm)->pid);
        break;

#ifdef TRACKSTATE
    case knmidScSyncError:
        ReportSyncError();
        break;
#endif
	}
}

void SimUIForm::OnReceiveChat(const char *player, const char *chat) {
    ShowAlert(base::Format::ToString("%s: %s", player, chat));
    if (m_pchatter != NULL) {
        m_pchatter->AddChat(player, chat, false);
    }
}

void SimUIForm::OnGameDisconnect()
{
    HtMessageBox(kfMbWhiteBorder, "Network", 
            "You've been disconnected from this game. Press OK to continue.");

    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.eType = gameOverEvent;
    evt.dw = knGoAbortLevel;
    gevm.PostEvent(&evt);
}

void SimUIForm::OnStatusUpdate(char *pszStatus)
{
    // nothing to do here
}

void SimUIForm::OnConnectionClose()
{
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

std::string s_message;

void SimUIForm::OnShowMessage(const char *message)
{
    s_message = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool SimUIForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType == connectionCloseEvent) {
        // My connection to the server broke!

        if (m_pchatter != NULL) {
            m_pchatter->HideChat();
        }
        HtMessageBox(kfMbWhiteBorder, "Comm Problem",
                "Your connection to the server dropped.");

        if ((gpplrLocal->GetFlags() & kfPlrObserver) == 0) {
            gpplrLocal->ShowObjectives(ksoWinSummary, false, true);
        }

        Event evt;
        memset(&evt, 0, sizeof(evt));
        evt.eType = gameOverEvent;
        evt.dw = knGoAbortLevel;
        gevm.PostEvent(&evt);
        return true;
    }

    if (pevt->eType == showMessageEvent) {
        if (m_pchatter != NULL) {
            m_pchatter->HideChat();
        }
        HtMessageBox(kfMbWhiteBorder, "Server Message", s_message.c_str());
        s_message = "";
        return true;
    }
    return false;
}

#ifdef TRACKSTATE
void SimUIForm::ReportSyncError()
{
    // A sync error has occured, must remember this so no state changes
    // from here forward
    m_fSyncError = true;

    // Remove the game timer
    if (m_fTimerAdded) {
        gtimm.RemoveTimer(this);
        m_fTimerAdded = false;
    }

    // Alert the user to what is happening
    HtMessageBox(kfMbWhiteBorder, "Sync Error",
            "This mission has experienced a sync error and must end. "
            "Please notify the mission author.");
    gpmfrmm->DrawFrame(true);

    // Upload the state tracker state
    base::ByteBuffer *pbb = m_ptracker->ToJson(); 
    const char *url = base::Format::ToString("%s?gameid=%d&pid=%d",
            kszSyncErrorUploadUrl, m_gameid, gpplrLocal->GetId());
    UploadByteBuffer(gphttp, pbb, url);

    // End the game
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.eType = gameOverEvent;
    evt.dw = knGoAbortLevel;
    gevm.PostEvent(&evt);
}
#endif

void SimUIForm::QueueUpdateMessage(UpdateNetMessage *punm) {
#ifdef TRACKSTATE
    m_ptracker->ExpireFrames(punm->cUpdatesSync);
#endif

    // Remember the UpdateNetMessage

    UpdateNetMessageRecord *punmr = new UpdateNetMessageRecord;
    punmr->msReceived = HostGetMillisecondCount();
    punmr->punm = punm;
    punmr->punmrNext = NULL;

    // Put it on the end of the list

    UpdateNetMessageRecord **ppunmr = &m_punmrFirst;
    while ((*ppunmr) != NULL) {
        ppunmr = &(*ppunmr)->punmrNext;
    }
    *ppunmr = punmr;

    // If the client is blocked waiting for this update, process it
    // right away.

    bool fUpdateNow = false;
    if (gsim.GetUpdateCount() + 1 == m_cUpdatesBlock) {
        fUpdateNow = true;
    }

    // It's possible the client gets behind. For example, if the client
    // is blocked on network i/o and then sudden 5 updates come in,
    // running the timer as normal would take too long for it to catch up with
    // the server. So, process the commands here.

    if (m_punmrFirst != NULL && m_punmrFirst->punmrNext != NULL) {
        fUpdateNow = true;
    }

    // Running the simulation requires running from the event loop,
    // because of the state set, and state checked, from within
    // the event loop.

    if (fUpdateNow) {
        Event evt;
        evt.eType = runUpdatesNowEvent;
        evt.idf = m_idf;
        gevm.PostEvent(&evt);
    }
}

void SimUIForm::RunUpdatesNow()
{
    // If blocked waiting for an update and there is one, process it
    // and reset the timer

    if (m_punmrFirst != NULL && gsim.GetUpdateCount() + 1 == m_cUpdatesBlock) {
        gtimm.RemoveTimer(this);
        OnTimer(0);
        gtimm.AddTimer(this, gtGameSpeed);
        ggame.UpdateTriggers();
    }

    // If there are more than one updates available, process them to
    // catch up.

    while (m_punmrFirst != NULL && m_punmrFirst->punmrNext != NULL) {
        gtimm.RemoveTimer(this);
        OnTimer(0);
        gtimm.AddTimer(this, gtGameSpeed);
        ggame.UpdateTriggers();
    }
}

bool SimUIForm::ProcessUpdateMessage(CommandQueue *pcmdq)
{
    // This should not happen
    if (gsim.GetUpdateCount() + 1 > m_cUpdatesBlock) {
        Trace("Broken!");
        return false;
    }

    // Commands are processed at block points. If not at block point yet,
    // continue the simulation.

    if (gsim.GetUpdateCount() + 1 < m_cUpdatesBlock) {
        pcmdq->Clear();
        return true;
    }

    // At cUpdatesBlock for the first time? If so remember the timestamp
    // for latency calc purposes

    if (m_msUpdatesBlock == 0) {
        m_msUpdatesBlock = HostGetMillisecondCount();
    }

    // No messages to process? Block until they come. Users don't like
    // blocking since it results in on screen stutter, perceived as lag.

    if (m_punmrFirst == NULL) {
        Trace("BLOCK!");
        return false;
    }

    // Take the next set of commands and queue them up

    UpdateNetMessageRecord *punmr = m_punmrFirst;
    m_punmrFirst = punmr->punmrNext;
    UpdateNetMessage *punm = punmr->punm;
    pcmdq->Clear();
    for (int i = 0; i < punm->cmsgCommands; i++) {
        pcmdq->Enqueue(&punm->amsgCommands[i]);
    }

    // Send the update result with latency info
    // Don't send if cUpdatesBlock_ == 0. This is a special case
    // that is handled elsewhere (search for calls to SendUpdatesResult).

    int cmsLatency = (int)(m_msUpdatesBlock - punmr->msReceived);

#ifdef TRACKSTATE
    dword hash = m_ptracker->GetHash();
    Trace("GetHash: end update %d", gsim.GetUpdateCount());
    m_ptracker->SetNextBlock();
#else
    dword hash = 0;
#endif

#if 0
    // Adjust the timer trigger, without changing the rate. If updates are
    // coming a little too soon, wait a bit before triggering next. If updates
    // are a bit old, trigger sooner next time. This maintains "just in time"
    // update delivery.
    if (cmsLatency <= 0) {
        gtimm.BoostTimer(this, 1);
    }
    if (cmsLatency >= 200) {
        gtimm.BoostTimer(this, -1);
    }
#endif

    if (m_cUpdatesBlock != 0) {
        SendUpdateResult(m_cUpdatesBlock, cmsLatency, hash);
    }

    // This is the next point to block. Ideally the server sends another
    // UpdateNetMessage before the clients get to this point. This is how
    // stuttering is reduced, which users perceive as lag.

    m_cUpdatesBlock = punm->cUpdatesBlock;
    m_msUpdatesBlock = 0;

    delete punm;
    delete punmr;

    // Ok to update the simulation
    return true;
}

void SimUIForm::SendUpdateResult(int cUpdatesBlock, int cmsLatency,
        dword hash)
{
	UpdateResultNetMessage urnm;
	urnm.ur.cUpdatesBlock = cUpdatesBlock;
    urnm.ur.hash = hash;
    urnm.ur.cmsLatency = cmsLatency;
    gptra->SendNetMessage(&urnm);
}

void SimUIForm::OnTimer(long tCurrent) {
#ifdef TRACKSTATE
    // If there has been a sync error, do nothing
    if (m_fSyncError) {
        return;
    }
#endif

	if (gfSuspendUpdates && !gfSingleStep) {
		gevm.SetRedrawFlags(kfRedrawDirty);
		return;
	}
	gfSingleStep = false;
	if (gsim.IsPaused()) {
		return;
    }

// TUNE:
#define kctLocalLag 200
	if ((m_wfRole & (kfRoleMultiplayer | kfRoleServer)) == kfRoleMultiplayer) {
		if (m_tLastCommunication != 0) {
            long tCurrent = HostGetTickCount();
            if (tCurrent - m_tLastCommunication >= kctLocalLag) {
                ShowAlert("Network lag");

                // Request redraw. When the client is waiting on the server it
                // won't redraw unless asked to.

                gevm.SetRedrawFlags(kfRedrawDirty);
            }
        }
	}

    // Send queued commands to server whenever there are any

    if ((m_wfRole & (kfRoleMultiplayer | kfRoleServer)) == kfRoleMultiplayer) {

        int cmsg = gcmdq.GetCount();
        if (cmsg != 0) {
            int cbcc = sizeof(ClientCommandsNetMessage) +
                    ((cmsg - 1) * sizeof(Message));
            ClientCommandsNetMessage *pcc =
                    (ClientCommandsNetMessage *)new byte[cbcc];
            pcc->nmid = knmidCsClientCommands;
            pcc->cb = cbcc;
            pcc->cmsgCommands = cmsg;
            memcpy(&pcc->amsgCommands, gcmdq.GetFirst(),
                    cmsg * sizeof(Message));
            gptra->SendNetMessage(pcc);
            delete[] pcc;
        }
        gcmdq.Clear();
    }

    if ((m_wfRole & (kfRoleMultiplayer | kfRoleServer)) == kfRoleMultiplayer) {
        // Process the next update message
        if (ProcessUpdateMessage(m_pcmdqServer)) {
#ifdef TRACKSTATE
            BeginTrackState();
#endif
            gsim.Update(m_pcmdqServer);
#ifdef TRACKSTATE
            EndTrackState();
#endif
        }
    } else {
        // Update (i.e., 'step') the Simulation

        gsim.Update(&gcmdq);
    }

    // Update UI

	Update();

#ifdef DEBUG_HELPERS
    extern void UpdateLog();
    UpdateLog();
#endif

    // Check to see if something needs to be done with the lag form

    CheckLagForm();
}

#ifdef TRACKSTATE
StateFrame *gpframeCurrent;

void SimUIForm::BeginTrackState()
{
    long cUpdates = gsim.GetUpdateCount() + 1;
    StateFrame *frame = m_ptracker->AddFrame(cUpdates);
    int i = frame->AddCountedValue('SMUI');
    frame->AddValue('BLCK', (dword)m_cUpdatesBlock, i);

    // Set this so it can be used during gsim.Update()
    gpframeCurrent = frame;
}

void SimUIForm::EndTrackState()
{
    gsim.TrackState(gpframeCurrent);
    gpframeCurrent = NULL;
}
#endif

bool SimUIForm::EventProc(Event *pevt)
{
	switch (pevt->eType) {
    case runUpdatesNowEvent:
        RunUpdatesNow();
        break;
	}

	return Form::EventProc(pevt);
}

void SimUIForm::CheckMultiplayerGameOver(Pid pidLeft) {
    if (!gfMultiplayer) {
        return;
    }

    // This should be handled by triggers, but for now it is
    // hamfistedly hardwired here.
    //
    // If transitioning to a single player (or team) then assume this player
    // has won. This isn't always the right choice, that is why it should
    // be handled by triggers. But there are a lot of maps made that
    // don't handle this case, and there is not enough conditions
    // available currently to handle resigning / leaving the game with
    // triggers.

    if (gplrm.DetectTransitionToSingleHumanTeam(pidLeft)) {

        // If this player isn't an observer it means they haven't
        // officially won/lost yet. Let them know the game is over and set
        // them to be an Observer.

        if ((gpplrLocal->GetFlags() &
                    (kfPlrObserver | kfPlrSummaryShown)) == 0) {
            gpplrLocal->ShowObjectives(ksoWinSummary, false, false);

            if (!ggame.AskObserveGame()) {
                Event evt;
                memset(&evt, 0, sizeof(evt));
                evt.eType = gameOverEvent;
                evt.dw = knGoAbortLevel;
                gevm.PostEvent(&evt);
            }
        } else {
            // Some other player is victorious. Show alert.
            // There is only one non-observer, non-computer player
            // left.

            Player *pplr = gplrm.GetNextHumanPlayer(NULL);
            if (pplr != NULL) {
                char szT[100];
                sprintf(szT, "%s is victorious!", pplr->GetName());
                ShowAlert(szT);
                if (m_pchatter != NULL) {
                    m_pchatter->AddChat("", szT, true);
                }
            }
        }
    }
}

void SimUIForm::SetObserving()
{
    gpplrLocal->SetObjective("< Observing >");
    gsim.GetLevel()->GetFogMap()->RevealAll(gpupdSim);
    GetControlPtr(kidcFps)->Show(false);
    GetControlPtr(kidcCountdown)->Show(false);
    GetControlPtr(kidcChat)->Show(true);
}

void SimUIForm::OnCheckWin(Pid pid) {
    if (gpplrLocal == NULL || gptra == NULL) {
        return;
    }

#if 0
// Disable challenges for now. It needs to be tested against mission
// scripting, and it needs to be updated to handle allies. Because challenge
// is disabled, it means the server believes the first client that says it
// won. So some clever hacker can cause mayhem.

    // A client has claimed win. In order to be valid, the local player
    // must be marked either winner (if == this pid) or a loser.
   
    bool fValid = true; 
    if (gpplrLocal->GetId() == pid) {
        if ((gpplrLocal->GetFlags() & (kfPlrWinner | kfPlrLoser)) !=
                kfPlrWinner) {
            fValid = false;
        }
    } else {
        if ((gpplrLocal->GetFlags() & (kfPlrWinner | kfPlrLoser)) !=
                kfPlrLoser) {
            fValid = false;
        }
    }
    if (!fValid) {
        ChallengeWinNetMessage cwnm;
        gptra->SendNetMessage(&cwnm);
    }
#endif

    // Assume this player won for purposes of chat system message
    Player *pplr = gplrm.GetPlayerFromPid(pid);
    if (pplr != NULL && m_pchatter != NULL) {
        m_pchatter->AddChat(NULL, base::Format::ToString(
                "%s has won this game.", pplr->GetName()), true);
    }
}

void SimUIForm::OnPlayerDisconnectNotify(Pid pid, int nReason)
{
    // We've been notified by the server of a player disconnecting. Display
    // alert

	Player *pplr = gplrm.GetPlayerFromPid(pid);
	if (pplr != NULL) {
		pplr->SetFlags(pplr->GetFlags() | kfPlrComputer);

        // Set observer flag so that it is possible to identify the observing
        // players later. Don't set it for the local player, since this
        // flag is heavily overloaded and used to determine whether to show
        // win/lose summaries and AskObserve form, at which point it will be
        // set there. Clearly the win/lose paths need re-writing rather than
        // patching.
        if (pplr != gpplrLocal) {
            pplr->SetFlags(pplr->GetFlags() | kfPlrObserver);
        }

		char szT[128];
		char *pszReason = NULL;
        char *pszChat = NULL;
		switch (nReason) {
        case knDisconnectReasonLeftGame:
            if (!pplr->GetLeftGame()) {
                pplr->SetLeftGame();
                pszReason = "left the game";
                pszChat = ".";
            }
            break;

		case knDisconnectReasonResign:
			pszReason = "resigned";
            pszChat = ", and is now observing.";
			break;

		case knDisconnectReasonKilled:
            pszReason = "kicked by another player";
            pszChat = ", and left the game.";
			break;

		case knDisconnectReasonAbnormal:
			pszReason = "dropped out";
            pszChat = ", and left the game.";
			break;

		case knDisconnectReasonNotResponding:
			pszReason = "disconnected - not responding";
            pszChat = ", and left the game.";
			break;
		}

        if (pszReason != NULL) {
            sprintf(szT, "%s %s", pplr->GetName(), pszReason);
            ShowAlert(szT);
        }
        if (pszChat != NULL && m_pchatter != NULL) {
            m_pchatter->AddChat(NULL, base::Format::ToString("%s%s", szT,
                    pszChat), true);
        }
	}
}

void SimUIForm::OnLagNotify(Pid pidLagging, int cSeconds)
{
    // First thing's first, acknowledge this to the server,
    // so the server knows this client is alive.
    
    LagAcknowledgeNetMessage lanm;
    gptra->SendNetMessage(&lanm);

    // pidLagging is kpidNeutral if there are no laggards,
    // which causes GetPlayerFromPid() to return NULL.

	Player *pplr = gplrm.GetPlayerFromPid(pidLagging);
    if (pplr == NULL) {
        ShowAlert("");
        if (m_pfrmLag != NULL) {
            m_pfrmLag->Show(false);
            delete m_pfrmLag;
            m_pfrmLag = NULL;
        }
        return;
    }

	// We've been notified by the server of this player lagging. Display alert

    ShowAlert(base::Format::ToString("%s lagging (%ds)", pplr->GetName(),
            cSeconds));

    // If cSeconds is not 0, hide the form. It only comes up at 0.

    if (cSeconds != 0) {
        if (m_pfrmLag != NULL) {
            m_pfrmLag->Show(false);
            delete m_pfrmLag;
            m_pfrmLag = NULL;
        }
        return;
    }

    // cSeconds is 0, then show the lag form. First see if it is for a
    // different player. If so, hide it first.

    if (m_pfrmLag != NULL) {
        // If same pid is lagging, nothing to do; keep the form up

        if (m_pidLagging == pidLagging) {
            return;
        }

        // A different pid is lagging, yet the form is up. Delete the current
        // one first.

        m_pfrmLag->Show(false);
        delete m_pfrmLag;
        m_pfrmLag = NULL;
    }

    // Show the lagging form, modeless, so that it can be hidden and reshown
    // at will. Check during update if in EndForm state, and handle
    // appropriately.

    m_pidLagging = pidLagging;
    m_pfrmLag = CreateHtMessageBox(kidfMessageBoxQuery, kfMbWhiteBorder,
            "LAG ALERT",
            base::Format::ToString(
"%s is communicating erratically and causing the game to lag. Disconnect %s?",
            pplr->GetName(), pplr->GetName()));
    m_pfrmLag->SetFlags(m_pfrmLag->GetFlags() | kfFrmDoModal);
    gsndm.PlaySfx(ksfxGuiFormShow);
    m_pfrmLag->Show(true);
}

void SimUIForm::CheckLagForm()
{
    if (m_pfrmLag == NULL || (m_pfrmLag->GetFlags() & kfFrmDoModal) != 0) {
        return;
    }

    int nResult = m_pfrmLag->GetResult();
    m_pfrmLag->Show(false);
    delete m_pfrmLag;
    m_pfrmLag = NULL;

    // Send a KillLaggingPlayerNetMessage with the result

    KillLaggingPlayerNetMessage klpnm;
    klpnm.pid = m_pidLagging;
    klpnm.fYes = (nResult == kidcOk) ? 1 : 0; 
    gptra->SendNetMessage(&klpnm);
}

void SimUIForm::Update()
{
#ifdef STRESS
	if (gfStress) {
		if (gsim.GetTickCount() > gtStressTimeout) {
            Event evt;
			memset(&evt, 0, sizeof(evt));
			evt.eType = gameOverEvent;
			evt.dw = knGoAbortLevel;
			gevm.PostEvent(&evt);
		}
	}
#endif

	// Advance the move target while it's visible

	if (m_nStateMoveTarget == 1) {
		m_aniMoveTarget.Advance(1);
		if (m_aniMoveTarget.GetFlags() & kfAniDone)
			m_nStateMoveTarget = 2;
	}

	if (m_pfrmWaitingForAllPlayers != NULL) {
		delete m_pfrmWaitingForAllPlayers;
		m_pfrmWaitingForAllPlayers = NULL;
	}

	// Have the InputUI update its displays (e.g., credits, power)

	InputUIForm *pfrmInputUI = ggame.GetInputUIForm();
	pfrmInputUI->Update();

	// Update the objective display

	LabelControl *plbl = (LabelControl *)GetControlPtr(kidcObjective);
	plbl->SetText(gpplrLocal->GetObjective());

	// Calc stats

	long tCurrent = HostGetTickCount();
	if (s_tLastFPSUpdate == 0)
		s_tLastFPSUpdate = tCurrent;
	if (tCurrent - s_tLastFPSUpdate >= 100) {
		fix32 fx1 = itofx32(gcPaint * 100);
		fix32 fx2 = itofx32(tCurrent - s_tLastFPSUpdate);
		if (fx2 == 0)
			s_cfxFPS = 0;
		else
			s_cfxFPS = (fix)divfx32(fx1, fx2);
		int cFpsWhole = fx32toi(s_cfxFPS);
		int cFpsFraction = (int)fxtoi(mulfx(fracfx(s_cfxFPS), itofx(10)));

		if (gfShowFPS) {
			char szFPS[80];
			sprintf(szFPS, "%d.%d", cFpsWhole, cFpsFraction);
			((LabelControl *)GetControlPtr(kidcFps))->SetText(szFPS);
		}

		s_tLastFPSUpdate = tCurrent;
		gcPaint = 0;

		// Set the # of pen events to accept per second. We don't need anything faster
		// than the frame rate.

		static long s_tLastPaint;
		if (s_tLastPaint == 0)
			s_tLastPaint = tCurrent;
		gevm.SetPenEventInterval((word)(tCurrent - s_tLastPaint));
		s_tLastPaint = tCurrent;
	}

	// Let the minimap decide if it should invalidate.

	gpmm->Update();

	// Everything has been updated for (at least) the first time
	// so we're ready to display it.

	if (!s_fReadyForPaint) {
		s_fReadyForPaint = true;
		InvalidateRect(NULL);
	}

	// Set redraw

	gevm.SetRedrawFlags(kfRedrawDirty);
}

void AddPointToLassoSelection(WPoint wpt)
{
	// TUNE:
	if (abs(s_awptSelection[s_cwptSelection - 1].wx - wpt.wx) >= WcFromTile16ths(4) ||
			abs(s_awptSelection[s_cwptSelection - 1].wy - wpt.wy) >= WcFromTile16ths(4)) {
		s_awptSelection[s_cwptSelection] = wpt;
		if (s_cwptSelection < (sizeof(s_awptSelection) / sizeof(WPoint)) - 1)
			s_cwptSelection++;
	}
}

void SimUIForm::OnUpdateMapInvalidate(UpdateMap *pupd, Rect *prcOpaque)
{
#ifdef DRAW_LINES
	if (gfDrawLines)
		pupd->InvalidateRect();
#endif

#ifdef DRAW_PATHS
	if (gfDrawPaths)
		pupd->InvalidateRect();
#endif

#if defined(DRAW_OCCUPIED_TILE_INDICATOR) || defined(MARK_TILE_BOUNDARIES) || defined(MARK_OCCUPIED_TILES)
	pupd->InvalidateRect();
#endif

	// Invalidate move target

	if (m_nStateMoveTarget != 0) {
		pupd->InvalidateTile(TcFromWc(m_wxMoveTarget), TcFromWc(m_wyMoveTarget));
		if (m_nStateMoveTarget == 2)
			m_nStateMoveTarget = 0;
	}

	// Handle selection invalidation

	InvalidateDragSelection();

	// Calculate the current opaquing rect for the map.
	// HACK: Store in a global since various places need it, such as when a gob gets removed or when
	// galaxite gets invalidated.

	Rect rcT;
	bool fEmpty = prcOpaque->IsEmpty();
	if (fEmpty) {
// Probably not worth it
#if 0
		// Use the minimap since it covers the map

		Control *pctl = GetControlPtr(kidcMiniMap);
		if (pctl->GetFlags() & kfCtlVisible) {
			pctl->GetRect(&rcT);
			fEmpty = false;
		}
#endif
	} else {
		// Use the opaque rect passed

		rcT = *prcOpaque;
	}

	// Map it map relative in tile coords

	if (!fEmpty) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);
		Rect rcMapRelative;
		rcMapRelative = rcT;
		rcMapRelative.Offset(PcFromWc(wxView), PcFromWc(wyView));
		gtrcMapOpaque.left = TcFromPc(rcMapRelative.left + gcxTile - 1);
		gtrcMapOpaque.top = TcFromPc(rcMapRelative.top + gcyTile - 1);
		gtrcMapOpaque.right = TcFromPc(rcMapRelative.right);
		gtrcMapOpaque.bottom = TcFromPc(rcMapRelative.bottom);

		// Expand the tile rect if the opaque rect is on the screen
		// edge. This way gobs will get opaqued when on screen edge even

		Size sizDib;
		m_pfrmm->GetDib()->GetSize(&sizDib);
		if (rcT.left <= 0)
			gtrcMapOpaque.left = 0;
		if (rcT.top <= 0)
			gtrcMapOpaque.top = 0;
		if (rcT.right >= sizDib.cx)
			gtrcMapOpaque.right = ktcMax;
		if (rcT.bottom >= sizDib.cy)
			gtrcMapOpaque.bottom = ktcMax;

		// Turn this opaque rect on by setting this global

		gptrcMapOpaque = &gtrcMapOpaque;
	} else {
		gptrcMapOpaque = NULL;
	}

	// Get visible gobs

	Gob **ppgobVisible;
	int cgobVisible;
	gsim.FindVisibleGobs(&ppgobVisible, &cgobVisible);

	// Add known invalid gobs to the update map

	Gob **ppgobStop = &ppgobVisible[cgobVisible];
	Gob **ppgobT;
	for (ppgobT = ppgobVisible; ppgobT < ppgobStop; ppgobT++) {
		Gob *pgob = *ppgobT;

		// Invalidate if asked to redraw

		if (pgob->GetFlags() & kfGobRedraw)
			pgob->Invalidate();
	}

	// Mark gobs not already marked for redraw that are touching invalid areas
	// Add this "overflow" to the damage map; this is what is used to create
	// a valid back buffer when scrolling

 	bool fNewInvalid = false;
 	do {
 		fNewInvalid = false;
 		for (ppgobT = ppgobVisible; ppgobT < ppgobStop; ppgobT++) {
 			Gob *pgob = *ppgobT;
			dword ff = pgob->GetFlags();
			if (!(ff & kfGobRedraw)) {
 				if (pupd->IsMapTileRectInvalidAndTrackDamage(&pgob->m_trcBoundingLast, &fNewInvalid))
 					pgob->SetFlags(ff | kfGobRedraw);
 			}
 		}
 	} while (fNewInvalid);

	// Let the base invalidate controls as necessary

	Form::OnUpdateMapInvalidate(pupd, prcOpaque);
}

void SimUIForm::InvalidateDragSelection()
{
	if (gfDragSelecting) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);

		if (gfLassoSelection) {
			// The slow redraw this causes is unavoidable unless we remove lasso as an option.

			WPoint wptLast;
			wptLast.wx = s_awptSelection[0].wx - wxView;
			wptLast.wy = s_awptSelection[0].wy - wyView;
			for (int i = 1; i < s_cwptSelection; i++) {
				WPoint wptNext;
				wptNext.wx = s_awptSelection[i].wx - wxView;
				wptNext.wy = s_awptSelection[i].wy - wyView;
				int x1 = PcFromWc(wptLast.wx);
				int y1 = PcFromWc(wptLast.wy);
				int x2 = PcFromWc(wptNext.wx);
				int y2 = PcFromWc(wptNext.wy);
				Rect rcT;
				rcT.left = _min(x1, x2);
				rcT.top = _min(y1, y2);
				rcT.right = rcT.left + abs(x2 - x1) + 1;
				rcT.bottom = rcT.top + abs(y2 - y1) + 1;
				gpupdSim->InvalidateRect(&rcT);
				wptLast = wptNext;
			}
		} else {
			// PERF: there is a way to make selection drawing faster by only invalidating edges
			// that move, and redrawing dirty cells only on the control layer. Can do if this isn't fast enough.

			int xLeft = PcFromWc(gwrcSelection.left - wxView);
			int yTop = PcFromWc(gwrcSelection.top - wyView);
			int xRight = PcFromWc(gwrcSelection.right - wxView);
			int yBottom = PcFromWc(gwrcSelection.bottom - wyView);

			Rect rcT;
			rcT.Set(xLeft, yTop, xRight, yTop + 1);
			gpupdSim->InvalidateRect(&rcT);
			rcT.Set(xRight - 1, yTop, xRight, yBottom);
			gpupdSim->InvalidateRect(&rcT);
			rcT.Set(xLeft, yBottom - 1, xRight, yBottom);
			gpupdSim->InvalidateRect(&rcT);
			rcT.Set(xLeft, yTop, xLeft + 1, yBottom);
			gpupdSim->InvalidateRect(&rcT);
		}
	}
}

void SimUIForm::ScrollInvalidate(UpdateMap *pupd)
{
	Form::ScrollInvalidate(pupd);
	InvalidateDragSelection();
	gpmm->Invalidate();
}

void SimUIForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	if (!s_fReadyForPaint)
		return;

	gsim.DrawBackground(pupd, pbm);
}

void SimUIForm::OnPaint(DibBitmap *pbm)
{
	// We're not ready to paint until the first Update has executed because
	// Update initializes things like the Credits and Power controls.

	if (!s_fReadyForPaint)
		return;

	// Draw everything managed by the Simulation (map, Gobs, Gob UI)

	gsim.Draw(m_pfrmm->GetUpdateMap(), pbm);

	// Draw the selection rectangle or lasso lines, if any

	if (gfDragSelecting) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);

		if (gfLassoSelection) {
			Color clr = GetColor(kiclrWhite);
			WPoint wptLast;
			wptLast.wx = s_awptSelection[0].wx - wxView;
			wptLast.wy = s_awptSelection[0].wy - wyView;
			for (int i = 1; i < s_cwptSelection; i++) {
				WPoint wptNext;
				wptNext.wx = s_awptSelection[i].wx - wxView;
				wptNext.wy = s_awptSelection[i].wy - wyView;
				pbm->DrawLine(PcFromWc(wptLast.wx), PcFromWc(wptLast.wy), PcFromWc(wptNext.wx), PcFromWc(wptNext.wy), clr);
				wptLast = wptNext;
			}
		} else {
			Rect rcT;
			rcT.left = PcFromWc(gwrcSelection.left - wxView);
			rcT.top = PcFromWc(gwrcSelection.top - wyView);
			rcT.right = PcFromWc(gwrcSelection.right - wxView);
			rcT.bottom = PcFromWc(gwrcSelection.bottom - wyView);
			DrawBorder(pbm, &rcT, 1, GetColor(kiclrWhite));
		}
	}

	// Handle placement form

	gpfrmPlace->OnPaintSimUI(pbm);

    // Let the input handler draw what it needs

    m_ppenh->OnPaint(pbm);

	// Call the default Form handling

	Form::OnPaint(pbm);
}

void SimUIForm::OnPaintControls(DibBitmap *pbm, UpdateMap *pupd)
{
	// We're not ready to paint until the first Update has executed because
	// Update initializes things like the Credits and Power controls.

	if (!s_fReadyForPaint)
		return;

	// Give some time to sound servicing

	HostSoundServiceProc();

	// Draw fog

	gsim.DrawFog(pupd, pbm);

	// Draw move indicator

	if (m_nStateMoveTarget == 1) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);
		m_aniMoveTarget.Draw(pbm, PcFromUwc(m_wxMoveTarget) - (PcFromWc(wxView) & 0xfffe), PcFromUwc(m_wyMoveTarget) - (PcFromWc(wyView) & 0xfffe));
	}

	// Give some time to sound servicing

	HostSoundServiceProc();

	// Draw controls

	Form::OnPaintControls(pbm, pupd);

	// Handle Structure Placement controls here

	gpfrmPlace->OnPaintControlsSimUI(pbm, pupd); 

	// Draw stats

#ifdef STATS_DISPLAY
	if (gfShowStats) {
		Font *pfnt = gapfnt[kifntDefault];
		int cy = pfnt->GetHeight();
		int y = cy;
		char szT[80];

		// Display useful info

		// Count of active Gobs

		int cpgob = 0;
		for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT), cpgob++);
		sprintf(szT, "active Gobs: %d", cpgob);
		pfnt->DrawText(pbm, szT, 1, y);
		y += cy;

		// Count of visible sprites

		sprintf(szT, "bitmaps drawn: %d", gcBitmapsDrawn);
		pfnt->DrawText(pbm, szT, 1, y);
		y += cy;

		// Count of messages per update

		sprintf(szT, "msgs/update: %d", gcMessagesPerUpdate);
		pfnt->DrawText(pbm, szT, 1, y);
		y += cy;

		// Count of updates

		sprintf(szT, "update count: %ld", gsim.GetUpdateCount());
		pfnt->DrawText(pbm, szT, 1, y);
		y += cy;

		// Invalidate

		Rect rcT = m_rc;
		rcT.top = cy;
		rcT.bottom = y;
		InvalidateRect(&rcT);
	}
#endif

#ifdef STATS_DISPLAY
	s_cUpdateRects += gpmfrmm->GetUpdateRectCount();
#endif
}

void SimUIForm::FrameComplete()
{
    Form::FrameComplete();
}

void SimUIForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcChat:
        if (m_pchatter != NULL) {
            m_pchatter->ShowChat();
        }
        break;
    }
}

// SimUI handles input events which may effect the whole playfield and/or
// multiple Gobs. It does as much as it can independent of the Gobs but 
// passes context-specific messages to the Gobs when specialized responses 
// are desired.
//
// SimUI's OnPenEvent handler is responsible for
//
// - individual selection/multiple selection
// - input routing, i.e., cooking and passing pertinent events to the 
//   appropriate Gobs. For example, passing selected Gobs SetTarget and
//   invoking Gob::PopupMenu.
//

bool SimUIForm::OnPenEvent(Event *pevt)
{
    // Give the form controls first crack at handling the event.  Form
    // controls include: soft menu button, mode cancel button, status
    // label)

    if (Form::OnPenEvent(pevt)) {
        return true;
    }

    // Debug gob identification

    if (pevt->eType == penHoverEvent) {
#if defined(WIN) && defined(DEBUG)
        WCoord wxTarget, wyTarget;
        gsim.GetViewPos(&wxTarget, &wyTarget);
        TCoord tx = TcFromWc(wxTarget + WcFromPc(pevt->x));
        TCoord ty = TcFromWc(wyTarget + WcFromPc(pevt->y));
        for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull;
                gid = ggobm.GetNextGid(gid)) {
            Gob *pgob = ggobm.GetGob(gid);
            if (pgob != NULL) {
                char sz[512];
                pgob->ToString(sz);
                Trace(sz);
            }
        }
#endif
        return true;
    }

    bool fScrollOnly = false;
    if (gpfrmPlace != NULL && gpfrmPlace->IsBusy()) {
        fScrollOnly = true;
    }
	if (gpplrLocal->GetFlags() & kfPlrObserver) {
        fScrollOnly = true;
    }

    return m_ppenh->OnPenEvent(pevt, fScrollOnly);
}

/*
var ar = new Array(64)
for (r = 0; r < 64; r++) {
	c = 0;
	for (y = 50; y >= -50; y--) {
		for (x = 50; x >= -50; x--) {
			d = Math.sqrt(x * x + y * y);
			if (d <= r)
				c++;
		}
	}

	ar[r] = c;
}

for (r = 0; r < 64; r++) {
	WScript.Echo("Range " + r + " fits " + ar[r]);
}

var str = "";
n = 0;
for (r = 0; r < 64; r++) {
	while (n <= ar[r]) {
		if (n % 16 == 0) {
			WScript.Echo(str);
			str = "";
		}
		str += r + ", ";
		n++;
	}
}
*/

// Map unit count to target range, i.e. what circle radius will fit a given count of units

byte gmpcUnitsToRadius[] = 
{
	0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10,
};

int RadiusFromUnitCount(int cUnits)
{
	TCoord tcRadius;
	if (cUnits >= ARRAYSIZE(gmpcUnitsToRadius)) {
		tcRadius = gmpcUnitsToRadius[ARRAYSIZE(gmpcUnitsToRadius) - 1];
	} else {
		tcRadius = gmpcUnitsToRadius[cUnits];
	}
	return tcRadius;
}

MobileUnitGob *SimUIForm::SetSelectionTargets(Gid gid, WCoord wxTarget, WCoord wyTarget)
{
	// Find a target that is free and close

	WPoint wptT;
	FindNearestFreeTile(TcFromWc(wxTarget), TcFromWc(wyTarget), &wptT, 0);
	wxTarget = wptT.wx;
	wyTarget = wptT.wy;

	// Find targets for all MobileUnits. Targets are a "closely packed" version
	// of the current selection, centered around the midpoint of the selection,
	// placed at the target.

    // First figure out which units are mobile and attempt to retain their formation   

    WCoord wxMin = kwcMax;   
    WCoord wxMax = 0;   
    WCoord wyMin = kwcMax;   
    WCoord wyMax = 0;   

	WCoord wcMoveDistPerUpdate = (WCoord)kwcMax;
	int cgobsSel = 0;
    Gob *pgobT;   
    for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {   
        if ((pgobT->GetFlags() & (kfGobSelected | kfGobMobileUnit)) == (kfGobSelected | kfGobMobileUnit) &&    
		        ((gfGodMode && !ggame.IsMultiplayer()) || pgobT->GetSide() == gpplrLocal->GetSide())) {   

			// Get selection dimensions

            WPoint wpt;
			pgobT->GetCenter(&wpt);
            if (wpt.wx < wxMin)   
                    wxMin = wpt.wx;   
            if (wpt.wx > wxMax)   
                    wxMax = wpt.wx;   
            if (wpt.wy < wyMin)   
                    wyMin = wpt.wy;   
            if (wpt.wy > wyMax)   
                    wyMax = wpt.wy;   
			cgobsSel++;

			// Get min speed per update

			if (pgobT->GetFlags() & kfGobMobileUnit) {
				MobileUnitGob *pmunt = (MobileUnitGob *)pgobT;
				MobileUnitConsts *pmuntc = (MobileUnitConsts *)pmunt->GetConsts();
				if (pmuntc->GetMoveDistPerUpdate() < wcMoveDistPerUpdate)
					wcMoveDistPerUpdate = pmuntc->GetMoveDistPerUpdate();
			}
        }   
    }
	WCoord cwxSel = wxMax - wxMin;
	if (cwxSel <= 0)
		cwxSel = 1;
	WCoord cwySel = wyMax - wyMin;
	if (cwySel <= 0)
		cwySel = 1;

	// The dest scaling rect is different if moving vs. attacking.

	int ctxSel = TcFromWc(wxMax) - TcFromWc(wxMin) + 1;
	int ctySel = TcFromWc(wyMax) - TcFromWc(wyMin) + 1;
	int ctxTarg;
	int ctyTarg;
	if (gid == kgidNull) {
		// Pick a destination scaling rect for movement

		ctxTarg = ctxSel;
		ctyTarg = ctySel;

		// Pack the rect if it is <= 50% full

		if (cgobsSel <= (ctxSel * ctySel + 1) / 2) {
			int ctxT = ctxSel;
			int ctyT = ctySel;
			while (true) {
				// Remember last 

				ctxTarg = ctxT;
				ctyTarg = ctyT;

				// Try smaller size, decrease whichever size is greater

				if (ctxT > ctyT) {
					ctxT--;
				} else {
					ctyT--;
				}

				if (ctxT * ctyT < cgobsSel + cgobsSel / 3)
					break;
			}
		}

	} else {
		// Pick a destination scaling rect for attacking

		Gob *pgob = ggobm.GetGob(gid);
		ctxTarg = 1;
		ctyTarg = 1;
		if (pgob != NULL) {
			TRect trc;
			pgob->GetTileRect(&trc);
			ctxTarg = trc.right - trc.left;
			ctyTarg = trc.bottom - trc.top;
		}

		// Make the outside edge away from the gob; this hardwires some accounting
		// for some firing distance, gives a larger edge for gobs to move to

		ctxTarg += 2;
		ctyTarg += 2;
	}

	WCoord cwxTarg = WcFromTc(ctxTarg - 1);
	if (cwxTarg == 0)
		cwxTarg = 1;
	WCoord cwyTarg = WcFromTc(ctyTarg - 1);
	if (cwyTarg == 0)
		cwyTarg = 1;

	// Figure out the range of this target

	TCoord tcTargetRadius = RadiusFromUnitCount(cgobsSel);

    // Find src center   

    WCoord wxCenterSrc = wxMin + (wxMax - wxMin) / 2;   
    WCoord wyCenterSrc = wyMin + (wyMax - wyMin) / 2;   

	// Find map size

    Size sizMapTiles; 
    gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&sizMapTiles); 
	WCoord cwxMap = WcFromTc(sizMapTiles.cx);
	WCoord cwyMap = WcFromTc(sizMapTiles.cy);

	// Move & keep formation, or target destination if not a mobile unit

	Gob *pgobTarget = ggobm.GetGob(gid);

	MobileUnitGob *pmuntSetTarget = NULL;
	for (pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		dword ff = pgobT->GetFlags();
		if ((ff & kfGobSelected) && 
				((gfGodMode && !ggame.IsMultiplayer()) || pgobT->GetSide() == gpplrLocal->GetSide() || (pgobTarget != NULL && pgobTarget->GetType() == kgtReplicator))) {

			// If it's a mobile gob, give it a move command to a scaled coordinate

			if (ff & kfGobMobileUnit) {
				// Calc dst

 				MobileUnitGob *pmunt = (MobileUnitGob *)pgobT;
 				WPoint wptSrc;
				pmunt->GetCenter(&wptSrc);
				WCoord wxDst;
				if (wptSrc.wx - wxCenterSrc >= 0) {
 					wxDst = (WCoord)(((long)(wptSrc.wx - wxCenterSrc) * cwxTarg * 8 + (cwxSel * 4)) / (cwxSel * 8) + wxTarget);
				} else {
 					wxDst = (WCoord)(((long)(wptSrc.wx - wxCenterSrc) * cwxTarg * 8 - (cwxSel * 4)) / (cwxSel * 8) + wxTarget);
				}
 				if (wxDst < 0)
 					wxDst = 0;
 				if (wxDst >= cwxMap)
 					wxDst = cwxMap - 1;
 				WCoord wyDst;
				if (wptSrc.wy - wyCenterSrc >= 0) {
					wyDst = (WCoord)(((long)(wptSrc.wy - wyCenterSrc) * cwyTarg * 8 + (cwySel * 4)) / (cwySel * 8) + wyTarget);
				} else {
					wyDst = (WCoord)(((long)(wptSrc.wy - wyCenterSrc) * cwyTarg * 8 - (cwySel * 4)) / (cwySel * 8) + wyTarget);
				}
 				if (wyDst < 0)
 					wyDst = 0;
 				if (wyDst >= cwyMap)
 					wyDst = cwyMap - 1;
				if (gfGodMode && !ggame.IsMultiplayer())
					pmunt->ClearAction();

				// Need to clean up the targets.
				// 1. Targets can be either in open terrain or on the unit being targetted,
				//    not on other structures (other munts is ok)
				// 2. For any targets on the munt being targetted, if it's a structure it needs
				//    to be checked for accessibility
				// 3. Adjust the target radius to be on the same side of any blockage

				// Clean up dest if attacking

				if (pgobTarget != NULL) {
					// Find the closest tile to this that is unblocked by terrain
					// Ok if on a structure... which structure is important, that is checked next

					WPoint wptT;
					FindNearestFreeTile(TcFromWc(wxDst), TcFromWc(wyDst), &wptT, 0);
					wxDst = wptT.wx;
					wyDst = wptT.wy;

					// Calc these now as wxDst, wyDst may have changed

					TCoord txT = TcFromWc(wxDst);
					TCoord tyT = TcFromWc(wyDst);

					// If dest is on top of a structure make sure that is the target and dest
					// is accessible

					Assert(pgobTarget->GetFlags() & kfGobUnit);
					UnitGob *puntTarget = (UnitGob *)pgobTarget;
					Gob *pgobT = ggobm.GetShadowGob(txT, tyT);
					if (pgobT != NULL) {
                        // The destination is on top of a structure. Need to
                        // check if this is ok If it isn't ok, get a new point
                        // that is accessible. If it is ok, make sure the
                        // position is accessible.

						Assert(pgobT->GetFlags() & kfGobStructure);
						StructGob *pstruT = (StructGob *)pgobT;
						if (pgobT != pgobTarget) {
                            // Targetting something but wxDst, wyDst on a
                            // structure we're not targetting.  Get a valid
                            // attack point from the real target

							WPoint wptT;
							puntTarget->GetAttackPoint(&wptT);
							wxDst = wptT.wx;
							wyDst = wptT.wy;
						} else {
                            // wxDst, wyDst is on top of the structure we are
                            // targetting which is fine. Get an accessible
                            // position if the one we have isn't accessible

							if (!pstruT->IsAccessible(txT, tyT)) {
								WPoint wptT;
								puntTarget->GetAttackPoint(&wptT);
								wxDst = wptT.wx;
								wyDst = wptT.wy;
							}
						}
					}
				} else {
					// Not attacking, just moving.
                    // Find a clear terrain spot that isn't terrain blocked and
                    // not on a structure

					WPoint wptT;
					FindNearestFreeTile(TcFromWc(wxDst), TcFromWc(wyDst), &wptT, kbfStructure);
					wxDst = wptT.wx;
					wyDst = wptT.wy;
				}

                // Adjust the target radius so we end up on the same side of
                // any blockage

				TCoord tcTargetRadiusT = tcTargetRadius;
				TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
				TCoord txTo = TcFromWc(wxTarget);
				TCoord tyTo = TcFromWc(wyTarget);
				TCoord txFrom = TcFromWc(wxDst);
				TCoord tyFrom = TcFromWc(wyDst);
				TCoord txFree, tyFree;
				if (ptrmap->FindLastUnoccupied(txTo, tyTo, txFrom, tyFrom, &txFree, &tyFree)) {
					if (txFree != txFrom || tyFree != tyFrom) {
						wxDst = WcFromTc(txFree) + kwcTileHalf;
						wyDst = WcFromTc(tyFree) + kwcTileHalf;
						int dtx = abs(txFree - txTo);
						if (dtx < ARRAYSIZE(gmpDistFromDxy)) {
							int dty = abs(tyFree - tyTo);
							if (dty < ARRAYSIZE(gmpDistFromDxy)) {
								tcTargetRadiusT = gmpDistFromDxy[dtx][dty];
							}
						}
					}
				}

				// Send command

 				pmunt->SetTarget(gid, wxDst, wyDst, wxTarget, wyTarget, tcTargetRadiusT, wcMoveDistPerUpdate);
				pmuntSetTarget = pmunt;
			}
			
            // Else give it a set target command with the original target
            // coordinate

			else if (ff & kfGobUnit) {
				UnitGob *punt = (UnitGob *)pgobT;
				punt->SetTarget(gid, wxTarget, wyTarget, 0, wcMoveDistPerUpdate);
			}
		}
	}

	return pmuntSetTarget;
}

void SimUIForm::ClearSelection()
{
	s_wptSelect1.wx = 0;
	s_wptSelect1.wy = 0;
	s_wptSelect2 = s_wptSelect1;
	s_cwptSelection = 0;
	gwrcSelection.Set(s_wptSelect1, s_wptSelect2);
	gsim.ClearGobSelection();
}

Gob *SimUIForm::HitTestGob(int x, int y, bool fFinger, WCoord *pwx,
        WCoord *pwy, bool *pfHitSurrounding)
{
	WCoord wxTarget, wyTarget;
	gsim.GetViewPos(&wxTarget, &wyTarget);
	wxTarget += WcFromPc(x);
	wyTarget += WcFromPc(y);

	// Pin to edges. Gobs created off the map will cause corruption

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	if (TcFromWc(wxTarget) >= ctx)
		wxTarget = WcFromTc(ctx - 1);
	if (TcFromWc(wyTarget) >= cty)
		wyTarget = WcFromTc(cty - 1);

    *pwx = wxTarget;
    *pwy = wyTarget;

    if (fFinger) {
        return gsim.FingerHitTest(wxTarget, wyTarget, kfGobActive | kfGobUnit,
                pfHitSurrounding);
    } else {
        Gob *pgobHit = NULL;
        Enum enm;
        gsim.HitTest(&enm, wxTarget, wyTarget, kfGobActive | kfGobUnit,
                &pgobHit);
        *pfHitSurrounding = false;
        return pgobHit;
    }
}

void SimUIForm::MoveOrAttackOrSelect(int x, int y, dword ff)
{
	WCoord wxTarget, wyTarget;
    bool fHitSurrounding;
    Gob *pgobHit = HitTestGob(x, y, false, &wxTarget, &wyTarget,
            &fHitSurrounding);
    MoveOrAttackOrSelect(pgobHit, wxTarget, wyTarget, ff);
}

// This routine is called in two situations.
// 1. On pen down to select the unit tapped on (fSelectOnly == true)
// 2. On pen up to attack the unit tapped on or to move to the location tapped on (fSelectOnly == false)

const int kctDoubleTapDelay = 100; // TUNE:

void SimUIForm::MoveOrAttackOrSelect(Gob *pgobHit, WCoord wxTarget,
        WCoord wyTarget, dword ff)
{
    // TODO: break up this giant routine

    if (pgobHit == NULL) {
		if (ff & kfMasMove) {
			MobileUnitGob *pmunt = SetSelectionTargets(kgidNull, wxTarget, wyTarget);
			if (pmunt != NULL) {
				// BUGBUG: fix this multiplayer problem (gob lists out of sync)
#if 0
				if (!ggame.IsMultiplayer()) {
					WCoord wxT = WcTrunc(wxTarget) + kwcTileHalf;
					WCoord wyT = WcTrunc(wyTarget) + kwcTileHalf;
					CreateAnimGob(wxT, wyT, kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, g_panidMoveTarget);
				}
#else
				// Single and multi-player friendly move target

				if (m_nStateMoveTarget != 0)
					m_pfrmm->GetUpdateMap()->InvalidateTile(TcFromWc(m_wxMoveTarget), TcFromWc(m_wyMoveTarget));
				m_aniMoveTarget.Start(0, 0, 0);
				m_wxMoveTarget = WcTrunc(wxTarget) + kwcTileHalf;
				m_wyMoveTarget = WcTrunc(wyTarget) + kwcTileHalf;
				m_nStateMoveTarget = 1;
#endif
				gsndm.PlaySfx(SfxFromCategory(((MobileUnitConsts *)gapuntc[pmunt->GetUnitType()])->sfxcMove));
			}

#ifdef RALLY_POINTS
			// TOTAL HACK:
            // If the player has HRC or VTS selected and they've tapped the
            // ground set that point as the structure's rally point. Of course
            // this should be visualized, etc. Ah, if only we had time for such
            // things.

			for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
				if ((pgobT->GetFlags() & (kfGobStructure | kfGobSelected)) != (kfGobStructure | kfGobSelected))
					continue;
				BuilderGob *pbldr = (BuilderGob *)pgobT;

				// We know that if an enemy structure is selected then a local player's
				// structure can't also be selected.

				if (pgobT->GetOwner() != gpplrLocal)
					break;

				// VTS or HRC?

				if (!((1UL << pbldr->GetUnitType()) & (kumVehicleTransportStation | kumHumanResourceCenter)))
					break;

				pbldr->SetRallyPoint(TcFromWc(wxTarget), TcFromWc(wyTarget));
				WCoord wxT = WcTrunc(wxTarget) + kwcTileHalf;
				WCoord wyT = WcTrunc(wyTarget) + kwcTileHalf;
				CreateAnimGob(wxT, wyT, kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, g_panidMoveTarget);
				ShowAlert("Rally point set");
				break;
			}
#endif
		}
		return;
	}

    // Replicator is a special case. When it is specified as a target the
    // selected units are directed to move to its input.

	if (pgobHit->GetType() == kgtReplicator) {
		if ((ff & (kfMasMove | kfMasAttack)) == 0) {
			return;
        }

		MobileUnitGob *pmunt = SetSelectionTargets(pgobHit->GetId(), wxTarget, wyTarget);
		if (pmunt != NULL)
			gsndm.PlaySfx(SfxFromCategory(((MobileUnitConsts *)gapuntc[pmunt->GetUnitType()])->sfxcMove));
		return;
	}


	// God mode is allowed only if gfGodMode is on when playing single player

	bool fGodMode = false;
	if (gfGodMode && !ggame.IsMultiplayer())
		fGodMode = true;

	// Selection?

	if (IsSelectionCommand(pgobHit)) {
		static long s_tLastSelected = 0;
		static Gob *s_pgobLastSelected = NULL;

		if (ff & kfMasSelect) {
			bool fAlreadySelected = (pgobHit->GetFlags() & kfGobSelected) != 0;

            // Selection. Cancel selection for units elsewhere, set selection
            // for this unit

            // If the unit is already selected and the last selection occured
            // in within the double-tap window then select all units of the
            // same type on-screen.  UNDONE: this may not be the best place to
            // do this.

			if (fAlreadySelected && HostGetTickCount() - s_tLastSelected < kctDoubleTapDelay && s_pgobLastSelected == pgobHit) {
                SelectSameUnitTypes(pgobHit, false);
			} else {
				gsim.SetGobSelected(pgobHit);
			}
			s_tLastSelected = HostGetTickCount();
			s_pgobLastSelected = pgobHit;

			// Play sfx

			if (!gfGodMode) {
				if (pgobHit->GetFlags() & kfGobStructure) {
					gsndm.PlaySfx(((StructConsts *)gapuntc[((StructGob *)pgobHit)->GetUnitType()])->sfxSelect);
				} else {
					if (gfGodMode || pgobHit->GetSide() == gpplrLocal->GetSide())
						gsndm.PlaySfx(SfxFromCategory(((MobileUnitConsts *)gapuntc[((UnitGob *)pgobHit)->GetUnitType()])->sfxcSelect));
				}
			}
		}

        if (ff & kfMasShowMenu) {
			if (pgobHit->GetFlags() & kfGobStructure) {
                ShowUnitMenu(pgobHit);
			}
		}
	} else if (ff & kfMasAttack) {
        // An attack. Reset the wxTarget, wyTarget to be the middle of the gob
        // and issue targets

		UnitGob *puntHit = (UnitGob *)pgobHit;
		WPoint wptTarget;
		puntHit->GetAttackPoint(&wptTarget);
		wxTarget = wptTarget.wx;
		wyTarget = wptTarget.wy;
		MobileUnitGob *pmunt = SetSelectionTargets(puntHit->GetId(), wxTarget, wyTarget);
		if (pmunt != NULL)
			gsndm.PlaySfx(SfxFromCategory(((MobileUnitConsts *)gapuntc[((UnitGob *)pmunt)->GetUnitType()])->sfxcAttack));
	}
}

bool SimUIForm::IsSelectionCommand(Gob *pgobHit)
{
	// God mode is allowed only if gfGodMode is on when playing single player

	bool fGodMode = false;
	if (gfGodMode && !ggame.IsMultiplayer())
		fGodMode = true;

	// Figure out if this is a selection or an attack command:
	//
	// Player can select same side units
	// Player can select enemy units if no friendly units are selected
	// Player can attack enemy units
	// GodMode can select any unit if that doesn't imply an attack (from
    // either side)
	// GodMode can attack any side if there are selected enemy units

	// If there are any selected units that can attack pgobHit, it may be an
    // attack.

	bool fOwnSelection = false;
	bool fSelection = true;
	Side sideHit = pgobHit->GetSide();
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if ((pgobT->GetFlags() & (kfGobUnit | kfGobSelected)) != (kfGobUnit | kfGobSelected))
			continue;
		UnitGob *puntT = (UnitGob *)pgobT;
		if (puntT->GetOwner() == gpplrLocal)
			fOwnSelection = true;
		if (puntT->IsValidTarget(pgobHit)) {
			fSelection = false;
			break;
		}
	}

    // If not in god mode and the local player does not own any of the already
    // selected units then just select the new unit

	if ((!fGodMode) && (!fOwnSelection)) {
        fSelection = true;
    }

    return fSelection;
}

bool SimUIForm::HasSelectedUnits()
{
    Gob *pgobT = ggobm.GetFirstGob();
	for (; pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if ((pgobT->GetFlags() & (kfGobUnit | kfGobSelected | kfGobStructure))
                != (kfGobUnit | kfGobSelected)) {
			continue;
        }
		if (((UnitGob *)pgobT)->GetOwner() == gpplrLocal) {
            return true;
        }
    }
    return false;
}

void SimUIForm::ShowUnitMenu(Gob *pgob) {
    if (pgob != NULL && (pgob->GetFlags() & kfGobUnit) != 0) {
        // Don't popup menu if the player doesn't own the Gob (unless in
        // God mode). God mode is limited for multiplayer games.
        if ((!gfGodMode || ggame.IsMultiplayer()) && 
                pgob->GetSide() != gpplrLocal->GetSide()) {
            return;
        }
        pgob->PopupMenu();
    }
}

void SimUIForm::SelectSameUnitTypes(Gob *pgob, bool fSfx)
{
    if ((pgob->GetFlags() & (kfGobUnit | kfGobStructure)) != kfGobUnit) {
        return;
    }

    WCoord wxViewT, wyViewT;
    gsim.GetViewPos(&wxViewT, &wyViewT);
    TCoord txView = TcFromWc(wxViewT);
    TCoord tyView = TcFromWc(wyViewT);

    Size sizT;
    ggame.GetPlayfieldSize(&sizT);
    TCoord txRight = TcFromWc(wxViewT + WcFromPc(sizT.cx + gcxTile - 1));
    TCoord tyBottom = TcFromWc(wyViewT + WcFromPc(sizT.cy + gcyTile - 1));

    TRect trc;
    trc.Set(txView, tyView, txRight, tyBottom);
    gsim.SelectSameUnitTypes((UnitGob *)pgob, &trc);

    if (gfGodMode || pgob->GetSide() == gpplrLocal->GetSide()) {
        gsndm.PlaySfx(SfxFromCategory(((MobileUnitConsts *)
                gapuntc[((UnitGob *)pgob)->GetUnitType()])->sfxcSelect));
    }
}

void SimUIForm::CalcLevelSpecificConstants()
{
}

// OPT: this isn't hideous but could be made faster

bool PtInPolygon(WPoint *awpt, int cwpt, WCoord wx, WCoord wy)
{
	bool fInside = false;
	WPoint *pwptI = &awpt[0];
	WPoint *pwptJ = &awpt[cwpt - 1];
	WPoint *pwptEnd = pwptJ + 1;

    for (int i = 0; pwptI < pwptEnd; pwptI++) {
		if ((((pwptI->wy <= wy) && (wy < pwptJ->wy)) || ((pwptJ->wy <= wy) && (wy < pwptI->wy))) &&
				(wx < (pwptJ->wx - pwptI->wx) * (wy - pwptI->wy) / (pwptJ->wy - pwptI->wy) + pwptI->wx))
			fInside = !fInside;
		pwptJ = pwptI;
    }

    return fInside;
}

//
// MiniMapControl
//

MiniMapControl *gpmm;

MiniMapControl::MiniMapControl()
{
	m_pbm = NULL;
	m_cxyBorder = 0;
	m_wfMm = 0;
	m_xOff = 0;
	m_yOff = 0;
	m_tInvalidateLast = 0;
    m_pbTileData = NULL;
}

MiniMapControl::~MiniMapControl()
{
	gpmm = NULL;
	delete m_pbm;
    delete[] m_pbTileData;
    m_pbTileData = NULL;
}

int MiniMapControl::CalcWidth()
{
	// Figure out scale based on playfield size

	Size siz;
	ggame.GetPlayfieldSize(&siz);
	int nScale = 1;
	if (siz.cx >= 240)
		nScale = 2;

	// Width

	Size sizMap;
	gsim.GetLevel()->GetTileMap()->GetMapSize(&sizMap);
	return sizMap.cx / (gcxTile / nScale) + 1;
}

bool MiniMapControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) border

	char szBorder[32];
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %s", szBorder);

	// Remember the size of the input UI area since our border drawing needs it

	Size sizDib;
	ggame.GetInputUIForm()->GetFormMgr()->GetDib()->GetSize(&sizDib);
	m_cyInputUI = sizDib.cy;

	// Our even aligned blts depend on having a border for odd width maps

	m_cxyBorder = 1;

	// Figure out scale based on playfield size

	Size siz;
	ggame.GetPlayfieldSize(&siz);
	m_nScale = 1;
	if (siz.cx >= 240 && siz.cx < 320) {
		if (siz.cy > siz.cx)
			m_nScale = 2;
	}
	if (siz.cx >= 320)
		m_nScale = 2;
	gsim.SetMiniMapScale(m_nScale);

	// Size

	Size sizMap;
	gsim.GetLevel()->GetTileMap()->GetMapSize(&sizMap);
	m_rc.left = m_rc.right - ((sizMap.cx / (gcxTile / m_nScale)) + m_cxyBorder);
	m_rc.bottom = m_rc.top + (sizMap.cy / (gcyTile / m_nScale)) + m_cxyBorder;

	// The map is aligned to an edge. The edge is even, and the dib starts on
	// an even address, so we want the width to be even so our blts are aligned.
	// If the map is odd, draw the border inside the dib to make it even. If the
	// map is even, draw the border outside the dib.

	int cx = m_rc.Width();
	if (cx & 1) {
		m_wfMm &= ~kfMmVertBorderInside;
		cx -= m_cxyBorder;
		m_xOff = 0;
		m_yOff = m_cxyBorder;
	} else {
		m_wfMm |= kfMmVertBorderInside;
		m_xOff = m_cxyBorder;
		m_yOff = m_cxyBorder;
	}

	// Alloc a dib to hold it

	m_pbm = CreateDibBitmap(NULL, cx, m_rc.Height());
	if (m_pbm == NULL)
		return false;

	// Draw internal borders

	int x = 0;
	int y = 0;
	if (m_cxyBorder != 0) {
		int iclr = gfMultiplayer ? gaiclrSide[gpplrLocal->GetSide()] : kiclr0CyanSide;
		Color clrBorder = GetColor(iclr);
		if (m_wfMm & kfMmVertBorderInside) {
			m_pbm->Fill(x + m_cxyBorder, y, m_rc.Width() - m_cxyBorder, m_cxyBorder, clrBorder);
			m_pbm->Fill(x, y, m_cxyBorder, m_rc.Height() - m_cyInputUI + 1, clrBorder);
			m_pbm->Fill(x, y + m_rc.Height() - m_cyInputUI + 1, m_cxyBorder, m_rc.Height(), GetColor(kiclrBlack));
		} else {
			m_pbm->Fill(x, y, m_rc.Width(), m_cxyBorder, clrBorder);
		}
	}

	// Stuff constants

	TileMap *ptmap = gsim.GetLevel()->GetTileMap();
	MiniTileSetHeader *pmtseth = ptmap->GetMiniTileSetHeader(m_nScale);
	m_pwTileMap = ptmap->m_pwMapData;
	m_pbFogMap = gsim.GetLevel()->GetFogMap()->GetMapPtr();
    m_pbTrMap = gsim.GetLevel()->GetTerrainMap()->GetMapPtr();
	ggobm.GetMapSize(&m_ctx, &m_cty);
	m_cbRowBytes = m_ctx * m_nScale;
	m_clrBlack = GetColor(kiclrBlack);
	m_clrWhite = GetColor(kiclrWhite);
	m_clrGalaxite = GetColor(kiclrGalaxite);
    m_clrWall = GetColor(kiclrSideNeutral);
	for (Side sideT = ksideNeutral; sideT < kcSides; sideT++)
		m_aclrSide[sideT] = GetSideColor(sideT);

    // Cache the tile data

    dword *pbtd = (dword *)(pmtseth + 1);
    m_pbTileData = new dword[pmtseth->cTiles];
    for (int i = 0; i < pmtseth->cTiles; i++, pbtd++) {
        m_pbTileData[i] = *pbtd;
    }

	// Calc powered radar flag

	if (!CalcPoweredRadar())
		Redraw();

	// Remember this object globally

	gpmm = this;
	return true;
}

void MiniMapControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	int x = m_rc.left + rcForm.left;
	int y = m_rc.top + rcForm.top;

	// Lay down bits

	Size siz;
	m_pbm->GetSize(&siz);
	Rect rcSrc;
	rcSrc.Set(0, 0, siz.cx, siz.cy);

	if (m_wfMm & kfMmVertBorderInside) {
		pbm->Blt(m_pbm, &rcSrc, x, y);
	} else {
		pbm->Blt(m_pbm, &rcSrc, x + m_cxyBorder, y);
		int iclr = gfMultiplayer ? gaiclrSide[gpplrLocal->GetSide()] : kiclr0CyanSide;
		pbm->Fill(x, y, m_cxyBorder, m_rc.Height() - m_cyInputUI + 1, GetColor(iclr));
		pbm->Fill(x, y + m_rc.Height() - m_cyInputUI + 1, m_cxyBorder, m_rc.Height(), GetColor(kiclrBlack));
	}

	// Draw box showing the full-screen visible area of the map

	WCoord wxViewT, wyViewT;
	gsim.GetViewPos(&wxViewT, &wyViewT);
	short xView = PcFromUwc(wxViewT) & 0xfffe;
	short yView = PcFromUwc(wyViewT) & 0xfffe;
	WCoord wxView = WcFromUpc(xView);
	WCoord wyView = WcFromUpc(yView);

	int cxTile = gcxTile / m_nScale;
	int cyTile = gcyTile / m_nScale;

	Rect rc;
	rc.left = x + m_cxyBorder + PcFromUwc(wxView) / cxTile;
	rc.top = y + m_cxyBorder + PcFromUwc(wyView) / cyTile;
	ggame.GetPlayfieldSize(&siz);
	rc.right = rc.left + (siz.cx + cxTile - 1) / cxTile;
	rc.bottom = rc.top + (siz.cy + cyTile - 1) / cyTile;
	DrawBorder(pbm, &rc, 1, GetColor(kiclrWhite));
}

int MiniMapControl::OnHitTest(Event *pevt)
{
    // Don't use finger hit testing on the minimap control; this way
    // all input over the map goes to the map.

    if (pevt->ff & kfEvtFinger) {
        pevt->ff = ~kfEvtFinger;
        int result = Control::OnHitTest(pevt);
        pevt->ff |= kfEvtFinger;
        return result;
    }
    return Control::OnHitTest(pevt);
}

//TUNE:
#define kctMiniMapInvalidateRate 25

void MiniMapControl::Update()
{	
    // Gets called at Update time. If the minimap is dirty and it's been at
    // least kctMiniMapUpdateRate ticks since the last invalidate, invalidate
    // it now. This gives us immediate invalidates in many cases, but ensures
    // there is at least kctMiniMapUpdateRate ticks between invalidates.

    long tCurrent = gtimm.GetTickCount();
	if (m_wfMm & kfMmRedraw) {
		if (m_tInvalidateLast == 0 || tCurrent - m_tInvalidateLast >= kctMiniMapInvalidateRate) {
			m_tInvalidateLast = tCurrent;
			m_wfMm &= ~kfMmRedraw;
			Invalidate();
		}
	}

// Enough time to give the second finger time to come down, and not long
// enough to make the minimap jerky

#define kctPenDownTimeout 25

    if (m_wfMm & kfMmPenDownTimeout) {
        if (tCurrent - m_tPenDown >= kctPenDownTimeout) {
            m_wfMm &= ~kfMmPenDownTimeout;
            OnPenEvent2(&m_evtPenDown);
        }
    }
}

void MiniMapControl::OnBreakCapture()
{
    // Forget about the pen down, if there is one

    m_wfMm &= ~kfMmPenDownTimeout;
    Trace("Breaking capture");
}

void MiniMapControl::Redraw()
{
	TRect trc;
	trc.Set(0, 0, m_ctx, m_cty);
	RedrawTRect(&trc);
}

void MiniMapControl::OnPenEvent(Event *pevt)
{
    // Process the pen down in waiting if there is a pen up

    if (pevt->eType == penUpEvent) {
        if (m_wfMm & kfMmPenDownTimeout) {
            m_wfMm &= ~kfMmPenDownTimeout;
            OnPenEvent2(&m_evtPenDown);
        }
    }

	if (pevt->eType != penDownEvent && pevt->eType != penMoveEvent) {
		return;
    }

#if defined(IPHONE) || defined(__IPHONEOS__) || defined(__ANDROID__)
    // If already waiting for pen down timeout, then just update the x,y

    if (m_wfMm & kfMmPenDownTimeout) {
        m_evtPenDown.x = pevt->x;
        m_evtPenDown.y = pevt->y;
        return;
    }

    // If pen down event, remember it and don't process it for a little bit.
    // This gives enough time to forget about the pen down if the second
    // finger goes down and the control is forced to give up capture.

    if (pevt->eType == penDownEvent) {
        m_tPenDown = gtimm.GetTickCount();
        m_evtPenDown = *pevt;
        m_wfMm |= kfMmPenDownTimeout;
        return;
    }
#endif

    OnPenEvent2(pevt);
}

void MiniMapControl::OnPenEvent2(Event *pevt)
{
	static int s_xPenDown;
	static int s_yPenDown;
	static WCoord s_wxViewDown;
	static WCoord s_wyViewDown;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	int xPen = pevt->x - (rcForm.left + m_rc.left + m_cxyBorder);
	int yPen = pevt->y - (rcForm.top + m_rc.top + m_cxyBorder);

	WCoord wxView, wyView;
	gsim.GetViewPos(&wxView, &wyView);

	Size sizPlayfield;
	ggame.GetPlayfieldSize(&sizPlayfield);

	if (pevt->eType == penDownEvent) {
		// Calc the rectangle (in minimap pixel coordinates) bounding the map
		// area currently being viewed.

		Rect rcView;
		rcView.left = TcFromWc(wxView) * m_nScale;
		rcView.top = TcFromWc(wyView) * m_nScale;
		rcView.right = rcView.left + (((sizPlayfield.cx + gcxTile - 1) / gcxTile) * m_nScale);
		rcView.bottom = rcView.top + (((sizPlayfield.cy + gcyTile - 1) / gcyTile) * m_nScale);

#if 0
// This feels super sloppy, plus it isn't neeed for fine movement anymore,
// since the map scrolls nicely with a finger directly
        // If this is finger UI, make the rect larger
        
        if (pevt->ff & kfEvtFinger) {
            int cxInflate = (gcxTile * 2 - rcView.Width()) / 2;
            if (cxInflate < 0) {
                cxInflate = 0;
            }
            int cyInflate = (gcyTile * 2 - rcView.Height()) / 2;
            if (cyInflate < 0) {
                cyInflate = 0;
            }
            rcView.Inflate(cxInflate, cyInflate);
        }
#endif
        
		// Pen down inside the view rect?

		if (!rcView.PtIn(xPen, yPen)) {

			// No, set the view to be centered around the point tapped

			TCoord tx = xPen / m_nScale;
			TCoord ty = yPen / m_nScale;

			wxView = WcFromTc(tx - (sizPlayfield.cx / gcxTile) / 2);
			wyView = WcFromTc(ty - ((sizPlayfield.cy + gcyTile - 1) / gcyTile) / 2);
		}

		s_xPenDown = xPen;
		s_yPenDown = yPen;
		s_wxViewDown = wxView;
		s_wyViewDown = wyView;
	} else {
        int dx = xPen - s_xPenDown;
		wxView = s_wxViewDown + WcFromTc(dx / m_nScale);
        wxView += (dx % m_nScale) * kwcTile / m_nScale;
        int dy = yPen - s_yPenDown;
		wyView = s_wyViewDown + WcFromTc(dy / m_nScale);
        wyView += (dy % m_nScale) * kwcTile / m_nScale;
	}

	gsim.SetViewPos(wxView, wyView);
	Invalidate();
}

bool MiniMapControl::CalcPoweredRadar()
{
	// gpmm uninitialized?

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#endif

	if (this == NULL)
		return false;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	// Remember if the local player has fully powered Radar. This determines whether
	// enemy units show on the mini-map.

	bool fHasPoweredRadar = gpplrLocal->GetUnitCount(kutRadar) > 0 && gpplrLocal->GetPowerDemand() <= gpplrLocal->GetPowerSupply();
	word wfMm = fHasPoweredRadar ? kfMmHasPoweredRadar : 0;
	if ((wfMm ^ m_wfMm) != 0) {
		m_wfMm &= ~kfMmHasPoweredRadar;
		m_wfMm |= wfMm;
		Redraw();
		return true;
	}
	return false;
}

void MiniMapControl::RedrawTile(TCoord tx, TCoord ty)
{
	TRect trc;
	trc.left = tx;
	trc.top = ty;
	trc.right = tx + 1;
	trc.bottom = ty + 1;
	RedrawTRect(&trc);
}

void MiniMapControl::RedrawTRect(TRect *ptrc)
{
	// gpmm uninitialized?

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-undefined-compare"
#endif

	if (this == NULL)
		return;

#ifdef __clang__
#pragma clang diagnostic pop
#endif

	// Redraw this rect

    Rect rc;
    rc.Set(m_xOff, m_yOff, ptrc->Width(), ptrc->Height());
    DibBitmap *pbmDst = m_pbm->Suballoc(rc);
	long offset = (long)ptrc->top * m_ctx + ptrc->left;
	byte *pbFogMap = m_pbFogMap + offset;
	word *pwTileMap = m_pwTileMap + offset;
    byte *pbTrMap = m_pbTrMap + offset;
	int ctReturn = m_ctx - ptrc->Width();

#define HasWall(btt) ((btt) == kttWall)
    for (TCoord ty = ptrc->top; ty < ptrc->bottom; ty++) {
        for (TCoord tx = ptrc->left; tx < ptrc->right; tx++, pbFogMap++, pwTileMap++, pbTrMap++) {
            // Fogged?

            if (IsFogOpaque(*pbFogMap)) {
                pbmDst->Fill(tx * m_nScale, ty * m_nScale, m_nScale, m_nScale, m_clrBlack);
                continue;
            }

            // Not fogged; remember to redraw the minimap to the screen next timer

            m_wfMm |= kfMmRedraw;

            // Unit gob?

            UnitGob *punt = ggobm.GetUnitGob(tx, ty);
            if (punt != NULL) {
                dword wf = punt->GetFlags();
                if ((wf & (kfGobMobileUnit | kfGobActive)) != (kfGobMobileUnit))  {
                    Color clr;
                    if (wf & kfGobSelected) {
                        clr = m_clrWhite;
                    } else {
                        clr = m_aclrSide[punt->GetSide()];
                    }

                    pbmDst->Fill(tx * m_nScale, ty * m_nScale, m_nScale, m_nScale, clr);
                    continue;
                }
            }

            // Wall?

            if (HasWall(*pbTrMap)) {
                pbmDst->Fill(tx * m_nScale, ty * m_nScale, m_nScale, m_nScale, m_clrWall);
                continue;
            }

            // Galaxite?

            if (HasGalaxite(*pbFogMap)) {
                pbmDst->Fill(tx * m_nScale, ty * m_nScale, m_nScale, m_nScale, m_clrGalaxite);
                continue;
            }

            // Tile

            int nTile = (BigWord(*pwTileMap) & 0x7fc);
            dword pbSrc = m_pbTileData[m_nScale == 1 ? nTile >> 2 : nTile];
            pbmDst->Fill(tx * m_nScale, ty * m_nScale, m_nScale, m_nScale, pbSrc);
            continue;
        }
        pwTileMap += ctReturn;
        pbFogMap += ctReturn;
        pbTrMap += ctReturn;
    }
}

} // namespace wi
