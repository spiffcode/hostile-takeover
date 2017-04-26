#include "ht.h"

namespace wi {

bool Trigger::LoadAction(IniReader *pini, FindProp *pfind)
{
	char sz[1000];	// Must be big to hold large Ecom texts
	sz[0] = 0;
	int nAction;
	int cArgs = pini->GetPropertyValue(pfind, "%d,%s", &nAction, sz);
	if (cArgs == 0)
		return false;
	Assert(strlen(sz) + 1 < sizeof(sz));

	TriggerAction *pactn;
	switch (nAction) {
	case knPreserveTriggerTriggerAction:
		pactn = new PreserveTriggerAction();
		break;

	case knWaitTriggerAction:
		pactn = new WaitAction();
		break;

	case knCenterViewTriggerAction:
		pactn = new CenterViewAction();
		break;

	case knSetNextMissionTriggerAction:
		pactn = new SetNextMissionAction();
		break;

	case knEndMissionTriggerAction:
		pactn = new EndMissionAction();
		break;

	case knEcomTriggerAction:
		pactn = new EcomAction();
		break;

	case knSetAllowedUnitsTriggerAction:
		pactn = new SetAllowedUnitsAction();
		break;

	case knAlliesTriggerAction:
		pactn = new AlliesAction();
		break;

	case knSetObjectiveTriggerAction:
		pactn = new SetObjectiveAction();
		break;

	case knSetSwitchTriggerAction:
		pactn = new SetSwitchAction();
		break;

	case knDefogAreaTriggerAction:
		pactn = new DefogAreaAction();
		break;

	case knCreateUnitGroupTriggerAction:
		pactn = new CreateUnitGroupAction();
		break;

	case knHuntTriggerAction:
		pactn = new HuntAction();
		break;

	case knCreateRandomUnitGroupTriggerAction:
		pactn = new CreateRandomUnitGroupAction();
		break;

	case knStartCountdownTimerTriggerAction:
		pactn = new StartCountdownAction();
		break;

	case knModifyCountdownTimerTriggerAction:
		pactn = new ModifyCountdownAction();
		break;

	case knRepairTriggerAction:
		pactn = new RepairAction();
		break;

	case knEnableReplicatorTriggerAction:
		pactn = new EnableReplicatorAction();
		break;

	case knModifyCreditsTriggerAction:
		pactn = new ModifyCreditsAction();
		break;

	case knMoveUnitsInAreaTriggerAction:
		pactn = new MoveUnitsInAreaAction();
		break;

	case knSetFormalObjectiveTextTriggerAction:
		pactn = new SetFormalObjectiveTextAction();
		break;

	case knSetFormalObjectiveStatusTriggerAction:
		pactn = new SetFormalObjectiveStatusAction();
		break;

	case knSetFormalObjectiveInfoTriggerAction:
		pactn = new SetFormalObjectiveInfoAction();
		break;

	case knShowObjectivesTriggerAction:
		pactn = new ShowObjectivesAction();
		break;

	case knCutSceneTriggerAction:
		pactn = new CutSceneAction();
		break;

	case knJumpToMissionTriggerAction:
		pactn = new JumpToMissionAction();
		break;

	case knModifyPvarTriggerAction:
		pactn = new ModifyPvarAction();
		break;

	case knSetPvarTextTriggerAction:
		pactn = new SetPvarTextAction();
		break;

	case knShowAlertTriggerAction:
		pactn = new ShowAlertAction();
		break;

	case knSetAllowedUpgradesTriggerAction:
		pactn = new SetAllowedUpgradesAction();
		break;

	case knSetUpgradesTriggerAction:
		pactn = new SetUpgradesAction();
		break;

#ifdef UNDONE
	case knMoveUnitTriggerAction:
		pactn = new MoveUnitAction();
		break;

	case knSetPlayerControlsTriggerAction:
		pactn = new SetPlayerControlsAction();
		break;

	case knPanViewAction:
		pactn = new PanViewTriggerAction();
		break;

	case knTargetUnitTriggerAction:
		pactn = new TargetUnitAction();
		break;
#endif

	default:
		Assert(false);
		break;
	}

	// Init it, error if that failed

	Assert(pactn != NULL, "out of memory!");
	if (!pactn->Init(sz)) {
		delete pactn;
		return false;
	}

	// Link it in last

	TriggerAction **ppactn = &m_pactn;
	while ((*ppactn) != NULL)
		ppactn = &((*ppactn)->m_pactnNext);
	*ppactn = pactn;

	return true;
}

// Action base class implementation

TriggerAction::TriggerAction()
{
	m_pactnNext = NULL;
}

// Just here to be overridden by derived classes

TriggerAction::~TriggerAction()
{
}

bool TriggerAction::Init(char *psz)
{
	return true;
}

bool TriggerAction::LoadState(Stream *pstm)
{
	return true;
}

bool TriggerAction::SaveState(Stream *pstm)
{
	return true;
}

// PreserveTriggerAction

bool PreserveTriggerAction::Perform(Trigger *ptgr, Side side)
{
	ptgr->Arm(side);
	return true;
}

// WaitAction

WaitAction::WaitAction()
{
	memset(m_afWaitingSide, 0, sizeof(m_afWaitingSide));
}

#define knVerWaitActionState 2
bool WaitAction::LoadState(Stream *pstm) 
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerWaitActionState)
		return false;

	pstm->ReadBytesRLE((byte *)m_atStartSide, sizeof(m_atStartSide));
	pstm->ReadBytesRLE((byte *)m_afWaitingSide, sizeof(m_afWaitingSide));

	return pstm->IsSuccess();
}

bool WaitAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerWaitActionState);

	pstm->WriteBytesRLE((byte *)m_atStartSide, sizeof(m_atStartSide));
	pstm->WriteBytesRLE((byte *)m_afWaitingSide, sizeof(m_afWaitingSide));

	return pstm->IsSuccess();
}

bool WaitAction::Init(char *psz)
{
	int cSecs;
	if (!ParseNumber(&psz, &cSecs))
		return false;
	m_ctWait = cSecs * 100;
	return true;
}

bool WaitAction::Perform(Trigger *ptgr, Side side)
{
	// If in the middle of waiting, check if the wait is over.

	long t = gsim.GetTickCount();
	if (m_afWaitingSide[side]) {
		// Wait over? If so, return true

		if (t - m_atStartSide[side] >= m_ctWait) {
			m_afWaitingSide[side] = false;
			return true;
		}

		// Wait not over, return false

		return false;
	}

	// Starting the wait, assume wait not over

	m_afWaitingSide[side] = true;
	m_atStartSide[side] = (dword)t;
	return false;
}

#ifdef DEBUG_HELPERS
char *WaitAction::ToString() 
{
	// UNDONE: remove hardcode kside1
	sprintf(s_szDebugHelpers, "Wait %d [%d]", m_ctWait / 100, 
			(gsim.GetTickCount() - m_atStartSide[kside1]) / 100);
	return s_szDebugHelpers;
}
#endif

// CenterViewAction
// Immediately center the View on the specified Area

bool CenterViewAction::Init(char *psz)
{
	return ParseArea(&psz, &m_nArea);
}

bool CenterViewAction::Perform(Trigger *ptgr, Side side)
{
	// Only bother for the local player

	if (side != gpplrLocal->GetSide())
		return true;

	TRect trc;
	ggobm.GetAreaRect(m_nArea, &trc, side);
	WRect wrc;
	wrc.FromTileRect(&trc);
	Rect rcSimUI;
	ggame.GetSimUIForm()->GetRect(&rcSimUI);
	gsim.SetViewPos((wrc.left + wrc.Width() / 2) - (WcFromPc(rcSimUI.Width()) / 2), 
			(wrc.top + wrc.Height() / 2) - (WcFromPc(rcSimUI.Height()) / 2), true);
	return true;
}

// SetNextMissionAction

bool SetNextMissionAction::Init(char *psz)
{
	strncpyz(m_szLevel, psz, sizeof(m_szLevel));
	return true;
}

bool SetNextMissionAction::Perform(Trigger *ptgr, Side side)
{
	// Only bother for the local player

	if (side != gpplrLocal->GetSide())
		return true;

	if (strcmp(m_szLevel, "[none]") == 0)
		ggame.SetNextLevel("");
	else
		ggame.SetNextLevel(m_szLevel);
	return true;
}

// EndMissionAction

bool EndMissionAction::Init(char *psz)
{
	return ParseNumber(&psz, &m_nWinLose);
}

bool EndMissionAction::Perform(Trigger *ptgr, Side side)
{
	Assert(m_pactnNext == NULL, "Action attempted after End Mission");
#ifdef DEBUG
	if (m_nWinLose == knWinLoseTypeWin)
		Assert(strcmp(ggame.GetNextLevel(), gsim.GetLevel()->GetFilename()) != 0, "No Next Mission specified");
#endif

    // Treat local and remote players differently. In a single player game we
    // don't allow non-local players to perform EndMission actions (bad
    // scripting). 

	if (side != gpplrLocal->GetSide() && !gfMultiplayer)
		return true;

	// In a multiplayer game we want to notify the local player of the remote 
	// player's demise. We also want to mark the 'EndMission'ed player as an 
	// Observer and put them into observing 'mode'.

	if (gfMultiplayer) {

		Player *pplr = gplrm.GetPlayer(side);
		if ((pplr->GetFlags() & (kfPlrUnfulfilled | kfPlrObserver)) == 0) {
            // Show alert
            char szT[100];
            sprintf(szT, m_nWinLose == knWinLoseTypeWin ? "%s is victorious!" : "%s has been defeated!", pplr->GetName());
            ShowAlert(szT);

            // This player is now out, and an observer
			pplr->SetFlags(pplr->GetFlags() | kfPlrObserver | kfPlrComputer);

			if (pplr == gpplrLocal) {
                // In MP, TriggerMgr::Update() is called synchronously in the
                // simulation, so it is not possible to execute modal loops
                // such the end mission forms. For this reason, this part is
                // done asynchronously.

                Event evt;
                memset(&evt, 0, sizeof(evt));
                if (m_nWinLose == knWinLoseTypeWin) {
                    evt.eType = mpEndMissionWinEvent;
                } else {
                    evt.eType = mpEndMissionLoseEvent;
                }
                evt.dw = side;
                gevm.PostEvent(&evt);
            } else {
                // A non-local player lost. Have the local player check to
                // see if it won.
                if (m_nWinLose == knWinLoseTypeLose) {
                    Event evt;
                    memset(&evt, 0, sizeof(evt));
                    evt.eType = checkGameOverEvent;
                    evt.dw = pplr->GetId();
                    gevm.PostEvent(&evt);
                }
            }
		}

		return true;
	}

	// Keep any more triggers from executing
	// NOTE: doesn't keep subsequent actions in the same trigger from executing

	gsim.GetLevel()->GetTriggerMgr()->Enable(false);

	// Bring up the Objectives screen

	Event evt;
	memset(&evt, 0, sizeof(evt));
	evt.eType = gameOverEvent;

#ifdef STRESS
	if (gfStress) {
		evt.dw = knGoAbortLevel;
	} else {
#endif
		gsim.Pause(true);
		evt.dw = gplrm.GetPlayer(side)->ShowObjectives(m_nWinLose == knWinLoseTypeWin ? ksoWinSummary : ksoLoseSummary);
		gsim.Pause(false);
#ifdef STRESS
	}
#endif

	// UNDONE: post this event even if the app is stopping?
	gevm.PostEvent(&evt);

	// if the app was exited while the mission summary was up, return to it.

	return !gevm.IsAppStopping();
}

void EndMissionAction::OnMPEndMissionActionEvent(int nWinLose, Side side) {

    Player *pplr = gplrm.GetPlayer(side);
    if (pplr != gpplrLocal) {
        return;
    }

    pplr->ShowObjectives(nWinLose == knWinLoseTypeWin ?
            ksoWinSummary : ksoLoseSummary);

    if (ggame.AskObserveGame()) {

        // Cause modal forms to cancel such as build forms, help, options,
        // etc.

        gevm.PostEvent(cancelModeEvent);
    } else {
        Event evt;
        memset(&evt, 0, sizeof(evt));
        evt.eType = gameOverEvent;
        evt.dw = knGoAbortLevel;
        gevm.PostEvent(&evt);
    }
}

// EcomAction

EcomAction::~EcomAction()
{
	delete[] m_pszMessage;
}

bool EcomAction::Init(char *psz)
{
	m_pszMessage = NULL;
	int nParsed;
	if (!ParseNumber(&psz, &m_nBackground))
		return false;
	if (!ParseNumber(&psz, &nParsed))
		return false;
	m_fMore = (nParsed == knMoreCloseTypeMore);
	if (!ParseNumber(&psz, &m_nCharFrom)) {
		Assert(false);	// most likely don't have new ecoms in data file
		return false;
	}
	if (!ParseNumber(&psz, &m_nCharTo))
		return false;
	m_pszMessage = new char[strlen(psz) + 1];
	if (m_pszMessage != NULL)
		strcpy(m_pszMessage, psz);
	return true;
}

bool EcomAction::Perform(Trigger *ptgr, Side side)
{
    // Does nothing in multiplayer
    if (gfMultiplayer) {
        return true;
    }

// Let any side bring up an ecom. Mission Authors request
#if 0
	// Only bother for the local player

	if (side != gpplrLocal->GetSide())
		return true;
#endif

	// if there's a build form up, pass and try again later
	if (gpmfrmm->EcomSuppressed())
		return false;

	// Bring up the Ecom

	gsim.Pause(true);
	//fixme
	if (!gfMultiplayer)
		Ecom(m_nCharFrom, m_nCharTo, m_pszMessage, m_nBackground, m_fMore);
	gsim.Pause(false);

	// if the app was exited while the ecom was up, return to this ecom.

	return !gevm.IsAppStopping();
}

// SetAllowedUnitsAction

bool SetAllowedUnitsAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return ParseUnitMask(&psz, &m_um);
}

bool SetAllowedUnitsAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++)
		applr[n]->SetAllowedUnits(m_um);
	return true;
}

// SetAllowedUpgradesAction

bool SetAllowedUpgradesAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return ParseUpgradeMask(&psz, &m_upgm);
}

bool SetAllowedUpgradesAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++)
		applr[n]->SetAllowedUpgrades(m_upgm);
	return true;
}

// SetUpgradesAction

bool SetUpgradesAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return ParseUpgradeMask(&psz, &m_upgm);
}

bool SetUpgradesAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	word wfUpgrade = 0;
	if (m_upgm & kupgmAdvancedHRC)
		wfUpgrade |= kfUpgradeHrc;
	if (m_upgm & kupgmAdvancedVTS)
		wfUpgrade |= kfUpgradeVts;
	for (int n = 0; n < cplrs; n++)
		applr[n]->SetUpgrades(wfUpgrade);
	return true;
}

// AlliesAction

bool AlliesAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMaskA = nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMaskB = nCaSideMask;
	return true;
}

bool AlliesAction::Perform(Trigger *ptgr, Side side)
{
	SideMask sidmAllies = GetSideMaskFromCaSideMask(side, m_wfCaSideMaskB);
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMaskA, applr);
	gplrm.SetAllies(applr, cplrs, sidmAllies);
	return true;
}

// SetObjectiveAction

SetObjectiveAction::SetObjectiveAction()
{
	m_szObjective = NULL;
}

SetObjectiveAction::~SetObjectiveAction()
{
	if (m_szObjective != NULL)
		gmmgr.FreePtr(m_szObjective);
}

bool SetObjectiveAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	m_szObjective = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szObjective, 0, psz, strlen(psz) + 1);

	return true;
}

bool SetObjectiveAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++)
		applr[n]->SetObjective(m_szObjective);
	return true;
}

// SetSwitchAction

bool SetSwitchAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_iSwitch))
		return false;
	Assert(m_iSwitch < kcSwitchMax);

	int nOnOff;
	if (!ParseNumber(&psz, &nOnOff))
		return false;
	m_fOn = nOnOff == 1;
	return true;
}

bool SetSwitchAction::Perform(Trigger *ptgr, Side side)
{
	gsim.GetLevel()->GetTriggerMgr()->SetSwitch(m_iSwitch, m_fOn);
	return true;
}

// DefogAreaAction

bool DefogAreaAction::Init(char *psz)
{
	return ParseArea(&psz, &m_nArea);
}

bool DefogAreaAction::Perform(Trigger *ptgr, Side side)
{
#if 0
	// Only bother for the local player

	if (side != gpplrLocal->GetSide())
		return true;
#endif

	TRect trc;
	Level *plvl = gsim.GetLevel();
	ggobm.GetAreaRect(m_nArea, &trc, side);
	WCoord wxView, wyView;
	gsim.GetViewPos(&wxView, &wyView);
	plvl->GetFogMap()->Reveal(&trc, gpupdSim, wxView, wyView);
	return true;
}

// CreateUnitGroupAction

bool CreateUnitGroupAction::Init(char *psz)
{
	return ParseNumber(&psz, &m_nUnitGroup);
}

bool CreateUnitGroupAction::Perform(Trigger *ptgr, Side side)
{
	gsim.GetLevel()->GetUnitGroupMgr()->ActivateUnitGroup(m_nUnitGroup);
	return true;
}

// CreateRandomUnitGroupAction

bool CreateRandomUnitGroupAction::Init(char *psz)
{
	return true;
}

bool CreateRandomUnitGroupAction::Perform(Trigger *ptgr, Side side)
{
	gsim.GetLevel()->GetUnitGroupMgr()->ActivateRandomUnitGroup();
	return true;
}

// HuntAction

bool HuntAction::Init(char *psz)
{
	if (!ParseUnitMask(&psz, &m_um1))
		return false;
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask1 = nCaSideMask;

	if (!ParseUnitMask(&psz, &m_um2))
		return false;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask2 = nCaSideMask;
	return true;
}

// UNDONE: since hunting units start by producing a path to the target,
// path creation is expensive, and there may be a lot of hunting units
// set things up so that the hunt actions are performed over time.
// E.g., have 5 units hunt per update.

bool HuntAction::Perform(Trigger *ptgr, Side side)
{
	// UNDONE: enumerate all units that match SideMask1 & UnitMask1 and
	// direct them to hunt units that match SideMask2 & UnitMask2

	// Make a list of valid targets

	// Loop through all 
	return true;
}

// StartCountdownAction

StartCountdownAction::StartCountdownAction()
{
	m_szCountdown = NULL;
}

StartCountdownAction::~StartCountdownAction()
{
	if (m_szCountdown != NULL)
		gmmgr.FreePtr(m_szCountdown);
}

bool StartCountdownAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_nSecs))
		return false;

	m_szCountdown = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szCountdown, 0, psz, strlen(psz) + 1);

	return true;
}

bool StartCountdownAction::Perform(Trigger *ptgr, Side side)
{
	CountdownTimer *pcdt = gsim.GetLevel()->GetTriggerMgr()->GetCountdownTimer();
	Assert(pcdt != 0);
	pcdt->SetTimer(m_nSecs, m_szCountdown);
	pcdt->StartTimer(true);

	return true;
}

// ModifyCountdownAction

bool ModifyCountdownAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_nAction))
		return false;

	return true;
}

bool ModifyCountdownAction::Perform(Trigger *ptgr, Side side)
{
	CountdownTimer *pct = gsim.GetLevel()->GetTriggerMgr()->GetCountdownTimer();
	switch (m_nAction) {
	case knModifyCountdownTypeStop:
		pct->StartTimer(false);
		break;
	case knModifyCountdownTypeResume:
		pct->StartTimer(true);
		break;
	case knModifyCountdownTypeHide:
		pct->ShowTimer(false);
		break;
	case knModifyCountdownTypeShow:
		pct->ShowTimer(true);
		break;
	default:
		Assert(false);
	}
	return true;
}

#ifdef DEBUG_HELPERS
char *ModifyCountdownAction::ToString()
{
	switch (m_nAction) {
	case knModifyCountdownTypeStop:
		sprintf(s_szDebugHelpers, "Stop Countdown");
		break;
	case knModifyCountdownTypeResume:
		sprintf(s_szDebugHelpers, "Resume Countdown");
		break;
	case knModifyCountdownTypeHide:
		sprintf(s_szDebugHelpers, "Hide Countdown");
		break;
	case knModifyCountdownTypeShow:
		sprintf(s_szDebugHelpers, "Show Countdown");
		break;
	default:
		Assert(false);
	}
	return s_szDebugHelpers;
}
#endif

// RepairAction

bool RepairAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	int nOnOff;
	if (!ParseNumber(&psz, &nOnOff))
		return false;
	m_fOn = nOnOff == 1;
	return true;
}

bool RepairAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++)
		applr[n]->Repair(m_fOn);

	return true;
}

// EnableReplicatorAction

bool EnableReplicatorAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	int nOnOff;
	if (!ParseNumber(&psz, &nOnOff))
		return false;
	m_fOn = nOnOff == 1;
	return true;
}

bool EnableReplicatorAction::Perform(Trigger *ptgr, Side side)
{
	SideMask sidm = GetSideMaskFromCaSideMask(side, m_wfCaSideMask);

	for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
		if (pgob->GetType() == kgtReplicator && (sidm & GetSideMask(pgob->GetSide())) != 0) {
			((ReplicatorGob *)pgob)->Enable(m_fOn);
		}
	}

	return true;
}

// ModifyCreditsAction

bool ModifyCreditsAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_nAction))
		return false;

	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	if (!ParseNumber(&psz, &m_nAmount))
		return false;

	return true;
}

bool ModifyCreditsAction::Perform(Trigger *ptgr, Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++) {
		Player *pplr = applr[n];

		switch (m_nAction) {
		case knModifyNumberTypeSet:
			pplr->SetCredits(m_nAmount, true);
			break;

		case knModifyNumberTypeAdd:
			{
				int nCredits = pplr->GetCredits() + m_nAmount;
				if (nCredits < 0)
					nCredits = 0;
				pplr->SetCredits(nCredits, true);
			}
			break;

		case knModifyNumberTypeSubtract:
			{
				int nCredits = pplr->GetCredits() - m_nAmount;
				if (nCredits < 0)
					nCredits = 0;
				pplr->SetCredits(nCredits, true);
			}
			break;

		default:
			Assert(false);
		}
	}

	return true;
}

#ifdef DEBUG_HELPERS
char *ModifyCreditsAction::ToString()
{
	switch (m_nAction) {
	case knModifyNumberTypeSet:
		sprintf(s_szDebugHelpers, "Set Credits");
		break;

	case knModifyNumberTypeAdd:
		sprintf(s_szDebugHelpers, "Add Credits");
		break;

	case knModifyNumberTypeSubtract:
		sprintf(s_szDebugHelpers, "Subtract Credits");
		break;

	default:
		Assert(false);
	}
	return s_szDebugHelpers;
}
#endif

// MoveUnitsInAreaAction

bool MoveUnitsInAreaAction::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	if (!ParseUnitMask(&psz, &m_um))
		return false;

	if (!ParseArea(&psz, &m_nAreaSrc))
		return false;

	return ParseArea(&psz, &m_nAreaDst);
}

bool MoveUnitsInAreaAction::Perform(Trigger *ptgr, Side side)
{
	// Store the gobs on the stack

	Assert(kcpgobMax / 2 * sizeof(Gob *) <= 1536);
	Gob *apgob[kcpgobMax / 2];

	// Find all the relevent units in AreaSrc

	Enum enm;
	UnitMask umFind = m_um & kumMobileUnits;
	SideMask sidmFind = GetSideMaskFromCaSideMask(side, m_wfCaSideMask);
	Gob **ppgobT = apgob;
	Gob *pgob;
	while ((pgob = ggobm.EnumGobsInArea(&enm, m_nAreaSrc, sidmFind, umFind)) != NULL) {
		*ppgobT++ = pgob;
		if (ppgobT - apgob >= ARRAYSIZE(apgob))
			break;
	}

	// Send them off to AreaDst

	int cpmunt = (int)(ppgobT - apgob);
	if (cpmunt > 0) {
		TRect trc;
		ggobm.GetAreaRect(m_nAreaDst, &trc, side);
		MoveUnitsToArea((MobileUnitGob **)apgob, cpmunt, &trc);
	}

	return true;
}

void MoveUnitsToArea(MobileUnitGob **apmunt, int cpmunt, TRect *ptrc)
{
	// Figure out min speed

	MobileUnitGob **ppmunt = apmunt;
	WCoord wcMoveDistPerUpdate = kwcMax;
	int ipmunt;
	for (ipmunt = 0; ipmunt < cpmunt; ipmunt++, ppmunt++) {
		MobileUnitGob *pmunt = *ppmunt;
		Assert((pmunt->GetFlags() & kfGobMobileUnit) != 0);

		// Get min speed per update

		MobileUnitConsts *pmuntc = (MobileUnitConsts *)pmunt->GetConsts();
		if (pmuntc->GetMoveDistPerUpdate() < wcMoveDistPerUpdate)
			wcMoveDistPerUpdate = pmuntc->GetMoveDistPerUpdate();
	}

	// Send commands

	TCoord tcRadius = RadiusFromUnitCount(cpmunt) + 1;
	ppmunt = apmunt;
	for (ipmunt = 0; ipmunt < cpmunt; ipmunt++, ppmunt++) {
		MobileUnitGob *pmunt = *ppmunt;

		// Tell it where to go.

		Point ptCenter;
		ptrc->GetCenter(&ptCenter);
		SendMoveAction(pmunt->GetId(), WcFromTc(ptCenter.x), WcFromTc(ptCenter.y), tcRadius, wcMoveDistPerUpdate);
	}
}

// SetFormalObjectiveTextAction

SetFormalObjectiveTextAction::SetFormalObjectiveTextAction()
{
	m_szObjective = NULL;
}

SetFormalObjectiveTextAction::~SetFormalObjectiveTextAction()
{
	if (m_szObjective != NULL)
		gmmgr.FreePtr(m_szObjective);
}

bool SetFormalObjectiveTextAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_iObjective))
		return false;
	Assert(m_iObjective < kcFormalObjectivesMax, "Objective number (%d) exceeds range", m_iObjective);

	m_szObjective = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szObjective, 0, psz, strlen(psz) + 1);

	return true;
}

bool SetFormalObjectiveTextAction::Perform(Trigger *ptgr, Side side)
{
	Player *pplr = gplrm.GetPlayer(side);
	pplr->SetFormalObjectiveText(m_iObjective, m_szObjective);
	return true;
}

// SetFormalObjectiveStatusAction

SetFormalObjectiveStatusAction::SetFormalObjectiveStatusAction()
{
	m_szStatus = NULL;
}

SetFormalObjectiveStatusAction::~SetFormalObjectiveStatusAction()
{
	if (m_szStatus != NULL)
		gmmgr.FreePtr(m_szStatus);
}

bool SetFormalObjectiveStatusAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_iObjective))
		return false;
	Assert(m_iObjective < kcFormalObjectivesMax, "Objective number (%d) exceeds range", m_iObjective);

	m_szStatus = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szStatus, 0, psz, strlen(psz) + 1);

	return true;
}

bool SetFormalObjectiveStatusAction::Perform(Trigger *ptgr, Side side)
{
	Player *pplr = gplrm.GetPlayer(side);
	pplr->SetFormalObjectiveStatus(m_iObjective, m_szStatus);
	return true;
}

// ShowObjectivesAction

bool ShowObjectivesAction::Perform(Trigger *ptgr, Side side)
{
	// Only bother for the local player

	if (side != gpplrLocal->GetSide())
		return true;

	// Bring up the Objectives form

	if (!gfMultiplayer) {
		gsim.Pause(true);
        gplrm.GetPlayer(side)->ShowObjectives(ksoObjectives, true);
		gsim.Pause(false);
    } else {
		// Make sure this the action won't be executed again while 
		// nested inside of Player::ShowObjective's input loop.

		ptgr->SetCurrentActionComplete(side);

        // Show asynchronously so TriggerMgr::Update() doesn't block
        Event evt;
        memset(&evt, 0, sizeof(evt));
        evt.eType = mpShowObjectivesEvent;
        evt.dw = side;
        gevm.PostEvent(&evt);
    }

	// if the app was exited while the objectives screen was up, return to it.

	return !gevm.IsAppStopping();
}

void ShowObjectivesAction::OnMPShowObjectivesEvent(Side side) {
    // Don't pause the simulation
    gplrm.GetPlayer(side)->ShowObjectives(ksoObjectives, true);
}

// SetFormalObjectiveInfoAction

SetFormalObjectiveInfoAction::SetFormalObjectiveInfoAction()
{
	m_szInfo = NULL;
}

SetFormalObjectiveInfoAction::~SetFormalObjectiveInfoAction()
{
	if (m_szInfo != NULL)
		gmmgr.FreePtr(m_szInfo);
}

bool SetFormalObjectiveInfoAction::Init(char *psz)
{
	m_szInfo = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_szInfo, 0, psz, strlen(psz) + 1);

	return true;
}

bool SetFormalObjectiveInfoAction::Perform(Trigger *ptgr, Side side)
{
	Player *pplr = gplrm.GetPlayer(side);
	pplr->SetFormalObjectiveInfo(m_szInfo);
	return true;
}

// CutSceneAction

CutSceneAction::CutSceneAction()
{
	m_pszMessage = NULL;
}

CutSceneAction::~CutSceneAction()
{
	if (m_pszMessage != NULL)
		gmmgr.FreePtr(m_pszMessage);
}

bool CutSceneAction::Init(char *psz)
{
	m_pszMessage = (char *)gmmgr.AllocPtr(strlen(psz) + 1);
	gmmgr.WritePtr(m_pszMessage, 0, psz, strlen(psz) + 1);

	return true;
}

bool CutSceneAction::Perform(Trigger *ptgr, Side side)
{
    // Does nothing in multiplayer
    if (gfMultiplayer) {
        return true;
    }

	CutScene(m_pszMessage, true);

	// if the app was exited while the cut scene was up, return to it

	return !gevm.IsAppStopping();
}

// JumpToMissionAction

bool JumpToMissionAction::Init(char *psz)
{
	strncpyz(m_szLevel, psz, sizeof(m_szLevel));
	return true;
}

bool JumpToMissionAction::Perform(Trigger *ptgr, Side side)
{
    // Does nothing in multiplayer
    if (gfMultiplayer) {
        return true;
    }

	Assert(m_pactnNext == NULL, "Action attempted after End Mission");

	ggame.SetNextLevel(m_szLevel);

	// Keep any more triggers from executing
	// NOTE: doesn't keep subsequent actions in the same trigger from executing

	gsim.GetLevel()->GetTriggerMgr()->Enable(false);

	Event evt;
	memset(&evt, 0, sizeof(evt));
	evt.eType = gameOverEvent;
	evt.dw = knGoSuccess;

	gevm.PostEvent(&evt);

	return true;
}

// ModifyPvarAction

bool ModifyPvarAction::Init(char *psz)
{
	if (!ParseString(&psz, m_szName))
		return false;
	Assert(strlen(m_szName) < kcbPvarNameMax);

	if (!ParseNumber(&psz, &m_nAction))
		return false;

	if (!ParseNumber(&psz, &m_nAmount))
		return false;

	return true;
}

bool ModifyPvarAction::Perform(Trigger *ptgr, Side side)
{
	int nAmount = m_nAmount;

	switch (m_nAction) {
	case knModifyNumberTypeAdd:
		{
			char szT[kcbPvarValueMax];
			if (ggame.GetVar(m_szName, szT, sizeof(szT)))
				nAmount = atoi(szT) + m_nAmount;
		}
		break;

	case knModifyNumberTypeSubtract:
		{
			char szT[kcbPvarValueMax];
			if (ggame.GetVar(m_szName, szT, sizeof(szT)))
				nAmount = atoi(szT) - m_nAmount;
		}
		break;


	case knModifyNumberTypeSet:
// Not in v1.1 by accident
#if 0
		{
			char szT[kcbPvarValueMax];
			if (ggame.GetVar(m_szName, szT, sizeof(szT)))
				nAmount = atoi(szT);
		}
#endif
		break;

	default:
		Assert(false);
		break;
	}

	char szT[20];
	itoa(nAmount, szT, 10);
	ggame.SetVar(m_szName, szT);
	return true;
}

#ifdef DEBUG_HELPERS
char *ModifyPvarAction::ToString()
{
	switch (m_nAction) {
	case knModifyNumberTypeSet:
		sprintf(s_szDebugHelpers, "Set Pvar");
		break;

	case knModifyNumberTypeAdd:
		sprintf(s_szDebugHelpers, "Add to Pvar");
		break;

	case knModifyNumberTypeSubtract:
		sprintf(s_szDebugHelpers, "Subtract from Pvar");
		break;

	default:
		Assert(false);
	}
	return s_szDebugHelpers;
}
#endif

// SetPvarTextAction

bool SetPvarTextAction::Init(char *psz)
{
	if (!ParseString(&psz, m_szName))
		return false;
	Assert(strlen(m_szName) < kcbPvarNameMax);

	strncpyz(m_szValue, psz, sizeof(m_szValue));
	return true;
}

bool SetPvarTextAction::Perform(Trigger *ptgr, Side side)
{
	ggame.SetVar(m_szName, m_szValue);
	return true;
}

// ShowAlertAction

bool ShowAlertAction::Init(char *psz)
{
	strncpyz(m_szAlert, psz, sizeof(m_szAlert));
	return true;
}

bool ShowAlertAction::Perform(Trigger *ptgr, Side side)
{
	ShowAlert(m_szAlert);
	return true;
}

} // namespace wi
