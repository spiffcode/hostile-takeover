#include "game/ht.h"
#include "game/stateframe.h"

namespace wi {

// Portions Copyright (C) Steve Rabin, 2000

// StateMachine methods

StateMachine::StateMachine()
{
	m_fForceStateChange = false;
	m_st = m_stNext = kstReservedGlobal;
#ifdef DEBUG_HELPERS
	m_fDebug = false;
#endif
}

int StateMachine::ProcessStateMachineMessage(State st, Message *pmsg)
{
	return knHandled;
}

// StateMachineMgr methods

StateMachineMgr::StateMachineMgr()
{
	m_pdmsgHead = NULL;
}

StateMachineMgr::~StateMachineMgr()
{
	ClearDelayedMessages();
}

bool StateMachineMgr::Init(TimerMgr *ptimm)
{
	m_ptimm = ptimm;
	ClearDelayedMessages();
	return true;
}

#define knVerStateMachineMgrState 1
bool StateMachineMgr::LoadState(Stream *pstm)
{
	// Version check

	byte nVer = pstm->ReadByte();
	if (nVer != knVerStateMachineMgrState)
		return false;

	// Read msg count

	int cmsgs = pstm->ReadWord();

	// Read in messages, restore delivery time

	long tCurrent = m_ptimm->GetTickCount();
	while (cmsgs-- != 0) {
		Message msg;
		if (pstm->Read(&msg, sizeof(msg)) == 0)
			return false;
		dword ctDelivery = pstm->ReadDword();
		msg.tDelivery = tCurrent + ctDelivery;
		StoreDelayedMessage(&msg);
	}

	return pstm->IsSuccess();
}

bool StateMachineMgr::SaveState(Stream *pstm)
{
	// Save version

	pstm->WriteByte(knVerStateMachineMgrState);

	// Save count of messages

	int cmsgs = 0;
	DelayedMessage *pdmsg;
	for (pdmsg = m_pdmsgHead; pdmsg != NULL; pdmsg = pdmsg->pdmsgNext)
		cmsgs++;
	pstm->WriteWord(cmsgs);

	// Save messages and time delta remaining for each message.

	long tCurrent = m_ptimm->GetTickCount();
	for (pdmsg = m_pdmsgHead; pdmsg != NULL; pdmsg = pdmsg->pdmsgNext) {
		pstm->Write(&pdmsg->msg, sizeof(pdmsg->msg));
		pstm->WriteDword(pdmsg->msg.tDelivery - tCurrent);
	}

	return pstm->IsSuccess();
}

void StateMachineMgr::ClearDelayedMessages()
{
	DelayedMessage *pdmsg = m_pdmsgHead;
	while (pdmsg != NULL) {
		DelayedMessage *pdmsgNext = pdmsg->pdmsgNext;
		delete pdmsg;
		pdmsg = pdmsgNext;
	}
	m_pdmsgHead = NULL;
}

void StateMachineMgr::SendMsg(Message *pmsg)
{
	pmsg->tDelivery = 0;
	RouteMessage(pmsg);
}

void StateMachineMgr::SendMsg(MessageId mid, StateMachineId smidReceiver)
{
	Message msg;
	memset(&msg, 0, sizeof(msg));

	msg.mid = mid;
	msg.smidReceiver = smidReceiver;
	msg.smidSender = ksmidNull;

	RouteMessage(&msg);
}

void StateMachineMgr::SendMsg(MessageId mid, StateMachineId smidSender, StateMachineId smidReceiver)
{
	Message msg;
	memset(&msg, 0, sizeof(msg));

	msg.mid = mid;
	msg.smidSender = smidSender;
	msg.smidReceiver = smidReceiver;

	RouteMessage(&msg);
}

void StateMachineMgr::PostMsg(Message *pmsg)
{
	pmsg->tDelivery = 0;
	StoreDelayedMessage(pmsg);
}

void StateMachineMgr::PostMsg(MessageId mid, StateMachineId smidSender, StateMachineId smidReceiver)
{
	Message msg;
	memset(&msg, 0, sizeof(msg));

	msg.mid = mid;
	msg.smidSender = smidSender;
	msg.smidReceiver = smidReceiver;
	msg.tDelivery = 0;

	StoreDelayedMessage(&msg);
}

void StateMachineMgr::SendDelayedMsg(Message *pmsg, long tDelay)
{
	pmsg->tDelivery = m_ptimm->GetTickCount() + tDelay;
	RouteMessage(pmsg);
}

void StateMachineMgr::SendDelayedMsg(MessageId mid, long tDelay, StateMachineId smidSender, StateMachineId smidReceiver)
{
	Message msg;
	memset(&msg, 0, sizeof(msg));

	msg.mid = mid;
	msg.smidSender = smidSender;
	msg.smidReceiver = smidReceiver;
	msg.tDelivery = m_ptimm->GetTickCount() + tDelay;

	RouteMessage(&msg);
}

void StateMachineMgr::RouteMessage(Message *pmsg)
{
	extern int gcMessagesPerUpdate;
	gcMessagesPerUpdate++;

#ifdef TRACKSTATE
    // Track that this message has been sent
    extern StateFrame *gpframeCurrent;
    if (gpframeCurrent != NULL) {
        TrackState(gpframeCurrent, pmsg);
    }
#endif

	StateMachine *psm = GetStateMachine(pmsg->smidReceiver);

	// If receiver doesn't exist anymore, discard the message

	if (psm == NULL)
		return;

	if (pmsg->tDelivery > m_ptimm->GetTickCount()) {

		// This message needs to be stored until it's time to send it
		// UNDONE: this can fail due to out of memory

		StoreDelayedMessage(pmsg);
		return;
	}

    // If SetState() was called outside of ProcessStateMachineMessage()
    // handling, change the state before processing this message so that the
    // message is processed in the correct state

	if (psm->m_fForceStateChange)
		ProcessStateChange(psm);

	// See if this specific state handles this message

#if 0 // def DEBUG_HELPERS
	if (psm->m_fDebug && pmsg->mid != kmidReservedUpdate)
		DebugBreak();
#endif

	int nRet = psm->ProcessMessage(psm->m_st, pmsg);
	if (nRet == knDeleted)
		return;

	// If not, try the "global" handler

	if (nRet == knNotHandled)
		nRet = psm->ProcessMessage(kstReservedGlobal, pmsg);

	// If this state machine has been deleted, no further processing

	if (nRet == knDeleted)
		return;

	// Process state change if a state change occured as part of the previous
	// message handling.

	if (psm->m_fForceStateChange)
		ProcessStateChange(psm);
}

void StateMachineMgr::ProcessStateChange(StateMachine *psm)
{
#ifdef DEBUG
	int cStateChanges = 0;
#endif

	// Check for a state change

	while (psm->m_fForceStateChange) {
		
		// NOTE: Circular logic (state changes causing state changes) could cause
		// an infinite loop here. We don't attempt to protect against this.

#ifdef DEBUG
		Assert(cStateChanges < 3);
		cStateChanges++;
#endif

		// Create a general msg for initializing and cleaning up the state change

		Message msgT;
		msgT.smidSender = msgT.smidReceiver = GetId(psm);
		msgT.tDelivery = 0;

		psm->m_fForceStateChange = false;

		// Let the last state clean-up. 
		// NOTE: this purposefully does NOT bubble up to the Global state handler

		msgT.mid = kmidReservedExit;
		int nRet = psm->ProcessMessage(psm->m_st, &msgT);
		if (nRet == knDeleted)
			return;

		// We don't allow Exit handlers to change the state. 
//		Assert(!psm->m_fForceStateChange);
		psm->m_fForceStateChange = false;

		// Set the new state

		psm->m_st = psm->m_stNext;

		// Let the new state initialize
        // NOTE: this purposefully does NOT bubble up to the Global state
        // handler

		msgT.mid = kmidReservedEnter;
		nRet = psm->ProcessMessage(psm->m_st, &msgT);
		if (nRet == knDeleted)
			return;
	}
}

// This function is called every game tick

void StateMachineMgr::DispatchDelayedMessages()
{
	DelayedMessage **ppdmsg = &m_pdmsgHead;

	while (*ppdmsg != NULL) {
		DelayedMessage *pdmsg = *ppdmsg;
		if (pdmsg->msg.tDelivery <= m_ptimm->GetTickCount()) {
			*ppdmsg = pdmsg->pdmsgNext;
			RouteMessage(&pdmsg->msg);
			delete pdmsg;
		} else {
			ppdmsg = &(*ppdmsg)->pdmsgNext;
		}
	}
}

bool StateMachineMgr::StoreDelayedMessage(Message *pmsg)
{
	// Store this message (in some data structure) for later routing

	// A priority queue would be the ideal data structure (but not required)
	// to store the delayed messages - Check out Mark Nelson's article
	// "Priority Queues and the STL" in the January 1996 Dr. Dobb's Journal
	// http://www.dogma.net/markn/articles/pq_stl/priority.htm

	// Note: In main game loop call SendDelayedMessages() every game 
	// tick to check if it's time to send the stored messages

	DelayedMessage *pdmsg = new DelayedMessage;
	Assert(pdmsg != NULL, "out of memory!");
	if (pdmsg == NULL)
		return false;

	pdmsg->msg = *pmsg;
	pdmsg->pdmsgNext = NULL;

	DelayedMessage **ppdmsg = &m_pdmsgHead;

	while (*ppdmsg != NULL)
		ppdmsg = &(*ppdmsg)->pdmsgNext;
	*ppdmsg = pdmsg;

	return true;
}

#ifdef TRACKSTATE

dword gmpmidQuad[] = {
	'MRNL', // kmidReservedNull,
	'MREN', // kmidReservedEnter,
	'MREX', // kmidReservedExit,
	'MRUP', // kmidReservedUpdate,
	'MHIT', // kmidHit,
	'MNAH', // kmidNearbyAllyHit,
	'MDEL', // kmidDelete,
	'MENB', // kmidEnemyNearby,
	'MPLH', // kmidPowerLowHigh,
	'MMWN', // kmidMoveWaitingNearby,
	'MBUP', // kmidBeingUpgraded,
	'MUPC', // kmidUpgradeComplete,
	'MMVC', // kmidMoveCommand,
	'MATC', // kmidAttackCommand,
	'MANC', // kmidAnimationComplete,
	'MBOC', // kmidBuildOtherCommand,
	'MABO', // kmidAbortBuildOtherCommand,
	'MBDC', // kmidBuildComplete,
	'MFRC', // kmidFireComplete,
	'MSSM', // kmidSpawnSmoke,
	'MSDC', // kmidSelfDestructCommand,
	'MREC', // kmidRepairCommand,
	'MUPC', // kmidUpgradeCommand,
	'MAUC', // kmidAbortUpgradeCommand,
	'MTFC', // kmidTransformCommand,
	'MDLC', // kmidDeliverCommand,
	'MGXD', // kmidGalaxiteDelivery,
	'MMIC', // kmidMineCommand,
	'MMAC', // kmidMoveAction,
	'MAAC', // kmidAttackAction,
	'MGUA', // kmidGuardAction,
	'MGVA', // kmidGuardVicinityAction,
	'MGAA', // kmidGuardAreaAction,
	'MHEA', // kmidHuntEnemiesAction,
	'MFIR', // kmidFire,
	'MHET', // kmidHeal,
	'MSFX', // kmidPlaySfx,
    'PDIS', // kmidPlayerDisconnect,
};

void StateMachineMgr::TrackState(StateFrame *frame, Message *pmsg) {
    int i = frame->AddCountedValue(gmpmidQuad[pmsg->mid]);
    frame->AddValue('SMIS', (dword)pmsg->smidSender, i);
    frame->AddValue('SMIR', (dword)pmsg->smidReceiver, i);

    switch (pmsg->mid) {
    case kmidHit:
        frame->AddValue('GIDA', (dword)pmsg->Hit.gidAssailant, i);
        frame->AddValue('SIDA', (dword)pmsg->Hit.sideAssailant, i);
        frame->AddValue('NDAM', (dword)pmsg->Hit.nDamage, i);
        break;

    case kmidPlaySfx:
        // Output 0 so it is always the same, because some units that
        // send kmidPlaySfx select the sfx using SfxFromCategory, which
        // uses a random # generator that is not in sync with MP.
        // bug causing this.
        //frame->AddValue('SFX ', (dword)pmsg->PlaySfx.sfx, i);
        frame->AddValue('SFX ', 0, i);
        break;

    case kmidEnemyNearby:
        frame->AddValue('ENNB', (dword)pmsg->EnemyNearby.gidEnemy, i);
        break;

    case kmidMoveCommand:
        frame->AddValue('GIDT', (dword)pmsg->MoveCommand.gidTarget, i);
        frame->AddValue('TRWX', (dword)pmsg->MoveCommand.wptTarget.wx, i);
        frame->AddValue('TRWY', (dword)pmsg->MoveCommand.wptTarget.wy, i);
        frame->AddValue('TCWX', (dword)pmsg->MoveCommand.wptTargetCenter.wx, i);
        frame->AddValue('TCWY', (dword)pmsg->MoveCommand.wptTargetCenter.wy, i);
        frame->AddValue('TCRA', (dword)pmsg->MoveCommand.tcTargetRadius, i);
        frame->AddValue('MDPU', (dword)pmsg->MoveCommand.wcMoveDistPerUpdate,
                i);
        break;

    case kmidAttackCommand:
        frame->AddValue('GIDT', (dword)pmsg->AttackCommand.gidTarget, i);
        frame->AddValue('TRWX', (dword)pmsg->AttackCommand.wptTarget.wx, i);
        frame->AddValue('TRWY', (dword)pmsg->AttackCommand.wptTarget.wy, i);
        frame->AddValue('TCWX', (dword)pmsg->AttackCommand.wptTargetCenter.wx,
                i);
        frame->AddValue('TCWY', (dword)pmsg->AttackCommand.wptTargetCenter.wy,
                i);
        frame->AddValue('TCRA', (dword)pmsg->AttackCommand.tcTargetRadius, i);
        frame->AddValue('MDPU', (dword)pmsg->AttackCommand.wcMoveDistPerUpdate,
                i);
        break;

    case kmidUpgradeCommand:
        frame->AddValue('WFUP', (dword)pmsg->UpgradeCommand.wfUpgrade, i);
        break;

    case kmidBuildOtherCommand:
        frame->AddValue('UNTT', (dword)pmsg->BuildOtherCommand.ut, i);
        frame->AddValue('WPWX', (dword)pmsg->BuildOtherCommand.wpt.wx, i);
        frame->AddValue('WPWY', (dword)pmsg->BuildOtherCommand.wpt.wy, i);
        break;

    case kmidAbortBuildOtherCommand:
        frame->AddValue('UNTT', (dword)pmsg->AbortBuildOtherCommand.ut, i);
        break;

    case kmidDeliverCommand:
        frame->AddValue('GTAR', (dword)pmsg->DeliverCommand.gidTarget, i);
        break;

    case kmidMineCommand:
        frame->AddValue('GTAR', (dword)pmsg->MineCommand.gidTarget, i);
        frame->AddValue('TRWX', (dword)pmsg->MineCommand.wptTarget.wx, i);
        frame->AddValue('TRWY', (dword)pmsg->MineCommand.wptTarget.wy, i);
        break;

    case kmidGuardAreaAction:
        frame->AddValue('AREA', (dword)pmsg->GuardAreaCommand.nArea, i);
        break;

    case kmidHuntEnemiesAction:
        frame->AddValue('UMSK', (dword)pmsg->HuntEnemiesCommand.um, i);
        break;

    case kmidPlayerDisconnect:
        frame->AddValue('PID ', (dword)pmsg->PlayerDisconnect.pid, i);
        frame->AddValue('REAS', (dword)pmsg->PlayerDisconnect.nReason, i);
        break;
    }
}
#endif

#if defined(DEBUG_HELPERS)

// NOTE: these must be in the same order as the MessageId enum
// Note: Use LABEL macros instead of this

char *gaszMessageNames[] = {
	"Null",
	"Enter",
	"Exit",
	"Update",

	"Hit",
	"NearbyAllyHit",
	"Delete",
	"EnemyNearby",
	"PowerLowHigh",
	"MoveWaitingNearby",
	"BeginUpgraded",
	"UpgradeComplete",
	"MoveCommand",
	"AttackCommand",
	"AnimationComplete",
	"BuildOtherCommand",
	"AbortBuildOtherCommand",
	"BuildComplete",
	"FireComplete",
	"SpawnSmoke",
	"SelfDestructCommand",
	"RepairCommand",
	"UpgradeCommand",
	"AbortUpgradeCommand",
	"TransformCommand",
	"DeliverCommand",
	"GalaxiteDelivery",
	"MineCommand",

	"MoveAction",
	"AttackAction",
	"GuardAction",
	"GuardVicinityAction",
	"GuardAreaAction",
	"HuntEnemiesAction",
	
	"Fire",
	"Heal",
};

// NOTE: these must be in the same order as the State enum
// Note: Use LABEL macros instead of this

char *gaszStateNames[] = {
	"Null",
	"Zombie",
	"Global",

	"Guard",
	"Move",
	"Attack",
	"Chase",
	"Idle",
	"Dying",
	"BuildOtherCompleting",
	"BeingBuilt",
	"HuntEnemies",

	"ProcessorGetMiner",
	"ProcessorPutMiner",
	"ProcessorTakeGalaxite",

	"MinerMoveToProcessor",
	"MinerRotateForEntry",
	"MinerMine",
	"MinerFindGalaxite",
	"MinerFaceGalaxite",
	"MinerApproachGalaxite",
	"MinerSuck",
	"MinerStepAside",

	"ChangeStatePendingFireComplete",
	"ContinueActionPendingFireComplete",
};
#endif

} // namespace wi
