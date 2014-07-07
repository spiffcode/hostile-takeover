#include "ht.h"

namespace wi {

static int s_anGunTowerTurretStripIndices[16] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };

//
// Abstract base class for Towers
//

bool TowerGob::InitClass(TowerConsts *ptwrc, IniReader *pini)
{
	if (!StructGob::InitClass(ptwrc, pini))
		return false;

	ptwrc->anFiringStripIndices = s_anGunTowerTurretStripIndices;
	ptwrc->wf |= kfUntcNotifyEnemyNearby | kfUntcNotifyPowerLowHigh;

	char szTemplate[10];
	itoa(ptwrc->gt, szTemplate, 10);
	return true;
}

void TowerGob::ExitClass(TowerConsts *ptwrc)
{
	StructGob::ExitClass(ptwrc);
}

TowerGob::TowerGob(TowerConsts *ptwrc) : StructGob(ptwrc)
{
	m_tLastFire = 0;
	m_gidTargetPrimary = kgidNull;
	m_gidTargetSecondary = kgidNull;
	m_wptTarget.wx = kwxInvalid;		// kxInvalid = no target location

	// Turret stuff

	m_dir16Turret = kdirS * 2;
	m_aniTurret.Init(m_ptwrc->panid);
	SetAnimationStrip(&m_aniTurret, m_ptwrc->anFiringStripIndices[m_dir16Turret]);
}

#define knVerTowerGobState 1
bool TowerGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerTowerGobState)
		return false;
	m_dir16Turret = pstm->ReadByte();
	m_aniTurret.Init(m_ptwrc->panid);
	SetAnimationStrip(&m_aniTurret, m_ptwrc->anFiringStripIndices[m_dir16Turret]);
	m_gidTargetPrimary = pstm->ReadWord();
	m_gidTargetSecondary = pstm->ReadWord();
	m_tLastFire = gsim.GetTickCount() - pstm->ReadDword();
	m_wptTarget.wx = pstm->ReadWord();
	m_wptTarget.wy = pstm->ReadWord();
	return StructGob::LoadState(pstm);
}

bool TowerGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerTowerGobState);
	pstm->WriteByte(m_dir16Turret);
	pstm->WriteWord(m_gidTargetPrimary);
	pstm->WriteWord(m_gidTargetSecondary);
	pstm->WriteDword(gsim.GetTickCount() - m_tLastFire);
	pstm->WriteWord(m_wptTarget.wx);
	pstm->WriteWord(m_wptTarget.wy);
	return StructGob::SaveState(pstm);
}

void TowerGob::Activate()
{
	NotifyEnemyNearby();
	StructGob::Activate();
}

#ifdef DRAW_LINES
void TowerGob::DrawTargetLine(DibBitmap *pbm, int xViewOrigin, int yViewOrigin)
{
	if (m_wptTarget.wx == kwxInvalid)
		return;

	WPoint wptCenter;
	GetCenter(&wptCenter);
	pbm->DrawLine(PcFromUwc(wptCenter.wx) - xViewOrigin, PcFromUwc(wptCenter.wy) - yViewOrigin,
			PcFromUwc(m_wptTarget.wx) - xViewOrigin, PcFromUwc(m_wptTarget.wy) - yViewOrigin,
			GetSideColor(m_pplr->GetSide()));
}
#endif

void TowerGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	// Draw base

	StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	// Draw turret

	if (nLayer == knLayerDepthSorted) {

		// Don't draw the turret on top of the destroyed base

		if (m_ani.GetStrip() != 2) {
			Side side = m_pplr->GetSide();
			if (m_ff & kfGobDrawFlashed)
				side = (Side)-1;
			else if (m_ff & kfGobBeingBuilt)
				side = ksideNeutral;

			// The turret is aligned with the base's special point

			Point ptBaseSpecial;
			m_ani.GetSpecialPoint(&ptBaseSpecial);
			m_aniTurret.Draw(pbm, PcFromUwc(m_wx) - xViewOrigin + ptBaseSpecial.x, 
					PcFromUwc(m_wy) - yViewOrigin + ptBaseSpecial.y, side);
		}
	}
}

bool TowerGob::IsTakeoverable(Player *pplr)
{
	// towers are not Takeoverable
	return false;
}

bool TowerGob::IsValidTarget(Gob *pgobTarget)
{
	// most structures cannot attack, so bypass the struct class on this one
	// TUNE:
	// however, since we can't move, let's make sure the target is in range ?
	//if IsGobWithinRange(pgobTarget, m_ptwrc->tcFiringRange)

	return UnitGob::IsValidTarget(pgobTarget);
}

void TowerGob::SetTarget(Gid gid, WCoord wx, WCoord wy, WCoord wxCenter, WCoord wyCenter, TCoord tcRadius, WCoord wcMoveDistPerUpdate)
{
	// If target doesn't exist anymore, ignore

	Gob *pgobTarget = ggobm.GetGob(gid);
	if (pgobTarget == NULL)
		return;

	// If target is an ally, ignore

	if (IsAlly(pgobTarget->GetSide()))
		return;

	// Attack it!
	// Flash the target Gob

	if (!((UnitGob *)pgobTarget)->IsAlly(gpplrLocal->GetSide()))
		pgobTarget->Flash();

	// Queue message

	Message msgT;
	msgT.mid = kmidAttackCommand;
	msgT.smidSender = m_gid;
	msgT.smidReceiver = m_gid;
	msgT.AttackCommand.wptTarget.wx = 0;
	msgT.AttackCommand.wptTarget.wy = 0;
	msgT.AttackCommand.gidTarget = gid;
	gcmdq.Enqueue(&msgT);
}

void TowerGob::DefUpdate()
{
	// Try to make the turret point toward the target
	// UNDONE: unless we're already firing

	// BUGBUG: this usage of the special point effectively makes the turret's direction
	// resolution dependent which will break multiplayer games between devices of
	// differing resolution.

	Point ptSpecial;
	m_ani.GetSpecialPoint(&ptSpecial);
	WCoord wx = m_wx + WcFromPc(ptSpecial.x);
	WCoord wy = m_wy + WcFromPc(ptSpecial.y);

	if (m_wptTarget.wx != kwxInvalid) {

		// If the target is in the tile we're in stop trying to look exactly at it

		if (WcTrunc(wx) != WcTrunc(m_wptTarget.wx) || WcTrunc(wy) != WcTrunc(m_wptTarget.wy)) {

			Direction16 dir16 = CalcDir16(m_wptTarget.wx - wx, m_wptTarget.wy - wy);

			// Make sure we're facing the way we want to go

			int d = dir16 - m_dir16Turret;
			if (d != 0) {
				if (d < -8)
					d = 1;
				else if (d > 8)
					d = -1;
				if (d < 0)
					m_dir16Turret--;
				else
					m_dir16Turret++;
				m_dir16Turret = ((unsigned int)m_dir16Turret) % 16;
				SetAnimationStrip(&m_aniTurret, m_ptwrc->anFiringStripIndices[m_dir16Turret]);
				m_unvl.MinSkip();
			}
		}
	}

	AdvanceAnimation(&m_aniTurret);

	StructGob::DefUpdate();
}

UnitGob *TowerGob::FindEnemyNearby(TCoord tcRange)
{
	// smart targeting requires a surveillance center and power

	if (!m_pplr->IsPowerLow() && (m_pplr->GetUnitCount(kutRadar) > 0)) {
		return UnitGob::FindEnemyNearby(tcRange);
	} else {
		return NULL;
	}
}

int TowerGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidEnemyNearby)
		// Notification that an enemy is nearby

		RememberEnemyNearby(pmsg->EnemyNearby.gidEnemy);
		m_unvl.MinSkip();

	OnMsg(kmidAttackCommand)
		// stash attack parameters to be picked up by the attack state

		m_gidTargetPrimary = pmsg->AttackCommand.gidTarget;
		SetState(kstAttack);

	OnMsg(kmidFire)
		UnitGob *puntTarget = (UnitGob *)ggobm.GetGob(m_gidTargetPrimary);
		if (puntTarget == NULL || !IsGobWithinRange(puntTarget, m_ptwrc->tcFiringRange))
			puntTarget = (UnitGob *)ggobm.GetGob(m_gidTargetSecondary);
		if (puntTarget != NULL) {
			// Fire off the shot!

			WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
			WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;

			Point ptTurretSpecial;
			m_aniTurret.GetSpecialPoint(&ptTurretSpecial, 1);
			Point ptBaseSpecial;
			m_ani.GetSpecialPoint(&ptBaseSpecial);

			WPoint wpt;
			puntTarget->GetCenter(&wpt);
			CreateBulletGob(m_wx + WcFromPc(ptBaseSpecial.x + ptTurretSpecial.x), 
					m_wy + WcFromPc(ptBaseSpecial.y + ptTurretSpecial.y), 
					wpt.wx + wdxRnd, wpt.wy + wdyRnd, 
					GetDamageTo(puntTarget) / 3, m_gid, puntTarget->GetId());
		}

	OnMsg(kmidHit)
		// It is possible to be hit by someone without having a chance to target
		// it first. The worse case of this is when an enemy is lost track of
		// due to the short m_agidEnemyNearby. In any case, if we aren't already
		// busy with something else attack the assailant.

		if (m_gidTargetSecondary == kgidNull && !m_pplr->IsPowerLow() && m_pplr->GetUnitCount(kutRadar) > 0) {
			m_gidTargetSecondary = pmsg->Hit.gidAssailant;
			SetState(kstAttack);
		}

		return StructGob::ProcessStateMachineMessage(st, pmsg);

	//-----------------------------------------------------------------------

	State(kstIdle)
		OnEnter
			m_wptTarget.wx = kwxInvalid;	// no target

		//OnMsg(kmidHit) - if power is low then we'll end up taking hits that 
		// we don't respond to. Sometimes we'll take a first hit before we respond too. C'est le guerre

		OnUpdate
			// any enemy in range?

			UnitGob *puntTarget = FindEnemyNearby(m_ptwrc->tcFiringRange);
			if (puntTarget != NULL) {

				// let's have a little focus

// DWM: this can occur if the tower loses power while it has a secondary target
// It's OK because it just means the tower picks a new secondary when it wakes up
//				Assert(m_gidTargetSecondary == kgidNull);
				m_gidTargetSecondary = puntTarget->GetId();
				SetState(kstAttack);
			} 

			// Handle flashing, turret pointing, firing animation

			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstAttack)
		OnUpdate

			// if power is low then we're just not going to do anything

			UnitGob* puntTarget = NULL;
			if (!GetOwner()->IsPowerLow()) {
			
				// look for our primary (player-set) enemy first

				if (m_gidTargetPrimary != kgidNull) {
					puntTarget = (UnitGob *)ggobm.GetGob(m_gidTargetPrimary);
					if (puntTarget == NULL || !IsValidTarget(puntTarget)) {

						// primary enemy is dead/gone or taken over, we can forget it.

						m_gidTargetPrimary = kgidNull;
						puntTarget = NULL;
					} else if (!IsGobWithinRange(puntTarget, m_ptwrc->tcFiringRange)) {
						
						// primary target not in range

						puntTarget = NULL;
					}
				}

				if (puntTarget == NULL) {

					// primary is not available, who can we shoot at in the meantime?

					puntTarget = (UnitGob *)ggobm.GetGob(m_gidTargetSecondary);
					if (puntTarget == NULL || !IsValidTarget(puntTarget) || 
							!IsGobWithinRange(puntTarget, m_ptwrc->tcFiringRange)) {

						// secondary target not available, new secondary?

						puntTarget = FindEnemyNearby(m_ptwrc->tcFiringRange);
						if (puntTarget != NULL) 
							m_gidTargetSecondary = puntTarget->GetId();
						else
							m_gidTargetSecondary = kgidNull;
					}
				}
			}

			if (puntTarget != NULL) {
				
				//  we've got one, fire at the enemy

				Point ptSpecial;
				m_ani.GetSpecialPoint(&ptSpecial);
				puntTarget->GetAttackPoint(&m_wptTarget);
				Fire(puntTarget, m_wptTarget.wx, m_wptTarget.wy, m_wptTarget.wx - (m_wx + WcFromPc(ptSpecial.x)), 
						m_wptTarget.wy - (m_wy + WcFromPc(ptSpecial.y)));
			} else {

				// no valid targets in range

				SetState(kstIdle);
			}

			// Handle flashing

			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstDying)
		// Override to avoid animating through the Galaxite level frames
		OnUpdate

	//-----------------------------------------------------------------------

#if 0
EndStateMachineInherit(StructGob)
#else
            return knHandled;
        }
    } else {
        return (int)StructGob::ProcessStateMachineMessage(st, pmsg); 
    }
    return (int)StructGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

dword TowerGob::GetAnimationHash() {
    dword dw = StructGob::GetAnimationHash();
    return dw ^ (m_dir16Turret << 8);
}

void TowerGob::GetAnimationBounds(Rect *prc, bool fBase) {
    StructGob::GetAnimationBounds(prc, fBase);

    if (!fBase) {
        Rect rcTurret;
        m_aniTurret.GetBounds(&rcTurret);
        Point ptBaseSpecial;
        m_ani.GetSpecialPoint(&ptBaseSpecial);
        rcTurret.Offset(ptBaseSpecial.x, ptBaseSpecial.y);
        prc->Union(&rcTurret);
    }
}

void TowerGob::DrawAnimation(DibBitmap *pbm, int x, int y)
{
    Side side = m_pplr->GetSide();
    StructGob::DrawAnimation(pbm, x, y);
    Point ptBaseSpecial;
    m_ani.GetSpecialPoint(&ptBaseSpecial);
    m_aniTurret.Draw(pbm, x + ptBaseSpecial.x, y + ptBaseSpecial.y, side);
}

void TowerGob::GetClippingBounds(Rect *prc)
{
	StructGob::GetClippingBounds(prc);
	Rect rcTurret;
	m_aniTurret.GetBounds(&rcTurret);
	Point ptBaseSpecial;
	m_ani.GetSpecialPoint(&ptBaseSpecial);
	rcTurret.Offset(ptBaseSpecial.x + PcFromUwc(m_wx), ptBaseSpecial.y + PcFromUwc(m_wy));
	prc->Union(&rcTurret);
}

//
// GunTowerGob implementation
//

static TowerConsts gtwrcGun;

bool GunTowerGob::InitClass(IniReader *pini)
{
	gtwrcGun.gt = kgtMachineGunTower;
	gtwrcGun.ut = kutMachineGunTower;
	gtwrcGun.umPrerequisites = kumReactor | kumRadar;

	// Sound effects

	gtwrcGun.sfxAbortRepair = ksfxMachineGunTowerAbortRepair;
	gtwrcGun.sfxAttack = ksfxMachineGunTowerAttack;
	gtwrcGun.sfxDamaged = ksfxMachineGunTowerDamaged;
	gtwrcGun.sfxDestroyed = ksfxMachineGunTowerDestroyed;
	gtwrcGun.sfxFire = ksfxMachineGunTowerFire;
	gtwrcGun.sfxImpact = ksfxNothing;
	gtwrcGun.sfxRepair = ksfxMachineGunTowerRepair;
	gtwrcGun.sfxSelect = ksfxMachineGunTowerSelect;

	return TowerGob::InitClass(&gtwrcGun, pini);
}

void GunTowerGob::ExitClass()
{
	TowerGob::ExitClass(&gtwrcGun);
}

GunTowerGob::GunTowerGob() : TowerGob(&gtwrcGun)
{
}

// UNDONE: UnitGob can handle some of this (e.g., timing)

bool GunTowerGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// Make sure we're facing the way we want to fire before we try to fire

	Direction16 dir16Fire = CalcDir16(wdx, wdy);
	if (m_dir16Turret != dir16Fire) {
		m_unvl.MinSkip();
		return false;
	}

	// Firing rate is limited by ctFiringRate

	long t = gsim.GetTickCount();
	long ctWait = m_ptwrc->ctFiringRate;
	long ctRemaining = ctWait - (t - m_tLastFire);
	if (ctRemaining > 0) {
		m_unvl.MinSkip((ctRemaining + (kctUpdate / 2)) / kctUpdate - 1);
		return false;
	}

	m_tLastFire = t;

	// Play firing animation

	StartAnimation(&m_aniTurret, m_ptwrc->anFiringStripIndices[dir16Fire], 0, kfAniResetWhenDone);
	gsmm.SendDelayedMsg(kmidFireComplete, m_aniTurret.GetRemainingStripTime(), m_gid, m_gid);

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;

	Point ptTurretSpecial;
	m_aniTurret.GetSpecialPoint(&ptTurretSpecial, 1);
	Point ptBaseSpecial;
	m_ani.GetSpecialPoint(&ptBaseSpecial);

	CreateBulletGob(m_wx + WcFromPc(ptBaseSpecial.x + ptTurretSpecial.x), 
			m_wy + WcFromPc(ptBaseSpecial.y + ptTurretSpecial.y), 
			wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget) / 3, m_gid, puntTarget->GetId());

	// Two more shots at slight delays

	gsmm.SendDelayedMsg(kmidFire, kctUpdate * 2, m_gid, m_gid);
	gsmm.SendDelayedMsg(kmidFire, kctUpdate * 4, m_gid, m_gid);

	// Play sound

	gsndm.PlaySfx(m_ptwrc->sfxFire);

	return true;
}

#if defined(DEBUG_HELPERS)
char *GunTowerGob::GetName()
{
	return "GunTower";
}
#endif

//
// RocketTowerGob implementation
//

static TowerConsts gtwrcRocket;

bool RocketTowerGob::InitClass(IniReader *pini)
{
	gtwrcRocket.gt = kgtRocketTower;
	gtwrcRocket.ut = kutRocketTower;
	gtwrcRocket.umPrerequisites = kumReactor | kumRadar;

	// Sound effects

	gtwrcRocket.sfxAbortRepair = ksfxRocketTowerAbortRepair;
	gtwrcRocket.sfxAttack = ksfxRocketTowerAttack;
	gtwrcRocket.sfxDamaged = ksfxRocketTowerDamaged;
	gtwrcRocket.sfxDestroyed = ksfxRocketTowerDestroyed;
	gtwrcRocket.sfxFire = ksfxRocketTowerFire;
	gtwrcRocket.sfxImpact = ksfxRocketTowerImpact;
	gtwrcRocket.sfxRepair = ksfxRocketTowerRepair;
	gtwrcRocket.sfxSelect = ksfxRocketTowerSelect;
	return TowerGob::InitClass(&gtwrcRocket, pini);
}

void RocketTowerGob::ExitClass()
{
	TowerGob::ExitClass(&gtwrcRocket);
}

RocketTowerGob::RocketTowerGob() : TowerGob(&gtwrcRocket)
{
}

#if defined(DEBUG_HELPERS)
char *RocketTowerGob::GetName()
{
	return "RocketTower";
}
#endif

// UNDONE: UnitGob can handle some of this (e.g., timing)

bool RocketTowerGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// Firing rate is limited by ctFiringRate

	long t = gsim.GetTickCount();
	long ctWait = m_ptwrc->ctFiringRate;
	long ctRemaining = ctWait - (t - m_tLastFire);
	if (ctRemaining > 0) {
		m_unvl.MinSkip((ctRemaining + (kctUpdate / 2)) / kctUpdate - 1);
		return false;
	}

	Direction16 dir16Fire = CalcDir16(wdx, wdy);
	if (m_dir16Turret != dir16Fire)
		return false;

	m_tLastFire = t;

	// Play firing animation (start on frame 1 where the action is)

	StartAnimation(&m_aniTurret, m_ptwrc->anFiringStripIndices[dir16Fire], 1, kfAniIgnoreFirstAdvance | kfAniResetWhenDone);
	gsmm.SendDelayedMsg(kmidFireComplete, m_aniTurret.GetRemainingStripTime(), m_gid, m_gid);

	// Fire off the shot!

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptTurretSpecial;
	m_aniTurret.GetSpecialPoint(&ptTurretSpecial);
	Point ptBaseSpecial;
	m_ani.GetSpecialPoint(&ptBaseSpecial);

	// This little hack to handle rocket firing is easier/cheaper than having
	// the RocketTankGob override TankGob::Fire

	CreateRocketGob(m_wx + WcFromPc(ptBaseSpecial.x + ptTurretSpecial.x), 
			m_wy + WcFromPc(ptBaseSpecial.y + ptTurretSpecial.y), 
			wx + wdxRnd, wy + wdyRnd, GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sound

	gsndm.PlaySfx(m_ptwrc->sfxFire);

	return true;
}

} // namespace wi
