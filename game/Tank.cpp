#include "ht.h"

namespace wi {

const int kcMTankSecondShot = 2;	// Frame count of first shot (ignoring 1st frame) - MTank Anims need to match!
const int kifrmTankAction = 1;		// frame where the shot really starts

//
// Abstract Tank
//

bool TankGob::InitClass(MobileUnitConsts *pmuntc, IniReader *pini)
{
	pmuntc->wf |= kfUntcNotifyEnemyNearby;
	return MobileUnitGob::InitClass(pmuntc, pini);
}

void TankGob::ExitClass(MobileUnitConsts *pmuntc)
{
	MobileUnitGob::ExitClass(pmuntc);
}

TankGob::TankGob(MobileUnitConsts *pmuntc) : MobileUnitGob(pmuntc)
{
	m_dir = kdirS;

	// Turret stuff

	m_dir16Turret = m_dir * 2;
}

bool TankGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	if (!MobileUnitGob::Init(wx, wy, pplr, fxHealth, ff, pszName))
		return false;

	StartAnimation(&m_ani, m_dir, 0, 0);
	m_aniTurret.Init(m_pmuntc->panid);
	SetAnimationStrip(&m_aniTurret, m_pmuntc->anFiringStripIndices[m_dir16Turret]);
	return true;
}

#define knVerTankGobState 1
bool TankGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerTankGobState)
		return false;
	m_dir16Turret = pstm->ReadByte();
	m_aniTurret.Init(m_pmuntc->panid);
	SetAnimationStrip(&m_aniTurret, m_pmuntc->anFiringStripIndices[m_dir16Turret]);
	return MobileUnitGob::LoadState(pstm);
}

bool TankGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerTankGobState);
	pstm->WriteByte(m_dir16Turret);
	return MobileUnitGob::SaveState(pstm);
}

void TankGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	if (nLayer == knLayerDepthSorted) {

#ifdef DRAW_OCCUPIED_TILE_INDICATOR
		{
			WRect wrcT;
			GetTilePaddedWRect(&wrcT);
			Rect rcT;
			rcT.FromWorldRect(&wrcT);
			rcT.Offset(-xViewOrigin, -yViewOrigin);
			DrawBorder(pbm, &rcT, 1, GetColor(kiclrWhite));
		}
#endif
		Side side = m_pplr->GetSide();
		if (m_ff & kfGobDrawFlashed)
			side = (Side)-1;

		// Draw base

		int x = PcFromUwc(m_wx) - xViewOrigin;
		int y = PcFromUwc(m_wy) - yViewOrigin;
		m_ani.Draw(pbm, x, y, side);

		// Draw turret
		// The turret is aligned with the base's special point

		Point ptBaseSpecial;
		m_ani.GetSpecialPoint(&ptBaseSpecial);
		m_aniTurret.Draw(pbm, x + ptBaseSpecial.x, y + ptBaseSpecial.y, side);
	} else {
		MobileUnitGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);
	}
}

// UNDONE: MobileUnitGob can handle some of this (e.g., timing)

bool TankGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	Direction16 dir16Fire = CalcDir16(wdx, wdy);
	if (m_dir16Turret != dir16Fire) {
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

	StartAnimation(&m_aniTurret, m_pmuntc->anFiringStripIndices[dir16Fire], kifrmTankAction, kfAniIgnoreFirstAdvance | kfAniResetWhenDone);
	m_wfMunt |= kfMuntFiring;
	gsmm.SendDelayedMsg(kmidFireComplete, m_aniTurret.GetRemainingStripTime(), m_gid, m_gid);

	// Fire off the shot!

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptSpecial;
	m_aniTurret.GetSpecialPoint(&ptSpecial, kifrmTankAction);

	LaunchProjectile(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sound

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

// default. Overridden by several tanks
void TankGob::LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	CreateTankShotGob(wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget);
}

void TankGob::Idle()
{
	// 1/2 of the time we pivot the turret left, 1/2 we pivot it right

	if (GetRandom() & 1) {
		m_dir16Turret--;
		if (m_dir16Turret < 0)
			m_dir16Turret = 15;
	} else {
		m_dir16Turret++;
		if (m_dir16Turret > 15)
			m_dir16Turret = 0;
	}
	SetAnimationStrip(&m_aniTurret, m_pmuntc->anFiringStripIndices[m_dir16Turret]);
}

void TankGob::DefUpdate()
{
	// Try to make the turrent point toward the target (unless we're already firing)

	if (m_wptTarget.wx != kwxInvalid && !(m_wfMunt & kfMuntFiring)) {
		// Get the aim point. If we have a unit target, it is the unit's center
		// If we don't have a unit target, it is whatever is in m_wptTarget
		
		WPoint wptAim = m_wptTarget;
		Gob *pgobTarget = ggobm.GetGob(m_gidTarget);
		if (pgobTarget != NULL)
			pgobTarget->GetCenter(&wptAim);

		// If the target is in the tile we're in stop trying to look exactly at it

		if (WcTrunc(m_wx) != WcTrunc(wptAim.wx) || WcTrunc(m_wy) != WcTrunc(wptAim.wy)) {

			Direction16 dir16 = CalcDir16(wptAim.wx - m_wx, wptAim.wy - m_wy);

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
				SetAnimationStrip(&m_aniTurret, m_pmuntc->anFiringStripIndices[m_dir16Turret]);
				m_unvl.MinSkip();
			}
		}
	}

	AdvanceAnimation(&m_aniTurret);

	MobileUnitGob::DefUpdate();
}

dword TankGob::GetAnimationHash() {
    dword dw = MobileUnitGob::GetAnimationHash();
    return dw ^ (m_dir16Turret << 8);
}

void TankGob::GetAnimationBounds(Rect *prc, bool fBase) {
    MobileUnitGob::GetAnimationBounds(prc, fBase);

    if (!fBase) {
        Rect rcTurret;
        m_aniTurret.GetBounds(&rcTurret);
        Point ptBaseSpecial;
        m_ani.GetSpecialPoint(&ptBaseSpecial);
        rcTurret.Offset(ptBaseSpecial.x, ptBaseSpecial.y);
        prc->Union(&rcTurret);
    }
}

void TankGob::DrawAnimation(DibBitmap *pbm, int x, int y)
{
    Side side = m_pplr->GetSide();
    MobileUnitGob::DrawAnimation(pbm, x, y);
    Point ptBaseSpecial;
    m_ani.GetSpecialPoint(&ptBaseSpecial);
    m_aniTurret.Draw(pbm, x + ptBaseSpecial.x, y + ptBaseSpecial.y, side);
}

void TankGob::GetClippingBounds(Rect *prc)
{
	MobileUnitGob::GetClippingBounds(prc);
	Rect rcTurret;
	m_aniTurret.GetBounds(&rcTurret);
	Point ptBaseSpecial;
	m_ani.GetSpecialPoint(&ptBaseSpecial);
	rcTurret.Offset(ptBaseSpecial.x + PcFromUwc(m_wx), ptBaseSpecial.y + PcFromUwc(m_wy));
	prc->Union(&rcTurret);
}

//
// Light Tank
//

static MobileUnitConsts gLTankConsts;

#if defined(DEBUG_HELPERS)
char *LTankGob::GetName()
{
	return "LTank";
}
#endif

static int s_anTurretStripIndices[16] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
static int s_anBaseStripIndices[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
static int s_anIdleStripIndices[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

bool LTankGob::InitClass(IniReader *pini)
{
	gLTankConsts.gt = kgtLightTank;
	gLTankConsts.ut = kutLightTank;

	// Initialize the frame indices arrays

	gLTankConsts.anFiringStripIndices = s_anTurretStripIndices;
	gLTankConsts.anMovingStripIndices = s_anBaseStripIndices;
	gLTankConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gLTankConsts.sfxFire = ksfxLightTankFire;
	gLTankConsts.sfxImpact = ksfxLightTankImpact;

	gLTankConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gLTankConsts.sfxcSelect = ksfxcMajor02Select;
	gLTankConsts.sfxcMove = ksfxcMajor02Move;
	gLTankConsts.sfxcAttack = ksfxcMajor02Attack;

	return TankGob::InitClass(&gLTankConsts, pini);
}

void LTankGob::ExitClass()
{
	TankGob::ExitClass(&gLTankConsts);
}

LTankGob::LTankGob() : TankGob(&gLTankConsts)
{
}

//
// Medium Tank
//

static MobileUnitConsts gMTankConsts;

#if defined(DEBUG_HELPERS)
char *MTankGob::GetName()
{
	return "MTank";
}
#endif

bool MTankGob::InitClass(IniReader *pini)
{
	gMTankConsts.gt = kgtMediumTank;
	gMTankConsts.ut = kutMediumTank;
	gMTankConsts.upgmPrerequisites = kupgmAdvancedVTS;

	// Initialize the frame indices arrays

	gMTankConsts.anFiringStripIndices = s_anTurretStripIndices;
	gMTankConsts.anMovingStripIndices = s_anBaseStripIndices;
	gMTankConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gMTankConsts.sfxFire = ksfxMediumTankFire;
	gMTankConsts.sfxImpact = ksfxMediumTankImpact;

	gMTankConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gMTankConsts.sfxcSelect = ksfxcMale06Select;
	gMTankConsts.sfxcMove = ksfxcMale06Move;
	gMTankConsts.sfxcAttack = ksfxcMale06Attack;

	return TankGob::InitClass(&gMTankConsts, pini);
}

void MTankGob::ExitClass()
{
	TankGob::ExitClass(&gMTankConsts);
}

// need to load state & save state

MTankGob::MTankGob() : TankGob(&gMTankConsts)
{
	m_pgobSecondShot = NULL;
}

MTankGob::~MTankGob()
{
	// don't be left holding a shot. This does mean
	// that exiting and restarting will lose this shot.

	delete m_pgobSecondShot;
}

// Make sure the second shot doesn't fire after this tank has been
// destroyed.

void MTankGob::Deactivate()
{
	delete m_pgobSecondShot;
	m_pgobSecondShot = NULL;
	TankGob::Deactivate();
}

void MTankGob::LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	CreateTankShotGob(wx, wy, wxTarget, wyTarget, nDamage / 2, gidOwner, gidTarget);	
	
	// set up for the 2nd shot, kcMTankSecondShot updates (frames) later 
	//kcMTankSecondShot is defined in code, anims need to match!

	m_cShotcountdown = kcMTankSecondShot;

	// Add the current update interval count since m_cShotcountdown gets decremented in DefUpdate
	// right after this call

	m_cShotcountdown += m_unvl.GetUpdateCount();

	m_unvl.MinSkip(m_cShotcountdown);

	if (ggobm.IsBelowLimit(knLimitSupport)) {
		m_pgobSecondShot = new TankShotGob();
		Assert(m_pgobSecondShot != NULL, "out of memory!");
		if (m_pgobSecondShot != NULL) {
			// we need to find the 2nd special point for the 2nd shot, modify the endpoint as well

			Point ptSpecial1, ptSpecial2;
			m_aniTurret.GetSpecialPoint(&ptSpecial1, kifrmTankAction);
			m_aniTurret.GetSpecialPoint(&ptSpecial2, kifrmTankAction + kcMTankSecondShot);

			if (!m_pgobSecondShot->Init(TankShotGob::s_panidShot, wx + WcFromPc(ptSpecial2.x - ptSpecial1.x), wy + WcFromPc(ptSpecial2.y-ptSpecial1.y), 
					wxTarget + WcFromPc(ptSpecial2.x - ptSpecial1.x), wyTarget + WcFromPc(ptSpecial2.y-ptSpecial1.y), 
					nDamage / 2, gidOwner, gidTarget, kwcTankShotRate)) {
				delete m_pgobSecondShot;
				m_pgobSecondShot = NULL;
				return;
			}
		}
	}
}

void MTankGob::DefUpdate()
{
	// if we have a 2nd shot pending, countdown to fire and make it so!

	if (m_pgobSecondShot != NULL) {
		m_cShotcountdown -= m_unvl.GetUpdateCount();
		if (m_cShotcountdown < 0) {
			// The unit limit check was made when this gob was created, however now that it is time to fire,
			// the limit may have been reached (since it is Launch that adds the gob). Check again.

			if (ggobm.IsBelowLimit(knLimitSupport)) {
				m_pgobSecondShot->Launch();
			} else {
				delete m_pgobSecondShot;
			}
			m_pgobSecondShot = NULL;
			gsndm.PlaySfx(m_pmuntc->sfxFire);
		}
	}
	TankGob::DefUpdate();
}

//
// Rocket Tank
//

static MobileUnitConsts gRTankConsts;

#if defined(DEBUG_HELPERS)
char *RTankGob::GetName()
{
	return "RTank";
}
#endif

bool RTankGob::InitClass(IniReader *pini)
{
	gRTankConsts.gt = kgtRocketVehicle;
	gRTankConsts.ut = kutRocketVehicle;
	gRTankConsts.upgmPrerequisites = kupgmAdvancedVTS;

	// Initialize the frame indices arrays

	gRTankConsts.anFiringStripIndices = s_anTurretStripIndices;
	gRTankConsts.anMovingStripIndices = s_anBaseStripIndices;
	gRTankConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gRTankConsts.sfxFire = ksfxRocketVehicleFire;
	gRTankConsts.sfxImpact = ksfxRocketVehicleImpact;

	gRTankConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gRTankConsts.sfxcSelect = ksfxcMajor01Select;
	gRTankConsts.sfxcMove = ksfxcMajor01Move;
	gRTankConsts.sfxcAttack = ksfxcMajor01Attack;

	return TankGob::InitClass(&gRTankConsts, pini);
}

void RTankGob::ExitClass()
{
	TankGob::ExitClass(&gRTankConsts);
}

RTankGob::RTankGob() : TankGob(&gRTankConsts)
{
}

void RTankGob::LaunchProjectile(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	CreateRocketGob(wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget);
}


//
// Machine Gun Tank
//

static MobileUnitConsts gGTankConsts;

#if defined(DEBUG_HELPERS)
char *GTankGob::GetName()
{
	return "GTank";
}
#endif

bool GTankGob::InitClass(IniReader *pini)
{
	gGTankConsts.gt = kgtMachineGunVehicle;
	gGTankConsts.ut = kutMachineGunVehicle;

	// Initialize the frame indices arrays

	gGTankConsts.anFiringStripIndices = s_anTurretStripIndices;
	gGTankConsts.anMovingStripIndices = s_anBaseStripIndices;
	gGTankConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gGTankConsts.sfxFire = ksfxMachineGunVehicleFire;
	gGTankConsts.sfxImpact = ksfxNothing;

	gGTankConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gGTankConsts.sfxcSelect = ksfxcMale01Select;
	gGTankConsts.sfxcMove = ksfxcMale01Move;
	gGTankConsts.sfxcAttack = ksfxcMale01Attack;

	return TankGob::InitClass(&gGTankConsts, pini);
}

void GTankGob::ExitClass()
{
	TankGob::ExitClass(&gGTankConsts);
}

GTankGob::GTankGob() : TankGob(&gGTankConsts)
{
}

// UNDONE: MobileUnitGob can handle some of this (e.g., timing)

bool GTankGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// Firing rate is limited by ctFiringRate

	long t = gsim.GetTickCount();
	int ctWait = m_pmuntc->ctFiringRate;
	int ctRemaining = ctWait - (int)(t - m_tLastFire);
	if (ctRemaining > 0) {
		m_unvl.MinSkip((ctRemaining + (kctUpdate / 2)) / kctUpdate - 1);
		return false;
	}

	Direction16 dir16Fire = CalcDir16(wdx, wdy);
	if (m_dir16Turret != dir16Fire)
		return false;

	m_tLastFire = t;

	// Play firing animation

	StartAnimation(&m_aniTurret, m_pmuntc->anFiringStripIndices[dir16Fire], 0, kfAniResetWhenDone);
	m_wfMunt |= kfMuntFiring;
	gsmm.SendDelayedMsg(kmidFireComplete, m_aniTurret.GetRemainingStripTime(), m_gid, m_gid);

	// Fire off those shots!

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptSpecial;
	m_aniTurret.GetSpecialPoint(&ptSpecial, kifrmTankAction);

	CreateBulletGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget) / 3, m_gid, puntTarget->GetId());

	// Two more shots at slight delays

	gsmm.SendDelayedMsg(kmidFire, kctUpdate * 3, m_gid, m_gid);
	gsmm.SendDelayedMsg(kmidFire, kctUpdate * 6, m_gid, m_gid);

	// Play sound

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

int GTankGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidFire)
		UnitGob *puntTarget = (UnitGob *)ggobm.GetGob(m_gidTarget);
		if (puntTarget != NULL) {
			// Fire off the shot!

			WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
			WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;

			WPoint wpt;
			puntTarget->GetCenter(&wpt);
			Point ptSpecial;
			m_aniTurret.GetSpecialPoint(&ptSpecial, kifrmTankAction);
			CreateBulletGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wpt.wx + wdxRnd, wpt.wy + wdyRnd, 
					GetDamageTo(puntTarget) / 3, m_gid, puntTarget->GetId());
		}

#if 0
EndStateMachineInherit(MobileUnitGob)
#else
            return knHandled;
        }
    } else {
        return (int)MobileUnitGob::ProcessStateMachineMessage(st, pmsg);
    }
    return (int)MobileUnitGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

//
// TankShotGob implementation
//

AnimationData *TankShotGob::s_panidShot = NULL;

// this will actually create & fire
TankShotGob *CreateTankShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	TankShotGob *pgob = new TankShotGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(TankShotGob::s_panidShot, wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget, kwcTankShotRate)) {
		delete pgob;
		return NULL;
	}

	pgob->Launch();
	return pgob;
}

bool TankShotGob::InitClass(IniReader *pini)
{
	s_panidShot = LoadAnimationData("tankshot.anir");
	if (s_panidShot == NULL)
		return false;
	return true;
}

void TankShotGob::ExitClass()
{
	delete s_panidShot;
	s_panidShot = NULL;
}

TankShotGob::TankShotGob()
{
	m_ff |= kfGobStateMachine | kfGobLayerSmokeFire;
}

bool TankShotGob::Init(AnimationData *panid, WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget, WCoord wcMoveRate) 
{
	// Units fire from their special point which may be off the edge of the map.
	// We cannot allow Gobs to be off the map so here we bring it on.

	BringInBounds(&wx, &wy);
	BringInBounds(&wxTarget, &wyTarget);

#ifdef DEBUG
	TileMap *ptmap = gsim.GetLevel()->GetTileMap();
	Size sizT;
	ptmap->GetTCoordMapSize(&sizT);
	Assert(wx < WcFromTc(sizT.cx) && wy < WcFromTc(sizT.cy));
	Assert(wxTarget < WcFromTc(sizT.cx) && wyTarget < WcFromTc(sizT.cy));
#endif

	// initialize our state but don't actually fire, save that for Launch

	m_gidOwner = gidOwner;
	m_gidTarget = gidTarget;
	m_nDamage = nDamage;

	m_li.Init(wx, wy, wxTarget, wyTarget, wcMoveRate);

	// LineIterator initializes x,y to the first step-integral point on the line,
	// presuming that the final step should be at the target x,y

	m_wx = m_li.GetWX();
	m_wy = m_li.GetWY();

	m_ani.Init(panid);
	if (m_ani.GetAnimationData()->GetFrameCount(0) == 8)
		StartAnimation(&m_ani, 0, CalcDir(wxTarget - wx, wyTarget - wy), kfAniDone);

	return true;
}

void TankShotGob::Launch()
{
	// HACK: If this animation has 8 frames we assume that's one for each direction

	if (m_ani.GetAnimationData()->GetFrameCount(0) != 8)
		StartAnimation(&m_ani, 0, 0, kfAniLoop);

	// Add the fresh Gob to the GobMgr. GobMgr::AddGob assigns this Gob a gid

	ggobm.AddGob(this);

	// Let the target know when it will be hit. Doing it this way
	// means the hit will arrive in the same number of updates for all
	// players (the number of updates calc'd by the shooter). Depending
	// on screen resolution, the animated travel time may be off by an update

	Message msgT; 
	msgT.mid = kmidHit; 
	msgT.smidSender = m_gidOwner; 
	msgT.smidReceiver = m_gidTarget; 
	msgT.Hit.gidAssailant = m_gidOwner; 
	msgT.Hit.sideAssailant = ggobm.GetGob(m_gidOwner)->GetSide();
	msgT.Hit.nDamage = m_nDamage; 

	// BUGBUG: m_li.GetStepsRemaining is influenced by the firing point which is resolution dependent.
	// Instead, this message's delay should be derived using the distance from the world coordinate
	// centers of the source and target

	gsmm.SendDelayedMsg(&msgT, (m_li.GetStepsRemaining() + 1) * (kcmsUpdate / 10)); 
}

// TankShotGobs don't get loaded

bool TankShotGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	return true;
}

bool TankShotGob::IsSavable()
{
	return false;
}

GobType TankShotGob::GetType()
{
	return kgtTankShot;
}

void TankShotGob::GetClippingBounds(Rect *prc)
{
	// hardcoded that the travel is strip 0 and the impact is strip 1

	if (m_ani.GetStrip() == 0) {
		if (!(gwfPerfOptions & kfPerfShots)) {
			prc->SetEmpty();
			return;
		}
	} else {
		if (!(gwfPerfOptions & kfPerfShotImpacts)) {
			prc->SetEmpty();
			return;
		}
	}

	m_ani.GetBounds(prc);
	prc->Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
}

void TankShotGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	if (nLayer == knLayerSmokeFire) {
		// hardcoded that the travel is strip 0 and the impact is strip 1

		if (m_ani.GetStrip() == 0) {
			if (!(gwfPerfOptions & kfPerfShots))
				return;
		} else {
			if (!(gwfPerfOptions & kfPerfShotImpacts))
				return;
		}
		m_ani.Draw(pbm, PcFromUwc(m_wx) - xViewOrigin, PcFromUwc(m_wy) - yViewOrigin, m_pplr->GetSide());
	}
}

#if defined(DEBUG_HELPERS)
char *TankShotGob::GetName()
{
	return "TankShot";
}
#endif

int TankShotGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnUpdate
		// Advance the shot animation
		// hardcoded that the travel is strip 0 and the impact is strip 1

		if (m_ani.GetStrip() == 0) {
			AdvanceAnimation(&m_ani);
			MarkRedraw();

			// Advance the shot position

			if (m_li.Step()) {
				WCoord wxOld = m_wx;
				WCoord wyOld = m_wy;
				m_wx = m_li.GetWX();
				m_wy = m_li.GetWY();

				// Keep GobMgr in the loop so it can maintain proper depth sorting

				if (m_wx != wxOld || m_wy != wyOld)
					ggobm.MoveGob(this, wxOld, wyOld, m_wx, m_wy);

				// Assumes getting called each update

				m_unvl.MinSkip();

			} else {

				// Play impact sound

				UnitGob *punt = (UnitGob *)ggobm.GetGob(m_gidOwner);
				if (punt != NULL) {
					UnitConsts *puntc = (UnitConsts *)punt->GetConsts();
					gsndm.PlaySfx(puntc->sfxImpact);
				}

				// Start the impact animation

				StartAnimation(&m_ani, 1, 0, 0);
			}

			// Assume valid if we're not drawing shots

			if (!(gwfPerfOptions & kfPerfShots))
				m_ff &= ~kfGobRedraw;

		} else {

			// Advance the impact animation

			if (!AdvanceAnimation(&m_ani)) {

				// Kill this shot

				ggobm.RemoveGob(this);
				delete this;
				return knDeleted;
			}

			// Assume valid if we're not drawing shot impacts

			if (!(gwfPerfOptions & kfPerfShotImpacts))
				m_ff &= ~kfGobRedraw;
		}

EndStateMachine
}

} // namespace wi
