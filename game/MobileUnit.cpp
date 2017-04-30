#include "ht.h"

namespace wi {

// Each element of this table was calculated as:
// gawcDiagonalDist[i] = floor((cos(PI / 4) * i) + 0.5)

/* jscript to generate the table:
for (j = 0; j < 4; j++) {
	var str = "";
	for (i = 0; i < 16; i++)
		str += Math.floor((Math.cos(Math.PI / 4) * ((j * 16) + i) + .5)) + ", ";
	WScript.Echo(str);
}
*/
static byte gawcDiagonalDist[64] = {
	0, 1, 1, 2, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11,
	11, 12, 13, 13, 14, 15, 16, 16, 17, 18, 18, 19, 20, 21, 21, 22,
	23, 23, 24, 25, 25, 26, 27, 28, 28, 29, 30, 30, 31, 32, 33, 33,
	34, 35, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 45,
};

// Each element of this table was calculated as:
// gaDiv256byNWithRounding[i] = (256 + (i / 2)) / i

// The "+ (i / 2)" part of this rounds the # of steps rather than
// truncating it which gives gives better results when there is a partial
// step remainder.

/* javascript to generate the table:
for (j = 0; j < 4; j++) {
	var str = "";
	for (i = 0; i < 16; i++) {
		var wcMoveDist = ((j * 16) + i);
		str += Math.floor((256 + (wcMoveDist / 2)) / wcMoveDist) + ", ";
	}
	WScript.Echo(str);
}
*/
static byte gaDiv256byNWithRounding[64] = {
	0, 255, // Fudge to keep within 8-bit range
	128, 85, 64, 51, 43, 37, 32, 28, 26, 23, 21, 20, 18, 17,
	16, 15, 14, 13, 13, 12, 12, 11, 11, 10, 10, 9, 9, 9, 9, 8,
	8, 8, 8, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4,
};

int g_mpDirToDx[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
int g_mpDirToDy[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
AnimationData *g_panidMoveTarget;
static AnimationData *s_panidVehicleExplosion;

const int kcFireCountdown = 6;	// 6 updates (~.5 secs)

Path *MobileUnitGob::s_apathCached[kcPathsCache];
int MobileUnitGob::s_cpathCached;

int Dir16ToDx(Direction16 dir16)
{
    return g_mpDirToDx[dir16 / 2];
}

int Dir16ToDy(Direction16 dir16)
{
    return g_mpDirToDy[dir16 / 2];
}

bool Dir16IsDiagonal(Direction16 dir16)
{
    return (dir16 != kdir16N && dir16 != kdir16E &&
        dir16 != kdir16S && dir16 != kdir16W);
}

//===========================================================================
// MobileUnitGob implementation

bool MobileUnitGob::InitClass(MobileUnitConsts *pmuntc, IniReader *pini)
{
	if (!UnitGob::InitClass(pmuntc, pini))
		return false;

	char szTemplate[10];
	itoa(pmuntc->gt, szTemplate, 10);

	// Required properties

	int nT;
	if (pini->GetPropertyValue(szTemplate, "MoveRate", "%d", &nT) != 1)
		return false;
	pmuntc->wcMoveDistPerUpdate = (WCoord)nT;
	Assert(pmuntc->wcMoveDistPerUpdate < 64);	// gawcDiagonalDist only has 64 entries
	gwcMoveDistPerUpdateMin = _min(gwcMoveDistPerUpdateMin, pmuntc->wcMoveDistPerUpdate);
	gwcMoveDistPerUpdateMax = _max(gwcMoveDistPerUpdateMax, pmuntc->wcMoveDistPerUpdate);

	int nArmorStrength;
	if (pini->GetPropertyValue(szTemplate, "ArmorStrength", "%d", &nArmorStrength) != 1)
		return false;
	Assert(nArmorStrength < 1 << 10);
	pmuntc->fxArmorStrength = itofx(nArmorStrength);
	gfxMobileUnitArmorStrengthMin = _min(gfxMobileUnitArmorStrengthMin, pmuntc->fxArmorStrength);
	gfxMobileUnitArmorStrengthMax = _max(gfxMobileUnitArmorStrengthMax, pmuntc->fxArmorStrength);

	// Optional properties

	if (pini->GetPropertyValue(szTemplate, "ArmorStrengthMP", "%d", &nArmorStrength) != 1) {
		pmuntc->fxArmorStrengthMP = pmuntc->fxArmorStrength;
	} else {
		Assert(nArmorStrength < 1 << 10);
		pmuntc->fxArmorStrengthMP = itofx(nArmorStrength);
	}

	if (pini->GetPropertyValue(szTemplate, "MoveRateMP", "%d", &nT) != 1)
		pmuntc->wcMoveDistPerUpdateMP = pmuntc->wcMoveDistPerUpdate;
	else
		pmuntc->wcMoveDistPerUpdateMP = (WCoord)nT;
	Assert(pmuntc->wcMoveDistPerUpdateMP < 64);	// gawcDiagonalDist only has 64 entries

	// Preload the unit's menu form

	if (!LoadMenu(pmuntc, pini, szTemplate, kidfUnitMenu))
		return false;

	// MobileUnitGob owns the 'move target' and 'vehicle explosion' AnimationData

	if (g_panidMoveTarget == NULL) {
		g_panidMoveTarget = LoadAnimationData("movetarget.anir");
		if (g_panidMoveTarget == NULL)
			return false;
	}

	if (s_panidVehicleExplosion == NULL) {
		s_panidVehicleExplosion = LoadAnimationData("vexplosion.anir");
		if (s_panidVehicleExplosion == NULL)
			return false;
	}

	return true;
}

void MobileUnitGob::ExitClass(MobileUnitConsts *pmuntc)
{
	// Clear this out so MobileUnitGob derivatives can safely call MobileUnitGob::ExitClass

	delete s_panidVehicleExplosion;
	s_panidVehicleExplosion = NULL;

	// Clear this out so MobileUnitGob derivatives can safely call MobileUnitGob::ExitClass

	delete g_panidMoveTarget;
	g_panidMoveTarget = NULL;

	UnitGob::ExitClass(pmuntc);
}

MobileUnitGob::MobileUnitGob(MobileUnitConsts *pmuntc) : UnitGob(pmuntc)
{
	m_ff |= kfGobMobileUnit;
	m_tLastFire = 0;
	m_gidTarget = kgidNull;
	m_wptTarget.wx = kwxInvalid;		// kxInvalid = no target location
	m_cMoveStepsRemaining = 0;
	m_txDst = 0;
	m_tyDst = 0;
	m_wcMoveDistPerUpdate = 0;
	m_mua = kmuaNone;
	m_muaPending = kmuaNone;
	m_wfMunt = kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding | kfMuntAttackEnemiesWhenMoving;
	m_stPending = kstReservedNull;
	m_itptPath = 0;
	m_ppathUnit = NULL;
	m_ppathAvoid = NULL;
	m_nSeqMoveAside = 0;

	// Just to be clear this Unit or its nearby allies hasn't been hit for awhile

	m_cupdLastHitOrNearbyAllyHit = -1000000;
}

MobileUnitGob::~MobileUnitGob()
{
	delete m_ppathUnit;
}

void MobileUnitGob::Activate()
{
	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);
	Assert(!IsTileReserved(tx, ty));
	ReserveTile(tx, ty, true);
	UnitGob::Activate();
	ggobm.MoveGobBetweenAreas(m_gid, 0, ggobm.CalcAreaMask(tx, ty));
}

void MobileUnitGob::Deactivate()
{
	if (m_wfMunt & kfMuntDestinationReserved) {
		Assert(IsTileReserved(m_txDst, m_tyDst));
		ReserveTile(m_txDst, m_tyDst, false);
	} else {
#ifndef MP_DEBUG_SHAREDMEM
// DWM: We always hit it when a Processor containing a Bullpup is destroyed.
// We should fix that but in the meantime I need to release a DEBUG build
// that won't trip up on this case so I'm commenting it out for now.
//		Assert(IsTileReserved(TcFromWc(m_wx), TcFromWc(m_wy)));
#endif
		ReserveTile(TcFromWc(m_wx), TcFromWc(m_wy), false);
	}
	UnitGob::Deactivate();
	ggobm.MoveGobBetweenAreas(m_gid, ggobm.CalcAreaMask(TcFromWc(m_wx), TcFromWc(m_wy)), 0);
}

word AggBitsFromAgg(int nAggressiveness)
{
	switch (nAggressiveness) {
	case knAggressivenessCoward:
		return kfMuntRunAwayWhenHit;

	case knAggressivenessSelfDefense:
		return kfMuntReturnFire | kfMuntStayPut;

	case knAggressivenessDefender:
		return kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding | kfMuntAttackEnemiesWhenMoving;

	case knAggressivenessPitbull:
		return kfMuntReturnFire | kfMuntAttackEnemiesWhenGuarding | kfMuntAttackEnemiesWhenMoving | kfMuntChaseEnemies;

// Implicit
//		case knAggressivenessPacifist:
//			break; // non-aggressive
	}

	return 0;
}

bool MobileUnitGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	// UnitGob::Init(pini, ...) calls the overridden UnitGob::Init(wx, ...) below

	if (!UnitGob::Init(pini, pfind, pszName))
		return false;

	// Note that this MobileUnitGob has already been Activated by this point

	// Translate an aggressiveness type into the MobileUnit flags that actually
	// determine its behaviour.

	int nAggressiveness;
	char szAction[100];
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%*d ,%*d ,%*d ,%*d ,%*d, %d ,%a", &nAggressiveness, szAction);
	if (cArgs > 0)
		m_wfMunt = (m_wfMunt & ~kfMuntAggressivenessBits) | AggBitsFromAgg(nAggressiveness);

	// Only computer-controlled units respond to their initial Action

	if (m_pplr->GetFlags() & kfPlrComputer) {
		if (cArgs > 1)
			PerformAction(szAction);

	// Human-controlled units don't attack while executing a move command

	} else {
		m_wfMunt &= ~kfMuntAttackEnemiesWhenMoving;
	}

	return true;
}

bool MobileUnitGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	if (!UnitGob::Init(wx, wy, pplr, fxHealth, ff, pszName))
		return false;

	// Center Unit within tile

	m_wx += kwcTileHalf;
	m_wy += kwcTileHalf;

	Activate();

	// Human-controlled units don't attack while executing a move command

	if (IsHumanOrGodControlled())
		m_wfMunt &= ~kfMuntAttackEnemiesWhenMoving;

	// Notify nearby enemy gobs that they might want to attack this gob

	NotifyEnemyNearby();

	return true;
}

#define knVerMobileUnitGobState 10
bool MobileUnitGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerMobileUnitGobState)
		return false;
	m_dir = (Direction)pstm->ReadByte();
	m_dirNext = (Direction)pstm->ReadByte();
	m_gidTarget = pstm->ReadWord();
	m_tLastFire = gsim.GetTickCount() - pstm->ReadDword();
	pstm->Read(&m_msgPending, sizeof(m_msgPending));
	pstm->Read(&m_msgAction, sizeof(m_msgAction));
	m_cCountdown = pstm->ReadWord();
	m_cMoveStepsRemaining = pstm->ReadWord();
	m_tptChaseInitial.tx = pstm->ReadWord();
	m_tptChaseInitial.ty = pstm->ReadWord();
#ifdef DRAW_PATHS
	m_wxDst = pstm->ReadWord();
	m_wyDst = pstm->ReadWord();
#endif
	m_txDst = pstm->ReadWord();
	m_tyDst = pstm->ReadWord();
	m_wptTarget.wx = pstm->ReadWord();
	m_wptTarget.wy = pstm->ReadWord();
	if (pstm->ReadByte() != 0) {
		m_ppathUnit = new Path;
		if (m_ppathUnit == NULL)
			return false;
		if (!m_ppathUnit->LoadState(gsim.GetLevel()->GetTerrainMap(), pstm))
			return false;
		m_itptPath = pstm->ReadWord();
	}
	if (pstm->ReadByte() != 0) {
		m_ppathAvoid = new Path;
		if (m_ppathAvoid == NULL)
			return false;
		if (!m_ppathAvoid->LoadState(gsim.GetLevel()->GetTerrainMap(), pstm))
			return false;
	}
	m_mua = (MobileUnitAction)pstm->ReadByte();
	m_muaPending = (MobileUnitAction)pstm->ReadByte();
	m_wfMunt = pstm->ReadWord();
	m_stPending = (State)pstm->ReadByte();
	m_wptTargetCenter.wx = pstm->ReadWord();
	m_wptTargetCenter.wy = pstm->ReadWord();
	m_tcTargetRadius = pstm->ReadWord();
	m_wcMoveDistPerUpdate = pstm->ReadWord();
	m_cupdLastHitOrNearbyAllyHit = pstm->ReadDword();
	return UnitGob::LoadState(pstm);
}

bool MobileUnitGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerMobileUnitGobState);
	pstm->WriteByte(m_dir);
	pstm->WriteByte(m_dirNext);
	pstm->WriteWord(m_gidTarget);
	pstm->WriteDword((dword)(gsim.GetTickCount() - m_tLastFire));
	pstm->Write(&m_msgPending, sizeof(m_msgPending));
	pstm->Write(&m_msgAction, sizeof(m_msgAction));
	pstm->WriteWord(m_cCountdown);
	pstm->WriteWord(m_cMoveStepsRemaining);
	pstm->WriteWord(m_tptChaseInitial.tx);
	pstm->WriteWord(m_tptChaseInitial.ty);
#ifdef DRAW_PATHS
	pstm->WriteWord(m_wxDst);
	pstm->WriteWord(m_wyDst);
#endif
	pstm->WriteWord(m_txDst);
	pstm->WriteWord(m_tyDst);
	pstm->WriteWord(m_wptTarget.wx);
	pstm->WriteWord(m_wptTarget.wy);
	if (m_ppathUnit == NULL) {
		pstm->WriteByte(0);
	} else {
		pstm->WriteByte(1);
		m_ppathUnit->SaveState(pstm);
		pstm->WriteWord(m_itptPath);
	}
	if (m_ppathAvoid == NULL) {
		pstm->WriteByte(0);
	} else {
		pstm->WriteByte(1);
		m_ppathAvoid->SaveState(pstm);
	}
	pstm->WriteByte(m_mua);
	pstm->WriteByte(m_muaPending);
	pstm->WriteWord(m_wfMunt);
	pstm->WriteByte(m_stPending);
	pstm->WriteWord(m_wptTargetCenter.wx);
	pstm->WriteWord(m_wptTargetCenter.wy);
	pstm->WriteWord(m_tcTargetRadius);
	pstm->WriteWord(m_wcMoveDistPerUpdate);
	pstm->WriteDword(m_cupdLastHitOrNearbyAllyHit);
	return UnitGob::SaveState(pstm);
}

void MobileUnitGob::PerformAction(char *szAction)
{
	int nUnitAction;
	if (IniScanf(szAction, "%d", &nUnitAction) == 0) {
		Assert(false);
		return;
	}

	switch (nUnitAction) {
	case knMoveUnitAction:
		{
			int nArea;
			if (IniScanf(szAction, "%*d ,%d", &nArea) == 0) {
				Assert(false);
				return;
			}
			TRect trc;
			ggobm.GetAreaRect(nArea, &trc);
			Point ptCenter;
			trc.GetCenter(&ptCenter);
			SendMoveAction(m_gid, WcFromTc(ptCenter.x), WcFromTc(ptCenter.y), 1, m_pmuntc->GetMoveDistPerUpdate());
		}
		break;

	case knGuardUnitAction:
		break;

	case knGuardVicinityUnitAction:
		SendGuardVicinityAction(m_gid);
		break;

	case knGuardAreaUnitAction:
		{
			int nArea;
			if (IniScanf(szAction, "%*d ,%d", &nArea) == 0) {
				Assert(false);
				return;
			}
			SendGuardAreaAction(m_gid, nArea);
		}
		break;

	case knHuntEnemiesUnitAction:
		{
			UnitMask um;
			if (IniScanf(szAction, "%*d ,%d", &um) == 0) {
				Assert(false);
				return;
			}
			SendHuntEnemiesAction(m_gid, um);
		}
		break;
	}
}

#ifdef DRAW_PATHS
void MobileUnitGob::DrawPath(DibBitmap *pbm, WCoord wxViewOrigin, WCoord wyViewOrigin)
{
	if (m_ppathUnit != NULL)
		m_ppathUnit->Draw(pbm, PcFromWc(wxViewOrigin), PcFromWc(wyViewOrigin), GetSide());
}
#endif

#ifdef DRAW_LINES
void MobileUnitGob::DrawTargetLine(DibBitmap *pbm, int xViewOrigin, int yViewOrigin)
{
	if (m_wptTarget.wx == kwxInvalid)
		return;

	pbm->DrawLine(PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin, 
			PcFromUwc(m_wptTarget.wx) - xViewOrigin, PcFromUwc(m_wptTarget.wy) - yViewOrigin,
			GetSideColor(m_pplr->GetSide()));
}
#endif

bool MobileUnitGob::IsIdle()
{
	// UNDONE: introduce kstIdle?
	return (m_wfMunt & kfMuntCommandPending) == 0 && m_st == kstGuard;
}

// Handle rotating toward the target and limiting the firing rate

bool MobileUnitGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// Make sure we're facing the way we want to fire before we try to fire

	Direction16 dirFire = CalcDir16(wdx, wdy);
	if (m_dir != dirFire) {
		m_dir = TurnToward16(dirFire, m_dir);
		SetAnimationStrip(&m_ani, m_pmuntc->anMovingStripIndices[m_dir]);
		m_unvl.MinSkip();
		return false;
	}

	// Firing rate is limited by ctFiringRate

	long t = gsim.GetTickCount();

	int ctWait = m_pmuntc->ctFiringRate;
	int ctRemaining = ctWait - (int)(t - m_tLastFire);
	if (ctRemaining > 0) {
		m_unvl.MinSkip((ctRemaining + (kctUpdate / 2)) / kctUpdate - 1);
		return false;
	}

	m_tLastFire = t;

	// Play firing animation (start on frame 1 where the action is)

	StartAnimation(&m_ani, m_pmuntc->anFiringStripIndices[m_dir], 1, kfAniIgnoreFirstAdvance | kfAniResetWhenDone);
	m_wfMunt |= kfMuntFiring;
	gsmm.SendDelayedMsg(kmidFireComplete, m_ani.GetRemainingStripTime(), m_gid, m_gid);

	return true;
}

void MobileUnitGob::Idle()
{
}

void MobileUnitGob::SetTarget(Gid gid, WCoord wx, WCoord wy, WCoord wxCenter, WCoord wyCenter, TCoord tcRadius, WCoord wcMoveDistPerUpdate)
{
	if (gid == kgidNull) {
		// Send formed command

		Message msgT;
		msgT.mid = kmidMoveCommand;
		msgT.smidSender = kgidNull;
		msgT.smidReceiver = m_gid;
		msgT.MoveCommand.wptTarget.wx = wx;
		msgT.MoveCommand.wptTarget.wy = wy;
		msgT.MoveCommand.gidTarget = kgidNull;
		msgT.MoveCommand.wptTargetCenter.wx = wxCenter;
		msgT.MoveCommand.wptTargetCenter.wy = wyCenter;
		msgT.MoveCommand.tcTargetRadius = tcRadius;
		msgT.MoveCommand.wcMoveDistPerUpdate = wcMoveDistPerUpdate;
		gcmdq.Enqueue(&msgT);
		return;
	}

	// If the target no longer exists, discard the command

	Gob *pgobTarget = ggobm.GetGob(gid);
	if (pgobTarget == NULL)
		return;

	// If the target is a Replicator move to its input

	if (pgobTarget->GetType() == kgtReplicator) {
		pgobTarget->SetFlags(pgobTarget->GetFlags() | kfGobFlashing);

		// Send formed command

		ReplicatorGob *prep = (ReplicatorGob *)pgobTarget;
		TPoint tpt;
		prep->GetInputTilePosition(&tpt);
		Message msgT;
		msgT.mid = kmidMoveCommand;
		msgT.smidSender = kgidNull;
		msgT.smidReceiver = m_gid;
		msgT.MoveCommand.wptTarget.wx = WcFromTc(tpt.tx) + kwcTileHalf;
		msgT.MoveCommand.wptTarget.wy = WcFromTc(tpt.ty) + kwcTileHalf;
		msgT.MoveCommand.gidTarget = kgidNull;
		msgT.MoveCommand.wptTargetCenter.wx = msgT.MoveCommand.wptTarget.wx;
		msgT.MoveCommand.wptTargetCenter.wy = msgT.MoveCommand.wptTarget.wy;
		msgT.MoveCommand.tcTargetRadius = 0;
		msgT.MoveCommand.wcMoveDistPerUpdate = wcMoveDistPerUpdate;
		gcmdq.Enqueue(&msgT);
		return;
	}

	// Get target coords if none passed in

	if (wx == 0 && wy == 0) {
		Assert(pgobTarget->GetFlags() & kfGobUnit);
		UnitGob *puntTarget = (UnitGob *)pgobTarget;
		WPoint wpt;
		puntTarget->GetAttackPoint(&wpt);
		wx = wpt.wx;
		wy = wpt.wy;
	}

	// Friend or Foe? No special action for friends

	if (IsAlly(pgobTarget->GetSide()))
		return;

	// Foe -- attack!
	// Flash the target Gob

	if (m_pplr == gpplrLocal)
		pgobTarget->Flash();

	// Queue attack command

	Message msgT;
	msgT.mid = kmidAttackCommand;
	msgT.smidSender = m_gid;
	msgT.smidReceiver = m_gid;
	msgT.AttackCommand.wptTarget.wx = wx;
	msgT.AttackCommand.wptTarget.wy = wy;
	msgT.AttackCommand.gidTarget = gid;
	msgT.AttackCommand.wptTargetCenter.wx = wxCenter;
	msgT.AttackCommand.wptTargetCenter.wy = wyCenter;
	msgT.AttackCommand.tcTargetRadius = tcRadius;
	msgT.AttackCommand.wcMoveDistPerUpdate = wcMoveDistPerUpdate;
	gcmdq.Enqueue(&msgT);
}

void SendAttackCommand(Gid gidReceiver, Gid gidTarget) 
{
	// Bail if target is already gone

	Gob *pgobTarget = ggobm.GetGob(gidTarget);
	if (pgobTarget == NULL)
		return;
	Assert(pgobTarget->GetFlags() & kfGobUnit);
	UnitGob *puntTarget = (UnitGob *)pgobTarget;

	Message msgT;
	msgT.mid = kmidAttackCommand;
	msgT.smidSender = gidReceiver;
	msgT.smidReceiver = gidReceiver;
	puntTarget->GetAttackPoint(&msgT.AttackCommand.wptTarget);
	msgT.AttackCommand.gidTarget = gidTarget;
	msgT.AttackCommand.tcTargetRadius = 0;
	msgT.AttackCommand.wptTargetCenter = msgT.AttackCommand.wptTarget;
	msgT.AttackCommand.wcMoveDistPerUpdate = 0;
	gsmm.SendMsg(&msgT);
}

void MobileUnitGob::GetAttackPoint(WPoint *pwpt)
{
	// The attack point must be terrain accessible. 

	if (m_wfMunt & kfMuntDestinationReserved) {
		pwpt->wx = WcFromTc(m_txDst);
		pwpt->wy = WcFromTc(m_tyDst);
	} else {
		pwpt->wx = m_wx;
		pwpt->wy = m_wy;
	}
}

// UNDONE: so far behavior can be easily parameterized
// Parameters:
// target hit animation (piff) (optional)
// move animation (w/ table for 8 directions)
// fire animation (w/ table for 8 directions)
// idle animation (w/ table for 8 directions)
// death animation
// ShotGob (optional) -- NOTE: may handle target hit animation
// other stuff already in MobileUnitConsts

// If a path is being followed we have to wait until a tile center is
// reached before acting on a new move command.
// Returns true if command is processed (new state is set)

bool MobileUnitGob::PendOrProcessCommand(Message *pmsg, State stNew)
{
	if (!IsReadyForCommand()) {
		m_msgPending = *pmsg;
		m_wfMunt |= kfMuntCommandPending;
		return false;
	} else {
		m_gidTarget = kgidNull;
		SetState(stNew);
		return true;
	}
}

bool MobileUnitGob::PendOrProcessAction(Message *pmsg, State stNew, MobileUnitAction mua)
{
	if (!IsReadyForCommand()) {
		m_muaPending = mua;
		m_msgPending = *pmsg;
		m_wfMunt |= kfMuntCommandPending;
		return false;
	} else {
		m_mua = mua;
		m_gidTarget = kgidNull;
		m_msgAction = *pmsg;
		SetState(stNew);
		return true;
	}
}

bool MobileUnitGob::IsTargetInRange()
{
	Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
	if (pgobTarget == NULL)
		return false;
    return IsGobWithinRange(pgobTarget, m_pmuntc->tcFiringRange);
}

bool MobileUnitGob::IsStandingOnActivator()
{
	for (Gid gid = ggobm.GetFirstGid(TcFromWc(m_wx), TcFromWc(m_wy)); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
		Gob *pgob = ggobm.GetGob(gid, false);
		if (pgob == NULL)
			continue;
		if (pgob->GetType() == kgtActivator)
			return true;
	}

	return false;
}

void MobileUnitGob::SetStatePendingFireComplete(State st)
{
	if (m_wfMunt & kfMuntFiring) {
		m_stPending = st;
		SetState(kstChangeStatePendingFireComplete);
	} else {
		SetState(st);
	}
}

void MobileUnitGob::ContinueActionPendingFireComplete()
{
	if (m_wfMunt & kfMuntFiring) {
		SetState(kstContinueActionPendingFireComplete);
	} else {
		gsmm.SendMsg(&m_msgAction);
	}
}

bool MobileUnitGob::IsAttackPointWithinFiringRangeOfTarget(UnitGob *puntTarget)
{
	WPoint wptAttack;
	puntTarget->GetAttackPoint(&wptAttack);
	return IsTargetWithinRange(&wptAttack, puntTarget, m_pmuntc->tcFiringRange);
}

int MobileUnitGob::GetIdleCountdown()
{
    return (GetRandom() % 50) + 50; // somewhere between 4 & 8 seconds
}

// TUNE:

const TCoord ktcVicinity = 5;
const long kcupdAggressivenessBoost = 63; // 5 seconds

int MobileUnitGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnEnter
		m_dir = kdir16S;
		SetState(kstGuard);

	// Actions are sent by Trigger/UnitActions or the Overmind

	OnMsg(kmidMoveAction)
		if (PendOrProcessAction(pmsg, kstMove, kmuaMove))
			goto lbMoveCommand;

	OnMsg(kmidAttackAction)
		if (PendOrProcessAction(pmsg, kstAttack, kmuaAttack))
			goto lbAttackCommand;

	OnMsg(kmidGuardAction)
		PendOrProcessAction(pmsg, kstGuard, kmuaGuard);

	OnMsg(kmidGuardVicinityAction)
		PendOrProcessAction(pmsg, kstGuard, kmuaGuardVicinity);

	OnMsg(kmidGuardAreaAction)
		PendOrProcessAction(pmsg, kstGuard, kmuaGuardArea);

	OnMsg(kmidHuntEnemiesAction)
		PendOrProcessAction(pmsg, kstHuntEnemies, kmuaHuntEnemies);

	// Commands are the result of player interactions with the SimUI

	OnMsg(kmidMoveCommand)
		if (PendOrProcessCommand(pmsg, kstMove)) {
lbMoveCommand:
			// Stash move parameters to be picked up by the move state
			// Adjust target x/y to point to the closest tile center. We like tile centers.

			m_wptTarget.wx = WcTrunc(pmsg->MoveCommand.wptTarget.wx) + kwcTileHalf;
			m_wptTarget.wy = WcTrunc(pmsg->MoveCommand.wptTarget.wy) + kwcTileHalf;
			m_wptTargetCenter.wx = WcTrunc(pmsg->MoveCommand.wptTargetCenter.wx) + kwcTileHalf;
			m_wptTargetCenter.wy = WcTrunc(pmsg->MoveCommand.wptTargetCenter.wy) + kwcTileHalf;
			m_tcTargetRadius = pmsg->MoveCommand.tcTargetRadius;
			m_wcMoveDistPerUpdate = pmsg->MoveCommand.wcMoveDistPerUpdate;
		}

	OnMsg(kmidAttackCommand)
		if (PendOrProcessCommand(pmsg, kstAttack)) {
lbAttackCommand:
			// stash attack parameters to be picked up by the attack state

			m_gidTarget = pmsg->AttackCommand.gidTarget;
			m_wptTarget.wx = WcTrunc(pmsg->AttackCommand.wptTarget.wx) + kwcTileHalf;
			m_wptTarget.wy = WcTrunc(pmsg->AttackCommand.wptTarget.wy) + kwcTileHalf;
			m_wptTargetCenter.wx = WcTrunc(pmsg->AttackCommand.wptTargetCenter.wx) + kwcTileHalf;
			m_wptTargetCenter.wy = WcTrunc(pmsg->AttackCommand.wptTargetCenter.wy) + kwcTileHalf;
			m_tcTargetRadius = pmsg->AttackCommand.tcTargetRadius;
			m_wcMoveDistPerUpdate = pmsg->AttackCommand.wcMoveDistPerUpdate;
		}

	// Other messages are inter and intra-Unit communication, sometimes delayed

	OnMsg(kmidNearbyAllyHit)
		m_cupdLastHitOrNearbyAllyHit = gsim.GetUpdateCount();

		// Respond pretty much as if self was the one hit

		goto lbHitNoDamage;

	OnMsg(kmidHit)
		m_cupdLastHitOrNearbyAllyHit = gsim.GetUpdateCount();

		// apply damage
		// NOTE: This scoping is for the benefit of gcc which is concerned about
		// the above goto skipping the initialization of fxDamage

		{
			fix fxDamage = itofx(pmsg->Hit.nDamage);
			if (m_pplr->GetHandicap() & kfHcapIncreasedArmor)
				fxDamage = (fix)mulfx(fxDamage, (itofx(knDecreasedDamagePercent) / 100));
			SetHealth(subfx(m_fxHealth, fxDamage));
		}

		if (m_fxHealth <= 0) {
			Player *pplr = gplrm.GetPlayer(pmsg->Hit.sideAssailant);
			pplr->IncEnemyMobileUnitsKilled();
			m_pplr->IncMobileUnitsLost();

			SetState(kstDying);

		// Think about fighting back

		} else {
			ShowDamageIndicator();

lbHitNoDamage:
			// UNDONE: knAggressivenessCoward
			if (m_wfMunt & kfMuntRunAwayWhenHit) {

			
			// If we're supposed to return fire and we know who fired at us...

			} else if ((m_wfMunt & kfMuntReturnFire) && pmsg->Hit.gidAssailant != kgidNull) {

				// and we're not already attacking an in-range target...

				if (m_gidTarget == kgidNull || !IsTargetInRange()) {

					// and we're not on a human directed mission...

					if ((m_pplr->GetFlags() & kfPlrComputer) || (m_st == kstGuard)) {

						// Fight back!
	
						SendAttackCommand(m_gid, pmsg->Hit.gidAssailant);
					}
				}
			}
		}

		// Because this logic is reused by the kmidNearbyAllyHit handler

		if (pmsg->mid != kmidNearbyAllyHit)
			NotifyNearbyAlliesOfHit(pmsg->Hit.gidAssailant);

	OnMsg(kmidEnemyNearby)
		// Notification that an enemy is nearby

		RememberEnemyNearby(pmsg->EnemyNearby.gidEnemy);

		// Wake up and check out the enemy

		m_unvl.MinSkip();

	OnMsg(kmidMoveWaitingNearby)
        // We're waiting on a gob that has decided to either go into transition
        // or stop Wake up and check what to do

		m_unvl.MinSkip();

	OnMsg(kmidFireComplete)
		m_wfMunt &= ~kfMuntFiring;
		m_unvl.MinSkip();

	OnMsg(kmidDelete)
		Assert("Shouldn't receive kmidDelete when not in kstDying state");

	//-----------------------------------------------------------------------

	State(kstGuard)
		OnEnter
			// Play idle animation

			StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniDone);
			m_wptTarget.wx = kwxInvalid;
			m_gidTarget = kgidNull;
			m_cCountdown = GetIdleCountdown();
			if (m_wfMunt & kfMuntMoveWaitingNearby)
				NotifyMoveWaitingNearby(m_wx, m_wy);
			Assert(!(m_wfMunt & (kfMuntPathPending | kfMuntMoveWaitingNearby)));

		OnUpdate
			// Note if an enemy is nearby the update interval is min-ed 
			// by the enemy nearby notification, causing this path
			// to execute right away. Otherwise most of the time the
			// state machine is sleeping while in guard state.

			// A command might have been pended while we waited for the
			// firing animation to complete.

			if (m_wfMunt & kfMuntCommandPending && IsReadyForCommand()) {
				m_wfMunt &= ~kfMuntCommandPending;
				gsmm.SendMsg(&m_msgPending);
				return knHandled;
			}

			// if enemy in range and we're not already firing at something

			if ((m_wfMunt & (kfMuntAttackEnemiesWhenGuarding | kfMuntFiring)) == kfMuntAttackEnemiesWhenGuarding) {
				TCoord tcSightRange;
				if (m_mua == kmuaGuardVicinity)
					tcSightRange = ktcVicinity;
				else
					tcSightRange = m_pmuntc->tcFiringRange;
				UnitGob *puntTarget = FindEnemyNearby(tcSightRange);
				if (puntTarget != NULL) {
					if (m_pplr->GetFlags() & kfPlrComputer) {
						puntTarget->GetAttackPoint(&m_wptTarget);
						m_gidTarget = puntTarget->GetId();
						m_wptTargetCenter = m_wptTarget;
						m_tcTargetRadius = 0;
						m_wcMoveDistPerUpdate = m_pmuntc->GetMoveDistPerUpdate();
						SetStatePendingFireComplete(kstAttack);

						// Pretend we're hit so nearby units will come help

						NotifyNearbyAlliesOfHit(m_gidTarget);
						return knHandled;
					}

					// Play firing animation

					puntTarget->GetCenter(&m_wptTarget);
					Fire(puntTarget, m_wptTarget.wx, m_wptTarget.wy, m_wptTarget.wx - m_wx, m_wptTarget.wy - m_wy);
					m_cCountdown = GetIdleCountdown();

					// Pretend we're hit so nearby units will come help

					NotifyNearbyAlliesOfHit(puntTarget->GetId());

				} else if (m_mua == kmuaGuardArea) {
					// Find a valid target

					UnitGob *puntTarget = FindValidTargetInArea(m_msgAction.GuardAreaCommand.nArea);
					if (puntTarget != NULL) {
						puntTarget->GetAttackPoint(&m_wptTarget);
						m_gidTarget = puntTarget->GetId();
						m_wptTargetCenter = m_wptTarget;
						m_tcTargetRadius = 0;
						m_wcMoveDistPerUpdate = m_pmuntc->GetMoveDistPerUpdate();
						SetStatePendingFireComplete(kstAttack);
						return knHandled;
					}

				} else {
					m_wptTarget.wx = kwxInvalid;
					m_gidTarget = kgidNull;
				}
			}

			// Animate (idle or firing)

			AdvanceAnimation(&m_ani);

			m_cCountdown -= m_unvl.GetUpdateCount();
			if (m_cCountdown < 0) {
				Idle();
				m_cCountdown = GetIdleCountdown();
			}
			m_unvl.MinSkip(m_cCountdown);

			// Handle flashing

			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstMove)
		OnEnter
			MoveEnter();

		OnExit
			MoveExit();

		OnUpdate
			if (!(m_wfMunt & kfMuntCommandPending) && !InTransition()) {

				// If the Unit is sufficiently aggressive and sees an enemy 
				// unit it should attack it.

				if (m_wfMunt & kfMuntAttackEnemiesWhenMoving) {
					UnitGob *puntTarget = FindEnemyNearby(m_pmuntc->tcFiringRange);
					if (puntTarget != NULL) {
						m_gidTarget = puntTarget->GetId();
						puntTarget->GetAttackPoint(&m_wptTarget);
						m_wptTargetCenter = m_wptTarget;
						m_tcTargetRadius = 0;
						m_wcMoveDistPerUpdate = m_pmuntc->GetMoveDistPerUpdate();
						SetState(kstAttack);
						return knHandled;
					}
				}
			}

			switch (MoveUpdate()) {
			case knMoveTargetReached:
				if (m_mua == kmuaMove)
					m_mua = kmuaNone;	// Move action complete

				SetState(kstGuard);
				break;

			case knMoveStuck:
				// UNDONE: what to do? Doing nothing means we repath every
				// Update (jogging in place) until a valid path can be determined.

				// For now, give up and return to Guard state

				if (m_mua == kmuaMove)
					m_mua = kmuaNone;	// Move action complete

				SetState(kstGuard);
				break;
			}

	//-----------------------------------------------------------------------

	State(kstAttack)
		OnExit
			// Send notifications to any gobs waiting on this gob to get out
			// of attack state

			if (m_wfMunt & kfMuntMoveWaitingNearby)
				NotifyMoveWaitingNearby(m_wx, m_wy);

		OnUpdate
			Assert(m_gidTarget != kgidNull);

			// A command might have been pended while we waited for the
			// firing animation to complete.

			if (m_wfMunt & kfMuntCommandPending && IsReadyForCommand()) {
				m_wfMunt &= ~kfMuntCommandPending;
				gsmm.SendMsg(&m_msgPending);
				return knHandled;
			}

			// if enemy dead/gone or taken over, go to guard mode

			UnitGob *puntTarget = (UnitGob *)ggobm.GetGob(m_gidTarget);
			if (puntTarget == NULL || !IsValidTarget(puntTarget)) {

				// If an Attack action is being carried out and successfully
				// completed, indicate that it is done.

				if (m_mua == kmuaAttack && m_gidTarget == m_msgAction.AttackCommand.gidTarget) {
					m_mua = kmuaNone;	// Action complete

				// If some other action was interrupted to attack, return to it

				} else if (m_mua != kmuaNone) {
					m_gidTarget = kgidNull;
					ContinueActionPendingFireComplete();
					return knHandled;
				}

				m_gidTarget = kgidNull;
				SetStatePendingFireComplete(kstGuard);

			} else {
				// if enemy not in range think about chasing it

				if (!IsGobWithinRange(puntTarget, m_pmuntc->tcFiringRange)) {
					// Test actions and situations under which it is a good idea to chase

					if (!(m_wfMunt & kfMuntStayPut) && (m_mua != kmuaNone || !IsStandingOnActivator()) && 
							(m_wfMunt & kfMuntChaseEnemies ||
							(gsim.GetUpdateCount() - m_cupdLastHitOrNearbyAllyHit <= kcupdAggressivenessBoost) ||
							(m_mua == kmuaAttack && m_gidTarget == m_msgAction.AttackCommand.gidTarget) ||
							(m_mua == kmuaGuardArea && ggobm.IsGobWithinArea(puntTarget, m_msgAction.GuardAreaCommand.nArea)) ||
							(m_mua == kmuaGuardVicinity || m_mua == kmuaHuntEnemies) ||
							IsHumanOrGodControlled()) && IsAttackPointWithinFiringRangeOfTarget(puntTarget)) {
							
						SetStatePendingFireComplete(kstChase);

					// For all other situations we return to the action in-progress

					} else if (m_mua != kmuaNone) {
						ContinueActionPendingFireComplete();
						return knHandled;

					// Or if no action, just fall back to guard state

					} else {
						SetStatePendingFireComplete(kstGuard);
					}
				} else {
					// Check for assumptions

					Assert(m_ppathUnit == NULL);
					Assert(!IsMoveWaiting());

					// Otherwise, fire at the enemy

					if (!(m_wfMunt & kfMuntFiring)) {
						puntTarget->GetAttackPoint(&m_wptTarget);
						WPoint wptFire;
						puntTarget->GetCenter(&wptFire);
						Fire(puntTarget, wptFire.wx, wptFire.wy, wptFire.wx - m_wx, wptFire.wy - m_wy);
					}
				}
			}

			AdvanceAnimation(&m_ani);

			// Handle flashing

			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstChase)
		OnEnter
			MoveEnter();

			// Remember where we found the target last. If it moves position we'll repath

			Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
			if (pgobTarget != NULL) {
				Assert(pgobTarget->GetFlags() & kfGobUnit);
				UnitGob *puntTarget = (UnitGob *)pgobTarget;
				WPoint wptTarget;
				puntTarget->GetAttackPoint(&wptTarget);
				m_tptChaseInitial.tx = TcFromWc(wptTarget.wx);
				m_tptChaseInitial.ty = TcFromWc(wptTarget.wy);
			}

		OnExit
			MoveExit();

		OnUpdate
			if (IsReadyForCommand()) {
				if (m_wfMunt & kfMuntCommandPending) {
					m_wfMunt &= ~kfMuntCommandPending;
					gsmm.SendMsg(&m_msgPending);
					return knHandled;
				}

				// If the Unit is executing the AttackAction and is sufficiently
				// aggressive and sees an enemy unit it should attack it.

				if (m_mua == kmuaAttack) {
					if (m_wfMunt & kfMuntAttackEnemiesWhenMoving) {
						UnitGob *puntTarget = FindEnemyNearby(m_pmuntc->tcFiringRange);
						if (puntTarget != NULL) {
							m_gidTarget = puntTarget->GetId();
							puntTarget->GetAttackPoint(&m_wptTarget);
							SetState(kstAttack);
							return knHandled;
						}
					}
				}

				// Verify the target still exists

				Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
				if (pgobTarget == NULL || !IsValidTarget(pgobTarget)) {
					
					// If some other action was interrupted to attack/chase, return to it

					if (m_mua != kmuaNone) {
						m_gidTarget = kgidNull;
						gsmm.SendMsg(&m_msgAction);
						return knHandled;
					}

					m_gidTarget = kgidNull;
					SetState(kstGuard);
					return knHandled;
				}

				// if enemy is in range, drop back into Attack mode

				if (IsGobWithinRange(pgobTarget, m_pmuntc->tcFiringRange)) {
					SetState(kstAttack);
					return knHandled;

				} else if (!(m_wfMunt & kfMuntChaseEnemies) && !IsHumanOrGodControlled()) {

					// If we're trying to guard an area and the enemy has left it let it go.

					if (m_mua == kmuaGuardArea) {
						if (!ggobm.IsGobWithinArea(pgobTarget, m_msgAction.GuardAreaCommand.nArea)) {
							SetState(kstGuard);
							return knHandled;
						}

					// If we're trying to guard an expanded radius and the enemy has left it let it go.

					} else if (m_mua == kmuaGuardVicinity) {
						if (!IsGobWithinRange(pgobTarget, ktcVicinity) && gsim.GetUpdateCount() - m_cupdLastHitOrNearbyAllyHit > kcupdAggressivenessBoost) {
							SetState(kstGuard);
							return knHandled;
						}

					// These are inherently chase modes. Stick with it.

					} else if (m_mua == kmuaHuntEnemies || m_mua == kmuaAttack) {

					// If we're not acting under an aggressiveness boost anymore drop back to the
					// prior action or Guard

					} else if (gsim.GetUpdateCount() - m_cupdLastHitOrNearbyAllyHit > kcupdAggressivenessBoost) {
						if (m_mua != kmuaNone)
							gsmm.SendMsg(&m_msgAction);
						else
							SetState(kstGuard);
						return knHandled;
					}
				}

				// If the target moved, force a new path to get calced

				Assert(pgobTarget->GetFlags() & kfGobUnit);
				UnitGob *puntTarget = (UnitGob *)pgobTarget;
				WPoint wptTarget;
				puntTarget->GetAttackPoint(&wptTarget);
				TCoord tx = TcFromWc(wptTarget.wx);
				TCoord ty = TcFromWc(wptTarget.wy);
				if (tx != m_tptChaseInitial.tx || ty != m_tptChaseInitial.ty) {
					m_tptChaseInitial.tx = tx;
					m_tptChaseInitial.ty = ty;
					m_wptTarget = wptTarget;
					MoveExit();
				}
			}

			switch (MoveUpdate()) {
			case knMoveTargetReached:
				SetState(kstAttack);
				return knHandled;

			case knMoveStuck:
				// Next few updates, use the target gob's center
				{
					Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
					Assert(pgobTarget->GetFlags() & kfGobUnit);
					UnitGob *puntTarget = (UnitGob *)pgobTarget;
					if (puntTarget != NULL) {
						WPoint wptTarget;
						puntTarget->GetAttackPoint(&wptTarget);
						if (WcTrunc(wptTarget.wx) != WcTrunc(m_wptTarget.wx) || WcTrunc(wptTarget.wy) != WcTrunc(m_wptTarget.wy)) {
							m_wptTarget = wptTarget;
							m_unvl.MinSkip(12);
							break;
						}
					}
				}

				// UNDONE: Put in waiting code so a new path doesn't get searched
				// every update

				m_gidTarget = kgidNull;

#if 0
// Causes looping problems (kstChase->kstAttack->kstChase). Plus "stuck" cases happen less often, so try
// taking this out and revert to guard mode to see how it feels.

				// Return to any action in progress

				if (m_mua != kmuaNone) {
					gsmm.SendMsg(&m_msgAction);
					return knHandled;
				}
#endif

				// UNDONE: Perhaps in this case try searching around waiting gobs.

				SetState(kstGuard);
				return knHandled;
			}

	//-----------------------------------------------------------------------
	// Hunting units sit idle if they can't find a target and won't do 
	// anything except return fire (same as knAggressivenessSelfDefense)

	State(kstHuntEnemies)
		OnEnter
			// Play idle animation

			StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniDone);
			m_wptTarget.wx = kwxInvalid;
			m_gidTarget = kgidNull;
			m_cCountdown = GetIdleCountdown();

		OnUpdate
			Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
			if (pgobTarget == NULL || !IsValidTarget(pgobTarget)) {
				int cpuntFound = 0;
				Assert(gcbScratch >= kcpgobMax * sizeof(UnitGob *));
				UnitGob **apuntFound = (UnitGob **)gpbScratch;

				// UNDONE: only do this every few updates
				// Find an enemy to attack

				Gob *pgob = ggobm.GetFirstGob();
				for (; pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
					// Is it an active enemy that this unit type is allowed to attack?

					if (!IsValidTarget(pgob))
						continue;	// no

					// Only Units pass the IsValidTarget test

					UnitGob *punt = (UnitGob *)pgob;

					// Is it one of the type of units we're supposed to hunt?

					if (!(punt->GetConsts()->um & m_msgAction.HuntEnemiesCommand.um))
						continue;

					apuntFound[cpuntFound++] = punt;
				}

				if (cpuntFound > 0) {
					UnitGob *punt = apuntFound[GetRandom() % cpuntFound];
					punt->GetAttackPoint(&m_wptTarget);
					m_gidTarget = punt->GetId();
					m_wptTargetCenter = m_wptTarget;
					m_tcTargetRadius = 0;
					m_wcMoveDistPerUpdate = m_pmuntc->GetMoveDistPerUpdate();
					SetState(kstAttack);
					return knHandled;
				}
			}

			AdvanceAnimation(&m_ani);	// idle animation

			m_cCountdown -= m_unvl.GetUpdateCount();
			if (m_cCountdown < 0) {
				Idle();
				m_cCountdown = GetIdleCountdown();
			}
			m_unvl.MinSkip(m_cCountdown);

			// Handle flashing

			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstDying)
		OnEnter
			TRect trc;
			GetTileRect(&trc);

			Deactivate();

			// Redraw this part of minimap. It will skip inactive munts

			gpmm->RedrawTRect(&trc);

#ifdef DEBUG
			if (m_wfMunt & kfMuntDestinationReserved) {
				Assert(!IsTileReserved(m_txDst, m_tyDst));
			} else {
				Assert(!IsTileReserved(TcFromWc(m_wx), TcFromWc(m_wy)));
			}
#endif

			m_ff ^= kfGobLayerDepthSorted | kfGobLayerSurfaceDecal;

			gsndm.PlaySfx(SfxFromCategory(m_pmuntc->sfxcDestroyed));

			// remove corpse in .2 seconds

			gsmm.SendDelayedMsg(kmidDelete, 20, m_gid, m_gid);

			Gob *pgobExpl = CreateAnimGob(m_wx, m_wy, kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, s_panidVehicleExplosion);
			if (pgobExpl != NULL)
				pgobExpl->SetOwner(m_pplr);

		OnUpdate
			// Handle flashing

			DefUpdate();

		OnMsg(kmidDelete)
			Delete();
			return knDeleted;

		// Eat all other messages (e.g., kmidHit, kmidNearbyAllyHit)

		DiscardMsgs

	//-----------------------------------------------------------------------

	State(kstChangeStatePendingFireComplete)
		OnUpdate
			if (!(m_wfMunt & kfMuntFiring)) {
				SetState(m_stPending);
				m_stPending = kstReservedNull;
			}

			// Will advance any ongoing firing animation

			DefUpdate();

	State(kstContinueActionPendingFireComplete)
		OnUpdate
			if (!(m_wfMunt & kfMuntFiring)) {
				gsmm.SendMsg(&m_msgAction);
				return knHandled;
			}

			// Will advance any ongoing firing animation

			DefUpdate();

EndStateMachine
}

void MobileUnitGob::ReserveTile(TCoord tx, TCoord ty, bool fReserve)
{
	if (fReserve) {
		gsim.GetLevel()->GetTerrainMap()->SetFlags(tx, ty, 1, 1, kbfMobileUnit);
	} else {
		gsim.GetLevel()->GetTerrainMap()->ClearFlags(tx, ty, 1, 1, kbfMobileUnit);
	}
}

bool MobileUnitGob::IsTileReserved(TCoord tx, TCoord ty)
{
	return gsim.GetLevel()->GetTerrainMap()->TestFlags(tx, ty, 1, 1, kbfMobileUnit);
}

byte MobileUnitGob::GetTileFlags(TCoord tx, TCoord ty)
{
	byte bf;
	bool f = gsim.GetLevel()->GetTerrainMap()->GetFlags(tx, ty, &bf);
	if (!f)
		bf = kbfStructure;
	return bf;
}

bool MobileUnitGob::InTransition()
{
	if (m_ppathUnit == NULL)
		return false;

	return m_cMoveStepsRemaining != 0;
}

bool MobileUnitGob::IsReadyForCommand()
{
	if (m_wfMunt & kfMuntFiring)
		return false;
	return !InTransition();
}

void MobileUnitGob::NotifyMoveWaitingNearby(WCoord wx, WCoord wy)
{
	Assert(m_wfMunt & kfMuntMoveWaitingNearby);
	m_wfMunt &= ~kfMuntMoveWaitingNearby;

	// Look around our dst to see if there are units trying to get in to our dst tile

	TCoord txDst = TcFromWc(wx);
	TCoord tyDst = TcFromWc(wy);

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);

	for (int n = 0; n < ARRAYSIZE(g_mpDirToDx); n++) {
		// Get valid TCoord to check

		TCoord tx = txDst + g_mpDirToDx[n];
		TCoord ty = tyDst + g_mpDirToDy[n];
		if (tx < 0 || tx >= ctx)
			continue;
		if (ty < 0 || ty >= cty)
			continue;

		// Check it for gobs that are waiting on this gob

		for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
			// Any mobile units in this tile?

			MobileUnitGob *pgob = (MobileUnitGob *)ggobm.GetGob(gid);
			if (pgob == NULL)
				continue;
			if (!(pgob->m_ff & kfGobMobileUnit))
				continue;

			// See if this mobile unit is in transition towards our desired tile

			if (pgob->m_txDst == txDst && pgob->m_tyDst == tyDst) {
				if (pgob->IsMoveWaiting()) {
					gsmm.SendMsg(kmidMoveWaitingNearby, pgob->GetId());
				}
			}
		}
	}
}

MobileUnitGob *MobileUnitGob::GetReservingUnit(TCoord tx, TCoord ty)
{
	// Need to enumerate all mobile units in this tile, there can be more than one
	// in certain transition situations

	for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
		// Any mobile units in this tile?

		MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(gid);
		if (pmunt == NULL)
			continue;
		if (!(pmunt->m_ff & kfGobMobileUnit))
			continue;

		// It is the reserving gob if it either isn't in transition or
		// is in transition to this tile

		if (!pmunt->InTransition() || (pmunt->m_txDst == tx && pmunt->m_tyDst == ty)) {
			Assert(IsTileReserved(tx, ty));
			return pmunt;
		}
	}

	// Now look for units transitioning into this tile

	MobileUnitGob *pmunt = AnyTransitionsIntoTile(tx, ty, NULL);
	Assert((pmunt != NULL) == IsTileReserved(tx, ty));
	return pmunt;
}

MobileUnitGob *MobileUnitGob::AnyTransitionsIntoTile(TCoord txDst, TCoord tyDst, Gob *pgobIgnore)
{
	// Look around our dst to see if there are units trying to get in to our dst tile

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);

	for (int n = 0; n < ARRAYSIZE(g_mpDirToDx); n++) {
		// Get valid TCoord to check

		TCoord tx = txDst + g_mpDirToDx[n];
		TCoord ty = tyDst + g_mpDirToDy[n];
		if (tx < 0 || tx >= ctx)
			continue;
		if (ty < 0 || ty >= cty)
			continue;

		// Check it for gobs that are entering our dst

		for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
			// Any mobile units in this tile?

			MobileUnitGob *pgob = (MobileUnitGob *)ggobm.GetGob(gid);
			if (pgob == NULL || pgob == pgobIgnore)
				continue;
			if (!(pgob->m_ff & kfGobMobileUnit))
				continue;

			// See if this mobile unit is in transition towards our desired tile

			if (pgob->m_txDst == txDst && pgob->m_tyDst == tyDst) {
				if (pgob->InTransition()) {
					return pgob;
                }
			}
		}
	}

	return NULL;
}

void MobileUnitGob::MoveEnter(bool fReset)
{
	// Catch movewait bits that are set but shouldn't be

	Assert(!IsMoveWaiting());

	// Make sure we're not in transition

	Assert(!InTransition());
	StartAnimation(&m_ani, m_pmuntc->anMovingStripIndices[m_dir], 0, kfAniLoop);

	// These get reset when this move isn't aware of group behavior

	if (fReset) {
		m_wptTargetCenter = m_wptTarget;
		m_tcTargetRadius = 0;
	}
	if (m_wcMoveDistPerUpdate == 0)
		m_wcMoveDistPerUpdate = m_pmuntc->GetMoveDistPerUpdate();

	// If the target is in blocked area, walk back toward this unit to find the first free tile,
	// in a straight line.

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	TCoord txTo = TcFromWc(m_wptTarget.wx);
	TCoord tyTo = TcFromWc(m_wptTarget.wy);
	TCoord txFree, tyFree;
	if (ptrmap->IsBlocked(txTo, tyTo, 0)) {
		if (ptrmap->FindFirstUnoccupied(txTo, tyTo, TcFromWc(m_wx), TcFromWc(m_wy), &txFree, &tyFree)) {
			m_wptTarget.wx = WcFromTc(txFree) + kwcTileHalf;
			m_wptTarget.wy = WcFromTc(tyFree) + kwcTileHalf;
		}
	}

	// Let movement code know that a path is pending for this gob so proper waiting semantics can be
	// performed.

	m_wfMunt |= kfMuntPathPending;
	m_wfMunt &= ~kfMuntStuck;
}

void MobileUnitGob::MoveExit()
{
	m_wfMunt &= ~(kfMuntMoveWait | kfMuntPathPending);
	delete m_ppathUnit;
	m_ppathUnit = NULL;
	delete m_ppathAvoid;
	m_ppathAvoid = NULL;

	// Send notifications to any gobs waiting on this gob

	if (m_wfMunt & kfMuntMoveWaitingNearby)
		NotifyMoveWaitingNearby(m_wx, m_wy);
}

Path *MobileUnitGob::FindPath(TCoord txFrom, TCoord tyFrom, TCoord txTo, TCoord tyTo)
{
	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();

#ifdef __CPU_68K
	// See if we have a cached path that we can reuse - 68K only since this
	// sucks

#define ktcPathLengthReuse 35
#define ktcPathEndPointReuse 7

	word nDistT = ganSquared[abs(txFrom - txTo)] + ganSquared[abs(tyFrom - tyTo)];
	if (GetType() != kgtGalaxMiner && nDistT >= ganSquared[ktcPathLengthReuse]) {
		for (int nPath = 0; nPath < s_cpathCached; nPath++) {
			// Long enough to be worth it?

			Path *ppathT = s_apathCached[nPath];
			if (ppathT->GetCount() < ktcPathLengthReuse)
				continue;

			// In the general vicinity of our start / end points

			TPoint tptStart;
			ppathT->GetStartPoint(&tptStart);
			TPoint tptEnd;
			ppathT->GetPointRaw(ppathT->GetCount() - 1, &tptEnd);
			word nDistStart = ganSquared[abs(tptStart.tx - txFrom)] + ganSquared[abs(tptStart.ty - tyFrom)];
			word nDistEnd = ganSquared[abs(tptEnd.tx - txTo)] + ganSquared[abs(tptEnd.ty - tyTo)];
			word nDistReuse = ganSquared[ktcPathEndPointReuse] * 2;
			if (nDistStart > nDistReuse || nDistEnd > nDistReuse)
				continue;

			// See if our closest approach is free of obstacles

			TPoint tptClosest;
			if (ppathT->FindClosestPoint(txFrom, tyFrom, 0, 5, &tptClosest) == -1)
				continue;
			if (ptrmap->IsLineOccupied(txFrom, tyFrom, tptClosest.tx, tptClosest.ty, 0))
				continue;

			// Looking good so far. See if our closest end point is free of obstacles

			TPoint tptDestClosest;
			int itptDestClosest = ppathT->FindClosestPoint(txTo, tyTo, 0, ppathT->GetCount(), &tptDestClosest);
			if (itptDestClosest == -1)
				continue;
			if (ptrmap->IsLineOccupied(tptDestClosest.tx, tptDestClosest.ty, txTo, tyTo, 0))
				continue;

			// We're ready to use this path. First create a copy

			ppathT = ppathT->Clone();
			if (ppathT == NULL)
				continue;

			// If the ending is the same, use it

			if (tptDestClosest.tx == txTo && tptDestClosest.ty == tyTo)
				return ppathT;

			// Now lop off the end and amend it with our new dest
			// If this doesn't work, use it as is

			if (!ppathT->TrimEnd(itptDestClosest))
				return ppathT;

			// Add our new ending. If we run into something, use the path as is

			Path *ppathAppend = ptrmap->FindLinePath(tptDestClosest.tx, tptDestClosest.ty, txTo, tyTo, kbfStructure);
			if (ppathAppend == NULL)
				return ppathT;

			// Append this to our ppath

			ppathT->Append(ppathAppend);
			delete ppathAppend;

			// Done

			return ppathT;
		}
	}
#endif

	// No cached paths that satisfy our needs. Make a new one.
	// If it's close first try an easy path

#define ktcFastPathRangeMax 20
	Path *ppathT = NULL;
	if (ganSquared[abs(txFrom - txTo)] + ganSquared[abs(tyFrom - tyTo)] < ganSquared[ktcFastPathRangeMax] * 2)
		ppathT = ptrmap->FindLinePath(txFrom, tyFrom, txTo, tyTo, kbfStructure);
	if (ppathT == NULL)
		ppathT = ptrmap->FindPath(txFrom, tyFrom, txTo, tyTo, kbfStructure);

#ifdef __CPU_68K
	// Cache this path if it is long enough to be interesting

	if (ppathT != NULL) {
		if (ppathT->GetCount() >= ktcPathLengthReuse) {
			// Move the existing paths that we have down one

			Path *ppathCache = ppathT->Clone();
			if (ppathCache != NULL) {
				if (s_cpathCached == ARRAYSIZE(s_apathCached)) {
					delete s_apathCached[ARRAYSIZE(s_apathCached) - 1];
				} else {
					s_cpathCached++;
				}
				memmove(&s_apathCached[1], &s_apathCached[0], (ARRAYSIZE(s_apathCached) - 1) * ELEMENTSIZE(s_apathCached));
				s_apathCached[0] = ppathCache;
			}
		}
	}
#endif

	return ppathT;
}

void MobileUnitGob::FreeCachedPaths()
{
	for (int nPath = 0; nPath < s_cpathCached; nPath++)
		delete s_apathCached[nPath];
	s_cpathCached = NULL;
}

bool MobileUnitGob::PrepPath(WCoord wxDst, WCoord wyDst)
{
	// Get a path.

	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	TCoord txTo = TcFromWc(wxDst);
	TCoord tyTo = TcFromWc(wyDst);
	Path *ppathT = FindPath(txFrom, tyFrom, txTo, tyTo);

	// Prep unit path

	delete m_ppathAvoid;
	m_ppathAvoid = NULL;
	delete m_ppathUnit;
	m_ppathUnit = ppathT;
	m_itptPath = 0;
	m_wfMunt &= ~kfMuntPathPending;

#ifdef DRAW_PATHS
	m_wxDst = m_wx;
	m_wyDst = m_wy;
#endif
	m_cMoveStepsRemaining = 0;

	return m_ppathUnit != NULL;
}

int MobileUnitGob::MoveUpdate()
{
	// If a command is pending and now is a good time to handle it, do so.

	if ((m_wfMunt & kfMuntCommandPending) && IsReadyForCommand()) {
		m_wfMunt &= ~kfMuntCommandPending;
		gsmm.SendMsg(&m_msgPending);
		return knMoveMoving;
	}

    // Initialize a path if we haven't yet. Don't start moving on it yet so
    // that all mobile units in a group move have paths. This establishes
    // proper waiting behavior

	if (m_ppathUnit == NULL) {
		// Check if already there

		if (CheckDestinationReached())
			return knMoveTargetReached;

		// No, so calc a path

		if (!PrepPath(m_wptTarget.wx, m_wptTarget.wy)) {
//			Trace("0x%08lx: 0x%08lx @ %d,%d calc path failed", HostGetTickCount(), this, TcFromWc(m_wx), TcFromWc(m_wy));
			m_wfMunt |= kfMuntStuck;
			return knMoveStuck;
		}
//		Trace("0x%08lx: 0x%08lx @ %d,%d calc path succeeded", HostGetTickCount(), this, TcFromWc(m_wx), TcFromWc(m_wy));
	}

	// We have a path. If we're not in transition, prepare the next transition

	if (!InTransition()) {
//		Trace("0x%08lx: 0x%08lx @ %d,%d not in transition.", HostGetTickCount(), this, TcFromWc(m_wx), TcFromWc(m_wy));

		// If the next transition could not be prepared, error out

		TPoint tptNext;
		int nMoveResult;
		if (!PrepareNextTransition(&tptNext, &nMoveResult)) {
#if 0 // def DEBUG
			static char *s_aszMoveResult[] = { "knMoveMoving", "knMoveTargetReached", "knMoveStuck", "knMoveWaiting" };
			Trace("0x%08lx @ %d,%d returning %s", this, TcFromWc(m_wx), TcFromWc(m_wy), s_aszMoveResult[nMoveResult]);
			if (nMoveResult == knMoveWaiting)
				Trace("    (waiting on %d, %d, reserving munt: 0x%08lx)", m_txDst, m_tyDst, GetReservingUnit(m_txDst, m_tyDst));
#endif
			return nMoveResult;
		}

		// Looks good, enter transition

//		Trace("0x%08lx @ %d,%d -> %d, %d entering transition.", this, TcFromWc(m_wx), TcFromWc(m_wy), tptNext.tx, tptNext.ty);
		EnterTransition(&tptNext);
	}

	// We're in transition; move through the transition

	ContinueTransition();
	return knMoveMoving;
}

void MobileUnitGob::EnterTransition(TPoint *ptptNext)
{
	// Attempt to enter the next transition

	Assert(!InTransition());
	Assert(ptptNext->tx != TcFromWc(m_wx) || ptptNext->ty != TcFromWc(m_wy));

	// Clear the replicator input spot. This gob isn't a viable replication candidate in transition.

	m_wfMunt &= ~kfMuntAtReplicatorInput;

	// Remember where this unit is going

	m_txDst = ptptNext->tx;
	m_tyDst = ptptNext->ty;

	// Remember this for "draw paths" feature

#ifdef DRAW_PATHS
	m_wxDst = WcFromTc(m_txDst) + kwcTileHalf;
	m_wyDst = WcFromTc(m_tyDst) + kwcTileHalf;
#endif

	// Unreserve current tile, reserve new tile

	TCoord txOld = TcFromWc(m_wx);
	TCoord tyOld = TcFromWc(m_wy);
	Assert(IsTileReserved(txOld, tyOld));
	ReserveTile(txOld, tyOld, false);
	m_dirNext = Direction16FromLocations(txOld, tyOld, ptptNext->tx, ptptNext->ty);
	Assert(m_dirNext != kdirInvalid);

	// Tile is assumed to be available; reserve it and start a transition

	Assert(!IsTileReserved(ptptNext->tx, ptptNext->ty));
	ReserveTile(ptptNext->tx, ptptNext->ty, true);

	// Mark that we've reserved the dest tile so it can be unreserved if
	// the unit becomes deactivated.

	m_wfMunt |= kfMuntDestinationReserved;

	// Send notifications to any gobs waiting on this gob's tile to clear, which it
	// just did.

	if (m_wfMunt & kfMuntMoveWaitingNearby)
		NotifyMoveWaitingNearby(m_wx, m_wy);

	// Prepare the appropriate animation

	StartAnimation(&m_ani, m_pmuntc->anMovingStripIndices[m_dir], 0, kfAniLoop | kfAniIgnoreFirstAdvance);

	// Calc move steps to next tile. Adjust for diagonal.
	// This unit is now "in transition".

	WCoord wcMoveDist = m_wcMoveDistPerUpdate;
	if (Dir16IsDiagonal(m_dirNext))
		wcMoveDist = gawcDiagonalDist[wcMoveDist];
	m_cMoveStepsRemaining = gaDiv256byNWithRounding[wcMoveDist];
}

void MobileUnitGob::ContinueTransition()
{
	// We're in transition and simply moving to the next tile

	Assert(InTransition());
	
	// Transition movement doesn't skip updates

	m_unvl.MinSkip();

	// Make sure we're facing the way we want to go before we go

	if (m_dirNext != m_dir) {
		m_dir = TurnToward16(m_dirNext, m_dir);
		StartAnimation(&m_ani, m_pmuntc->anMovingStripIndices[m_dir], 0, kfAniLoop | kfAniIgnoreFirstAdvance);
		return;
	}

	WCoord wxNew = m_wx;
	WCoord wyNew = m_wy;

	m_cMoveStepsRemaining--;
	if (m_cMoveStepsRemaining == 0) {

		// Hop to the exact center of the destination tile

		wxNew = WcTrunc(m_wx) + kwcTileHalf;
		wyNew = WcTrunc(m_wy) + kwcTileHalf;

		// No longer in transition; we're at the destination. Clear this bit
		// so we know which tile is reserved

		m_wfMunt &= ~kfMuntDestinationReserved;
	} else {
		WCoord wcMoveDist = m_wcMoveDistPerUpdate;

		// Are we moving diagonally?

		if (Dir16IsDiagonal(m_dir))
			wcMoveDist = gawcDiagonalDist[wcMoveDist];

		int dx = Dir16ToDx(m_dir);
		if (dx == -1) {
			wxNew -= wcMoveDist;
		} else if (dx == 1) {
			wxNew += wcMoveDist;
		}

		int dy = Dir16ToDy(m_dir);
		if (dy == -1) {
			wyNew -= wcMoveDist;
		} else if (dy == 1) {
			wyNew += wcMoveDist;
		}
	}

	// Move here

	SetPosition(wxNew, wyNew);

	// Animate the little fella running there.

	AdvanceAnimation(&m_ani);

	// Handle flashing, etc

	DefUpdate();
}

bool MobileUnitGob::IsMoveWaitCycle(MobileUnitGob *pmunt)
{
	// This routine detects move wait loops (units waiting on units waiting on
	// units, etc). If there is a loop, this move wait isn't valid.

	// Quick outs. If nobody is waiting on this unit, then by implication this won't
	// be a cycle. Likewise if the unit we want to wait on isn't waiting, then by
	// implication this won't be a cycle.

	if (!(m_wfMunt & kfMuntMoveWaitingNearby))
		return false;

	// Special case: if pmunt is attacking then not a wait cycle since it
	// isn't mobile.

	if (pmunt->m_st == kstAttack)
		return false;

	// Start at pmunt and see if we loop back to the current unit

	int cCycle = 0;
	MobileUnitGob *pmuntT = pmunt;
	while (pmuntT != NULL) {
		// If we've looped back to ourselves, this is a cycle

		if (pmuntT == this)
			return true;

		// If this unit isn't move waiting, not a cycle

		if (!(pmuntT->m_wfMunt & kfMuntMoveWait))
			return false;

		// Cycle to who pmuntT is waiting on
		// Because of evaulation order it is possible that a pmuntT is waiting on a
		// tile that is not reserved because the reserving munt started a transition
		// during this update cycle before pmuntT gob executed. This is a break in the cycle;
		// whatever gob takes this tile eventually will recheck for cycles, so it is
		// ok to assume this condition is not a cycle.

		pmuntT = GetReservingUnit(pmuntT->m_txDst, pmuntT->m_tyDst);
		if (pmuntT == NULL)
			return false;

		// Break out of we've looped too much. 

		if (cCycle++ > 15)
			return false;
	}

	return false;
}

bool MobileUnitGob::MoveWaitForUnit(MobileUnitGob *pmunt, TCoord tx, TCoord ty)
{
	// Invalid if this creates a cycle

	if (IsMoveWaitCycle(pmunt))
		return false;

	// This unit will "move wait".

	m_wfMunt |= kfMuntMoveWait;
	m_txDst = tx;
	m_tyDst = ty;
	pmunt->m_wfMunt |= kfMuntMoveWaitingNearby;
	return true;
}

bool MobileUnitGob::CheckDestinationReached()
{
	// If we're where we want to be, we're done

	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);
	if (tx == TcFromWc(m_wptTarget.wx) && ty == TcFromWc(m_wptTarget.wy))
		return true;

	// Not at dest if we have no path

	if (m_ppathUnit == NULL)
		return false;

	// See if we're "close enough"

	TCoord txTargetCenter = TcFromWc(m_wptTargetCenter.wx);
	TCoord tyTargetCenter = TcFromWc(m_wptTargetCenter.wy);
	int dtx = abs(tx - txTargetCenter);
	if (dtx >= ARRAYSIZE(gmpDistFromDxy))
		return false;
	int dty = abs(ty - tyTargetCenter);
	if (dty >= ARRAYSIZE(gmpDistFromDxy))
		return false;
	if (gmpDistFromDxy[dtx][dty] > m_tcTargetRadius)
		return false;

	// We're close enough; need to check if there is terrain between this unit
	// and the center. If so we're not at the dest.

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	if (ptrmap->IsLineOccupied(tx, ty, txTargetCenter, tyTargetCenter, 0))
		return false;

	return true;
}

bool MobileUnitGob::PrepareNextTransition(TPoint *ptptNext, int *pnMoveResult)
{
	Assert(!InTransition());	

	// First clear move wait since we'll be making a fresh decision

	m_wfMunt &= ~kfMuntMoveWait;

	// Get the viable transition locations

	TPoint atpt[8];
	int ctpt = GetNextLocations(atpt);
	if (ctpt == 0) {
		// Check to see if we're at a replicator input point. If so move wait until the replicator
		// takes over

		if (CheckReplicatorPoint()) {
			m_wfMunt |= kfMuntAtReplicatorInput | kfMuntMoveWait;
			*pnMoveResult = knMoveTargetReached;
			return false;
		}

		// No move candidates. Are we close enough to the target?

		if (CheckDestinationReached()) {
			*pnMoveResult = knMoveTargetReached;
			return false;
		}

		// No such luck. We're stuck.

		m_wfMunt |= kfMuntStuck;
		*pnMoveResult = knMoveStuck;
		return false;
	}

	// See if our first choice is clear; if so use it (common case)

	byte bf = GetTileFlags(atpt[0].tx, atpt[0].ty);
	if (!(bf & (kbfStructure | kbfMobileUnit))) {
		*ptptNext = atpt[0];
		m_unvl.MinSkip();
		*pnMoveResult = knMoveMoving;
		return true;
	}

	// Find reserving munts once

	MobileUnitGob *apmunt[8];
	byte abf[8];
	for (int itpt = 0; itpt < ctpt; itpt++) {
		// Structure or mobile unit?

		abf[itpt] = GetTileFlags(atpt[itpt].tx, atpt[itpt].ty);
		if (abf[itpt] & kbfMobileUnit) {
			apmunt[itpt] = GetReservingUnit(atpt[itpt].tx, atpt[itpt].ty);
#ifdef DEBUG
            if (apmunt[itpt] == NULL) {
                Assert();
            }
            if (apmunt[itpt] == this) {
                Assert();
            }
#endif
			//Assert(apmunt[itpt] != NULL && apmunt[itpt] != this);
		} else {
			apmunt[itpt] = NULL;
		}
	}

	// Check open tiles or move wait opportunities

	if (ContinueMoveWaiting(apmunt, abf, atpt, ctpt, ptptNext)) {
		if (IsMoveWaiting()) {
			*pnMoveResult = knMoveWaiting;
			return false;
		}
		return true;
	}

	// All the move locations are currently occupied. Let's see if we're close
	// enough to the destination already.

	if (CheckDestinationReached()) {
		*pnMoveResult = knMoveTargetReached;
		return false;
	}

	// Need to deal with blockers.

	if (HandleCollision(apmunt, abf, atpt, ctpt)) {
		// If move waiting, go into wait mode

		if (IsMoveWaiting()) {
			*pnMoveResult = knMoveWaiting;
			return false;
		}

        // Otherwise wait an update so that the new path options get
        // re-evaluated

		m_unvl.MinSkip();
		*pnMoveResult = knMoveMoving;
		return false;
	}

	// Nothing!

	m_wfMunt |= kfMuntStuck;
	*pnMoveResult = knMoveStuck;
	return false;
}

bool MobileUnitGob::HandleCollision(MobileUnitGob **apmunt, byte *abf, TPoint *atpt, int ctpt)
{
	// All locations are blocked. We need to know who we're colliding with
	// Ask the standing blocker to move out of the way. This usually works
	// if the unit isn't an enemy and isn't attacking

	int itpt;
	for (itpt = 0; itpt < ctpt; itpt++) {
		MobileUnitGob *pmunt = apmunt[itpt];
		if (pmunt == NULL)
			continue;
		if (!pmunt->IsMobile()) {
			if (PlanMoveAsidePath(pmunt, atpt[itpt].tx, atpt[itpt].ty))
				return true;
		}
	}

	// Try to locally path around. Not important to check all
	// units since the first path will work around the other choices.

	if (FindLocalAvoidPath(atpt[0].tx, atpt[0].ty))
		return true;

	// Find a friendly attacking unit to wait on

	for (itpt = 0; itpt < ctpt; itpt++) {
		if (apmunt[itpt] == NULL)
			continue;
		if (apmunt[itpt]->m_st == kstAttack && apmunt[itpt]->IsAlly(GetSide()))
			if (MoveWaitForUnit(apmunt[itpt], atpt[itpt].tx, atpt[itpt].ty))
				return true;
	}

	return false;
}

bool MobileUnitGob::ContinueMoveWaiting(MobileUnitGob **apmunt, byte *abf, TPoint *atpt, int ctpt, TPoint *ptptNext)
{
	int itpt;
#if 0
	for (itpt = 0; itpt < ctpt; itpt++) {
		// If the dest is not reserved, take it

		byte bf = abf[itpt];
		if (!(bf & (kbfStructure | kbfMobileUnit))) {
			*ptptNext = atpt[itpt];
			return true;
		}

		// If not a mobile unit, can't move wait on it

		if (!(bf & kbfMobileUnit))
			continue;

		// Deprioritize moving waiting on a munt that is still in transition

		MobileUnitGob *pmunt = apmunt[itpt];
		if (!pmunt->InTransition() && pmunt->IsMobile()) {
			if (MoveWaitForUnit(pmunt, atpt[itpt].tx, atpt[itpt].ty))
				return true;
		}
	}
#endif

	// Didn't find anything; try move waiting on the first possible unit

	for (itpt = 0; itpt < ctpt; itpt++) {
		// If the dest is not reserved, take it

		byte bf = abf[itpt];
		if (!(bf & (kbfStructure | kbfMobileUnit))) {
			*ptptNext = atpt[itpt];
			return true;
		}

		// Deprioritize moving waiting on a munt that is still in transition

		MobileUnitGob *pmunt = apmunt[itpt];
		if (pmunt != NULL && pmunt->IsMobile()) {
			if (MoveWaitForUnit(pmunt, atpt[itpt].tx, atpt[itpt].ty))
				return true;
		}
	}

	return false;
}

bool MobileUnitGob::FindLocalAvoidPath(TCoord tx, TCoord ty)
{
	// Get the trackpoint for the current location

	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	TrackPoint trkpStart;
	if (!trkpStart.Init(m_ppathUnit, txFrom, tyFrom, m_itptPath, 3))
		return false;
	Direction dirBlocked = DirectionFromLocations(txFrom, tyFrom, tx, ty);

	// Find a closer track point. 

#define kcStepsAvoid 15

	// Try clockwise first

	TPoint atptCW[kcStepsAvoid];
	TrackPoint trkpCW;
	int ctptCW = FindCloserTrackPoint(&trkpStart, dirBlocked, true, kcStepsAvoid, atptCW, &trkpCW);

	// Counter-clockwise

	TPoint atptCCW[kcStepsAvoid];
	TrackPoint trkpCCW;
	int ctptCCW = FindCloserTrackPoint(&trkpStart, dirBlocked, false, kcStepsAvoid, atptCCW, &trkpCCW);

	// Which one to use?

	int ctptUse;
	TPoint *atptUse;
	bool fSuccess = true;
	if (ctptCCW > 0) {
		// CCW is valid. Is CW valid? If so compare length

		if (ctptCW > 0) {
			// Both valid. Which is shorter?

			if (ctptCW < ctptCCW) {
				ctptUse = ctptCW;
				atptUse = atptCW;
			} else {
				ctptUse = ctptCCW;
				atptUse = atptCCW;
			}

		} else {
			// CW is invalid, CCW is valid

			ctptUse = ctptCCW;
			atptUse = atptCCW;
		}
	} else {
		// CCW is invalid. If CW is valid; use it

		if (ctptCW > 0) {
			ctptUse = ctptCW;
			atptUse = atptCW;
		} else {
			// Both invalid

			return false;
		}
	}

	// Create a path from this and use it
	// If our edge finder approach worked, use it

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	Path *ppath = NULL;
	if (fSuccess) {
		Assert(ctptUse != 0);
		ppath = CreatePath(ptrmap, txFrom, tyFrom, atptUse, ctptUse);
		if (ppath != NULL) {
			// Success. This "avoid" path gets us further along the unit path.

			delete m_ppathAvoid;
			m_ppathAvoid = ppath;
			return true;
		}
	}

	return false;
}

// Map the last direction travelled to next directions to scan

Direction gmpDirLastDirScanStartCW[] = { kdirNE, kdirSE, kdirSE, kdirSW, kdirSW, kdirNW, kdirNW, kdirNE };
Direction gmpDirLastDirScanEndCW[] = { (kdirNE - 5) & 7, (kdirSE - 5) & 7, (kdirSE - 5) & 7, (kdirSW - 5) & 7, (kdirSW - 5) & 7, (kdirNW - 5) & 7, (kdirNW - 5) & 7, (kdirNE - 5) & 7};
Direction gmpDirLastDirScanStartCCW[] = { kdirNW, kdirNW, kdirNE, kdirNE, kdirSE, kdirSE, kdirSW, kdirSW };
Direction gmpDirLastDirScanEndCCW[] = { (kdirNW + 5) & 7, (kdirNW + 5) & 7, (kdirNE + 5) & 7, (kdirNE + 5) & 7, (kdirSE + 5) & 7, (kdirSE + 5) & 7, (kdirSW + 5) & 7, (kdirSW + 5) & 7 };

int MobileUnitGob::FindCloserTrackPoint(TrackPoint *ptrkpStart, Direction dirBlocked, bool fClockwise, int ctpt, TPoint *atpt, TrackPoint *ptrkpNew)
{
	Direction *pmpDirLastDirScanStart;
	Direction *pmpDirLastDirScanEnd;
	Direction dirStart, dirEnd;
	int ddir;
	if (fClockwise) {
		pmpDirLastDirScanStart = gmpDirLastDirScanStartCW;
		pmpDirLastDirScanEnd = gmpDirLastDirScanEndCW;
		dirStart = (dirBlocked - 1) & 7;
		dirEnd = (dirStart - 7) & 7;
		ddir = -1;
	} else {
		pmpDirLastDirScanStart = gmpDirLastDirScanStartCCW;
		pmpDirLastDirScanEnd = gmpDirLastDirScanEndCCW;
		dirStart = (dirBlocked + 1) & 7;
		dirEnd = (dirStart + 7) & 7;
		ddir = 1;
	}

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	int itptClosestSearch = m_itptPath > 0 ? m_itptPath - 1 : 0;
	ptrkpNew->InitFrom(ptrkpStart);

	int ctptLastGood = 0;
	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	TCoord txLast = txFrom;
	TCoord tyLast = tyFrom;
	for (int itpt = 0; itpt < ctpt; itpt++) {
		Direction dirBest = kdirInvalid;
		for (Direction dirNew = dirStart; dirNew != dirEnd; dirNew = (dirNew + ddir) & 7) {
			// Valid TCoord?

			TCoord txNew = txLast + g_mpDirToDx[dirNew];
			TCoord tyNew = tyLast + g_mpDirToDy[dirNew];
			if (txNew < 0 || txNew >= ctx)
				continue;
			if (tyNew < 0 || tyNew >= cty)
				continue;

            // Is tile blocked? Can't rely on moving or standing units getting
            // out of the way because they may not be able to

			if (ptrmap->IsBlocked(txNew, tyNew, kbfStructure | kbfMobileUnit))
				continue;

			// Open tile, we're there

			dirBest = dirNew;
			break;
		}

		// Anything?

		if (dirBest == kdirInvalid)
			return ctptLastGood;

		// If this point is further along the path than our last track point
		// then remember this position

		atpt[itpt].tx = txLast + g_mpDirToDx[dirBest];
		atpt[itpt].ty = tyLast + g_mpDirToDy[dirBest];
		TrackPoint trkpT;
		if (trkpT.Init(m_ppathUnit, atpt[itpt].tx, atpt[itpt].ty, itptClosestSearch, ctpt)) {
			// Remember this as the last best progress point

			if (ptrkpNew->IsProgress(&trkpT)) {
#if 0
				// Remembers a "last best" progress point without knowing access to the unit
				// path is possible from there

				ptrkpNew->InitFrom(&trkpT);
				ctptLastGood = itpt + 1;
#else
				// If the unit path is accessible from here, return this path

				TPoint tptClosest;
				trkpT.GetClosestPoint(&tptClosest);
				if (!ptrmap->IsLineOccupied(atpt[itpt].tx, atpt[itpt].ty, tptClosest.tx, tptClosest.ty, kbfStructure)) {
					ptrkpNew->InitFrom(&trkpT);
					return itpt + 1;
				}
#endif
			}
		}

		// Go to next point in path

		txLast = atpt[itpt].tx;
		tyLast = atpt[itpt].ty;
		dirStart = pmpDirLastDirScanStart[dirBest];
		dirEnd = pmpDirLastDirScanEnd[dirBest];
	}

	return ctptLastGood;
}

MoveDirections s_movd[50];
word s_nSeqMoveAside;
bool MobileUnitGob::PlanMoveAsidePath(MobileUnitGob *pmuntBlocking, TCoord txTo, TCoord tyTo)
{
	// Enemies won't move aside

	if (!IsAlly(pmuntBlocking->GetSide()))
		return false;

	// If attacking, won't move aside

	if (pmuntBlocking->m_st == kstAttack)
		return false;

    // Calc the direction this unit is trying to move. In general we prioritize
    // moving orthogonal to this direction, then backwards, then forwards.

	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	Direction dirMover = DirectionFromLocations(txFrom, tyFrom, txTo, tyTo);

    // Before starting inc the sequence number; this'll make sure we don't
    // visit gobs that have already been enumerated.

	s_nSeqMoveAside++;

    // Mark this unit so that it isn't asked to move aside (no backwards
    // support at the moment)

	m_nSeqMoveAside = s_nSeqMoveAside;

	// Add the first one to the list

	MoveDirections *pmovd = s_movd;
	pmuntBlocking->m_nSeqMoveAside = s_nSeqMoveAside;
	if (!pmuntBlocking->GetMoveAsideDirections(dirMover, pmovd))
		return false;

    // Enumerate mobile units in a chain until an open space to move to is
    // found.

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	bool fFound = false;
	while (true) {
		// Enumerate the possible moving directions for this gob.

		TCoord txUnit = TcFromWc(pmovd->pmunt->m_wx);
		TCoord tyUnit = TcFromWc(pmovd->pmunt->m_wy);
		MoveDirections *pmovdT = pmovd;
		for (; pmovd->idir < pmovd->cdir; pmovd->idir++) {
			// Enumerate the directions in order provided, get that munt

			Direction dir = pmovd->adir[pmovd->idir];
			TCoord txNext = txUnit + g_mpDirToDx[dir];
			TCoord tyNext = tyUnit + g_mpDirToDy[dir];
			if (txNext < 0 || txNext >= ctx || tyNext < 0 || tyNext >= cty)
				continue;
			MobileUnitGob *pmuntNext = GetMobileUnitAt(txNext, tyNext);

			// If this space is open we're done and we have a path we
			// can create

			if (pmuntNext == NULL) {
				fFound = true;
				break;
			}

			// Is there room to add another? If not then continue.

			if ((pmovd + 1) - s_movd >= ARRAYSIZE(s_movd))
				continue;

			// Is it an ally? Enemies won't move aside

			if (!IsAlly(pmuntNext->GetSide()))
				continue;

			// Attacking? If attacking, won't move aside

			if (pmuntNext->m_st == kstAttack)
				continue;

			// Try to add it to the list. It'll get added if it can move
			// in this direction.

			if (!pmuntNext->GetMoveAsideDirections(dirMover, pmovd + 1))
				continue;

			// Added ok; advance pmovd to this new entry, break out of 
			// loop to re-enter

			pmovd++;
			break;
		}

		// If another was added to the list, enumerate it

		if (pmovd != pmovdT)
			continue;

		// If we found a path, we're done; break out

		if (fFound)
			break;

        // Couldn't find a munt from this munt that would move aside. Go back
        // to the previous munt and continue enumerating if there are any left

		if (pmovd == s_movd)
			break;
		pmovd--;
		pmovd->idir++;
	}
	if (!fFound)
		return false;


	// Walk backwards through the move directions asking units to move aside

	for (MoveDirections *pmovdT = pmovd; pmovdT >= s_movd; pmovdT--) {
		if (!pmovdT->pmunt->AcceptMoveAsideRequest(pmovdT->adir[pmovdT->idir]))
			return false;
	}

	// This unit needs to move wait on the column that is moving aside

	MoveWaitForUnit(pmuntBlocking, txTo, tyTo);
	return true;
}

MobileUnitGob *MobileUnitGob::GetMobileUnitAt(TCoord tx, TCoord ty)
{
	// Enum gids here

	for (Gid gid = ggobm.GetFirstGid(tx, ty); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
		// Any mobile units in this tile?

		MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(gid);
		if (pmunt == NULL)
			continue;
		if (!(pmunt->m_ff & kfGobMobileUnit))
			continue;
		if (pmunt->IsMobile())
			continue;
		if (pmunt->IsMoveWaiting())
			continue;
		return pmunt;
	}

	return NULL;
}

// Move asides look directional independently in this order: sides first, backwards, then forwards

Direction s_mpDirToDirsSorted[8][8] = {
	{ kdirW, kdirE, kdirSW, kdirSE, kdirNW, kdirNE, kdirS, kdirN, },
	{ kdirNW, kdirSE, kdirW, kdirS, kdirN, kdirE, kdirSW, kdirNE, },
	{ kdirN, kdirS, kdirNW, kdirSW, kdirNE, kdirSE, kdirW, kdirE, },
	{ kdirNE, kdirSW, kdirN, kdirW, kdirE, kdirS, kdirNW, kdirSE, },
	{ kdirE, kdirW, kdirNE, kdirNW, kdirSE, kdirSW, kdirN, kdirS, },
	{ kdirSE, kdirNW, kdirE, kdirN, kdirS, kdirW, kdirNE, kdirSW, },
	{ kdirS, kdirN, kdirSE, kdirNE, kdirSW, kdirNW, kdirE, kdirW, },
	{ kdirSW, kdirNE, kdirS, kdirE, kdirW, kdirN, kdirSE, kdirNW, },
};

// NOTE: Should this be overridable (virtual or a message)?

bool MobileUnitGob::GetMoveAsideDirections(Direction dirMover, MoveDirections *pmovd)
{
	// Special hack for miners

	if (m_st == kstMinerRotateForEntry)
		return false;

	// Initialize

	pmovd->pmunt = this;
	pmovd->cdir = 0;
	pmovd->idir = 0;

	// If this unit is being asked for move aside directions it has already been
	// marked, assert if not true.

	Assert(m_nSeqMoveAside == s_nSeqMoveAside);

	// Fill in reasonable move directons into pmovd. 

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	Direction *adir = s_mpDirToDirsSorted[dirMover];
	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);

	// Enumerate directions 

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	for (int idir = 0; idir < 8; idir++) {
		// Get mobile unit at this location

		Direction dir = adir[idir];
		TCoord tx = txFrom + g_mpDirToDx[dir];
		TCoord ty = tyFrom + g_mpDirToDy[dir];
		if (tx < 0 || tx >= ctx || ty < 0 || ty >= cty)
			continue;

		// If in attack state this new move aside position is only
		// plausible if it is still in range

		// Is this location blocked by terrain?

		if (ptrmap->IsBlocked(tx, ty, kbfStructure))
			continue;

		// See what munt is there if any

		MobileUnitGob *pmunt = GetMobileUnitAt(tx, ty);

		// Prioritize empty

		if (pmunt == NULL) {
            // Even if there is no munt there, the tile may be reserved by
            // another munt that is in transition to this tile. If the tile is
            // reserved, it is not a suitable destination

			if (IsTileReserved(tx, ty))
				continue;
			pmovd->adir[0] = dir;
			pmovd->cdir = 1;
			return true;
		}

		// Already been asked? If not it is fresh meat

		if (pmunt->m_nSeqMoveAside == s_nSeqMoveAside)
			continue;
		pmunt->m_nSeqMoveAside = s_nSeqMoveAside;

		// Mobile? Only non-mobile munts are candidates

		if (pmunt->IsMobile())
			continue;

		// Either a viable munt is there or nothing is there, in any case add
		// it to the list

		pmovd->adir[pmovd->cdir++] = dir;
	}

	return pmovd->cdir != 0;
}

bool MobileUnitGob::AcceptMoveAsideRequest(Direction dir)
{
    // This unit has agreed to move aside in this direction and is now being
    // asked to do so.

    // Two cases: either the unit is standing or move waiting. If it is
    // standing then it can just move out of the way.

	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	TCoord txTo = txFrom + g_mpDirToDx[dir];
	TCoord tyTo = tyFrom + g_mpDirToDy[dir];

	// Unit not move waiting. 
    // Move in this direction. This has already been evaluated to be safe and
    // sane.

	Assert(m_ppathUnit == NULL);
	Assert(!IsMoveWaiting());

	WCoord wxDst = WcFromTc(txTo) + kwcTileHalf;
	WCoord wyDst = WcFromTc(tyTo) + kwcTileHalf;

    // Issue move command. These occur in order so earlier units can move wait
    // on later units. We don't want overridden behavior (such as the miner
    // mining galaxite) to interfere with move aside, so call
    // MobileUnitGob::SetTarget directly.

	MobileUnitGob::SetTarget(kgidNull, WcFromTc(txTo) + kwcTileHalf, WcFromTc(tyTo) + kwcTileHalf);
	return true;
}

int MobileUnitGob::GetNextLocations(TPoint *atpt)
{
	// Use avoid path if it exists

	int cStepsFurtherStop = 3;
	if (m_ppathAvoid != NULL) {
		// Are we at the destination yet? 

		TCoord txFrom = TcFromWc(m_wx);
		TCoord tyFrom = TcFromWc(m_wy);
		TPoint tptT;
		bool fEnd = false;
		if (m_ppathAvoid->GetPoint(m_ppathAvoid->GetCount() - 1, &tptT, 0)) {
			if (tptT.tx == txFrom && tptT.ty == tyFrom)
				fEnd = true;
		} else {
			fEnd = true;
		}

		// If we still have more to avoid, do it

		if (!fEnd) {
			int itptDummy = 0;
			int ctpt = GetNextLocations2(m_ppathAvoid, &itptDummy, m_ppathAvoid->GetCount(), atpt);
			if (ctpt != 0)
				return ctpt;
		}

		// Avoid path is done; delete it

		cStepsFurtherStop = _min(cStepsFurtherStop, m_ppathAvoid->GetCount());
		delete m_ppathAvoid;
		m_ppathAvoid = NULL;
	}

	// Use unit path

	int ctpt = GetNextLocations2(m_ppathUnit, &m_itptPath, cStepsFurtherStop, atpt);
	if (ctpt == 0)
		return 0;
	if (m_itptPath > 1)
		m_ppathUnit->SetCacheIndex(m_itptPath - 2);
	return ctpt;
}

int MobileUnitGob::GetNextLocations2(Path *ppath, int *pitptStart, int cStepsFurtherStop, TPoint *atpt)
{
    // The current progress is measured relative to the unit path. Then
    // surrounding locations are profiled for making better progress. Positions
    // with better progress are returned as move destinations.

	// Initialize a trackpoint

	TCoord txFrom = TcFromWc(m_wx);
	TCoord tyFrom = TcFromWc(m_wy);
	Assert(GetReservingUnit(txFrom, tyFrom) == this);
	Assert(IsTileReserved(txFrom, tyFrom));
	TrackPoint trkpStart;
	if (!trkpStart.Init(ppath, txFrom, tyFrom, *pitptStart, cStepsFurtherStop))
		return false;
	TPoint tptClosest;
	trkpStart.GetClosestPoint(&tptClosest);
	int itptClosest = trkpStart.GetClosestPointIndex();
	*pitptStart = itptClosest;

	// If we're at the end we're done

	if (itptClosest == ppath->GetCount() - 1) {
		if (tptClosest.tx == txFrom && tptClosest.ty == tyFrom)
			return 0;
	}

	// See if we're close to the next step already

	TPoint tptNext;
	if (ppath->GetPoint(itptClosest + 1, &tptNext, kbfStructure | kbfMobileUnit)) {
		// Have a next step and it is free of obstacles. If we're on the path
		// then we're close to it, return it

		if (abs(tptNext.tx - txFrom) <= 1 && abs(tptNext.ty - tyFrom) <= 1) {
			Assert(tptNext.tx != txFrom || tptNext.ty != tyFrom);
			atpt[0] = tptNext;
			return 1;
		}
	}

	// Maybe not actually on "next" but closer to closest. If so take it

    byte bf = GetTileFlags(tptClosest.tx, tptClosest.ty);
	if (!(bf & (kbfStructure | kbfMobileUnit))) {
		if (abs(tptClosest.tx - txFrom) <= 1 && abs(tptClosest.ty - tyFrom) <= 1) {
			Assert(tptClosest.tx != txFrom || tptClosest.ty != tyFrom);
			atpt[0] = tptClosest;
			return 1;
		}
	}

	// Get after point of trackpoint

	TPoint tptAfter;
	trkpStart.GetAfterPoint(&tptAfter);

    // Enumerate the surrounding tiles and measure each for progress along the
    // unit path.

	int itptClosestMeasure = (itptClosest == 0 ? 0 : itptClosest - 1);
	TrackPoint atrkp[8];
	int ctpt = 0;
	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	for (Direction dir = 0; dir < 8; dir++) {
		TCoord tx = txFrom + g_mpDirToDx[dir];
		TCoord ty = tyFrom + g_mpDirToDy[dir];
		if (tx < 0 || tx >= ctx || ty < 0 || ty >= cty)
			continue;
		if (ptrmap->IsBlocked(tx, ty, 0))
			continue;
		atrkp[ctpt].Init(ppath, tx, ty, itptClosestMeasure, 1);
		if (trkpStart.IsProgress(&atrkp[ctpt])) {
			// If blocked by terrain, not valid progress

			if (ptrmap->IsLineOccupied(tx, ty, tptAfter.tx, tptAfter.ty, 0))
				continue;

			// Looks like a good candidate

			ctpt++;
		}
	}

    // If there are no unblocked positions, take the next one even if it is
    // blocked and callers will try and move around it.

	if (ctpt == 0) {
		for (Direction dir = 0; dir < 8; dir++) {
			TCoord tx = txFrom + g_mpDirToDx[dir];
			TCoord ty = tyFrom + g_mpDirToDy[dir];
			if (tx < 0 || tx >= ctx || ty < 0 || ty >= cty)
				continue;
			atrkp[0].Init(ppath, tx, ty, itptClosestMeasure, 1);
			if (trkpStart.IsProgress(&atrkp[0])) {
				atpt[0].tx = tx;
				atpt[0].ty = ty;
				return 1;
			}
		}
	}

	// Sort the candidates.

	int an[8];
	int i;
	for (i = 0; i < ctpt; i++)
		an[i] = i;
	for (i = ctpt - 1; i >= 0; i--) {
		for (int j = 1; j <= i; j++) {
			if (atrkp[an[j - 1]].IsBetterSort(&atrkp[an[j]])) {
				int n = an[j - 1];
				an[j - 1] = an[j];
				an[j] = n;
			}
		}
	}
	for (i = 0; i < ctpt; i++)
		atrkp[an[i]].GetInitialPoint(&atpt[i]);

	// Done

	return ctpt;
}

bool MobileUnitGob::IsMobile()
{
	if (m_ppathUnit == NULL && !(m_wfMunt & (kfMuntPathPending | kfMuntAtReplicatorInput)))
		return false;
	return true;
}

bool MobileUnitGob::CheckReplicatorPoint()
{
	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);
	int cReplicators = ReplicatorGob::GetReplicatorCount();
	for (int n = 0; n < cReplicators; n++) {
		TPoint tptReplicator;
		ReplicatorGob::GetReplicatorInputPoint(n, &tptReplicator);
		if (tptReplicator.tx == tx && tptReplicator.ty == ty)
			return true;
	}
	return false;
}

void MobileUnitGob::SetPosition(WCoord wx, WCoord wy)
{
	// Check if nothing to do

	if (m_wx == wx && m_wy == wy)
		return;

	// Notify waiters

	if (!InTransition()) {
		if (m_wfMunt & kfMuntMoveWaitingNearby)
			NotifyMoveWaitingNearby(m_wx, m_wy);
	}

	// Mark for redraw if this gob has changed pixel locations

	if (PcFromWc(wx) != PcFromWc(m_wx) || PcFromWc(wy) != PcFromWc(m_wy))
		MarkRedraw();

	// Be sure not to sleep on this important new development

	m_unvl.MinSkip();

	// Keep GobMgr in the loop so it can maintain proper depth sorting

	bool fTileChange = ggobm.MoveGob(this, m_wx, m_wy, wx, wy);
	WCoord wxOld = m_wx;
	WCoord wyOld = m_wy;
	m_wx = wx;
	m_wy = wy;

	// Special tasks if we've moved between tiles

	if (fTileChange) {
		// Reveal fog

		if (m_pplr == gpplrLocal) {
			WCoord wxView, wyView;
			gsim.GetViewPos(&wxView, &wyView);
			FogMap *pfogm = gsim.GetLevel()->GetFogMap();
			TPoint tpt;
			GetTilePosition(&tpt);
			RevealPattern *prvlp = (RevealPattern *)(m_puntc->wf & kfUntcLargeDefog ? grvlpLarge : grvlp);
			pfogm->Reveal(tpt.tx, tpt.ty, prvlp, gpupdSim, wxView, wyView);
		}

		// Update minimap

		if (gpmm != NULL) {
			TRect trc;
			trc.left = wxOld < wx ? TcFromWc(wxOld) : TcFromWc(wx);
			trc.top = wyOld < wy ? TcFromWc(wyOld) : TcFromWc(wy);
			trc.right = (wxOld > wx ? TcFromWc(wxOld) : TcFromWc(wx)) + 1;
			trc.bottom = (wyOld > wy ? TcFromWc(wyOld) : TcFromWc(wy)) + 1;
			gpmm->RedrawTRect(&trc);
		}

		// Notify enemy of this gob nearby

		NotifyEnemyNearby();

		// Move between areas

		AreaMask amOld = ggobm.CalcAreaMask(TcFromWc(wxOld), TcFromWc(wyOld));
		AreaMask amNew = ggobm.CalcAreaMask(TcFromWc(m_wx), TcFromWc(m_wy));
		ggobm.MoveGobBetweenAreas(m_gid, amOld, amNew);
	}
}

UnitGob *MobileUnitGob::FindValidTargetInArea(int nArea)
{
	Enum enm;
	while (true) {
		Gob *puntTarget = ggobm.EnumGobsInArea(&enm, m_msgAction.GuardAreaCommand.nArea, ~m_pplr->GetAllies(), kumMobileUnits);
		if (puntTarget == NULL)
			return NULL;
		if (IsValidTarget(puntTarget)) {
			Assert(puntTarget->GetFlags() & kfGobUnit);
			return (UnitGob *)puntTarget;
		}
	}
}

#ifdef MP_DEBUG_SHAREDMEM
void MobileUnitGob::MPValidate()
{
	MPValidateGobMember(MobileUnitGob, m_dir);
	MPValidateGobMember(MobileUnitGob, m_dirNext);
	MPValidateGobMember(MobileUnitGob, m_gidTarget);
	MPValidateGobMember(MobileUnitGob, m_tLastFire);
	MPValidateGobMember(MobileUnitGob, m_msgPending);
	MPValidateGobMember(MobileUnitGob, m_msgAction);
	MPValidateGobMember(MobileUnitGob, m_cCountdown);
	MPValidateGobMember(MobileUnitGob, m_cMoveStepsRemaining);
	MPValidateGobMember(MobileUnitGob, m_tptChaseInitial);
	MPValidateGobMember(MobileUnitGob, m_mua);
	MPValidateGobMember(MobileUnitGob, m_muaPending);
	MPValidateGobMember(MobileUnitGob, m_wfMunt);
	MPValidateGobMember(MobileUnitGob, m_stPending);
	MPValidateGobMember(MobileUnitGob, m_cupdLastHitOrNearbyAllyHit);
	MPValidateGobMember(MobileUnitGob, m_wxDst);
	MPValidateGobMember(MobileUnitGob, m_wyDst);
	MPValidateGobMember(MobileUnitGob, m_txDst);
	MPValidateGobMember(MobileUnitGob, m_tyDst);
	MPValidateGobMember(MobileUnitGob, m_wptTarget);
	MPValidateGobMember(MobileUnitGob, m_nSeqMoveAside);
	MPValidateGobMember(MobileUnitGob, m_itptPath);
	MPValidateGobMember(MobileUnitGob, m_wptTargetCenter);
	MPValidateGobMember(MobileUnitGob, m_tcTargetRadius);
	MPValidateGobMember(MobileUnitGob, m_wcMoveDistPerUpdate);
	UnitGob::MPValidate();
}
#endif

} // namespace wi
