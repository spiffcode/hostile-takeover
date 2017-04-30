#include "ht.h"

namespace wi {

static MobileUnitConsts gConsts;

#if defined(DEBUG_HELPERS)
char *SRInfantryGob::GetName()
{
	return "SRInfantry";
}
#endif

static int s_anFiringStripIndices[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
static int s_anMovingStripIndices[16] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 };
static int s_anIdleStripIndices[16] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool SRInfantryGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtShortRangeInfantry;
	gConsts.ut = kutShortRangeInfantry;
	gConsts.wf |= kfUntcNotifyEnemyNearby;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anFiringStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gConsts.sfxFire = ksfxMachineGunInfantryFire;
	gConsts.sfxImpact = ksfxNothing;

	gConsts.sfxcDestroyed = ksfxcInfantryDestroyed;
	gConsts.sfxcSelect = ksfxcMale03Select;
	gConsts.sfxcMove = ksfxcMale03Move;
	gConsts.sfxcAttack = ksfxcMale03Attack;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void SRInfantryGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
}

SRInfantryGob::SRInfantryGob() : MobileUnitGob(&gConsts)
{
}

bool SRInfantryGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// MobileUnitGob handles rotating towards the target, firing delay, 
	// and starting the fire animation

	if (!MobileUnitGob::Fire(puntTarget, wx, wy, wdx, wdy))
		return false;

	// Fire off the shot!

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptSpecial;
	m_ani.GetSpecialPoint(&ptSpecial, 1);	// frame #1 (second frame) has the firing point

	CreateBulletGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sound

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

void SRInfantryGob::Idle()
{
	// 1/4 of the time we pivot left, 1/4 we pivot right, and 1/2 we play the idle

	switch (GetRandom() & 3) {
	case 0:
		m_dir--;
		if (m_dir < 0)
			m_dir = 15;
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;

	case 1:
		m_dir++;
		if (m_dir > 15)
			m_dir = 0;
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;

	default:
		StartAnimation(&m_ani, m_pmuntc->anIdleStripIndices[m_dir], 0, kfAniResetWhenDone);
		break;
	}
}

int SRInfantryGob::GetIdleCountdown()
{
    return (GetRandom() % 100) + 50; // somewhere between 8 & 12 seconds
}

int SRInfantryGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	State(kstDying)
		OnEnter
			TRect trc;
			GetTileRect(&trc);

			Deactivate();

			// Redraw this part of minimap. It will skip inactive munts

			gpmm->RedrawTRect(&trc);

			m_ff ^= kfGobLayerDepthSorted | kfGobLayerSurfaceDecal;

			gsndm.PlaySfx(SfxFromCategory(m_pmuntc->sfxcDestroyed));
			m_ani.Start("die 3", kfAniIgnoreFirstAdvance);
			MarkRedraw();

			// Remove corpse after 10 seconds

			gsmm.SendDelayedMsg(kmidDelete, 1000, m_gid, m_gid);

		OnUpdate
			AdvanceAnimation(&m_ani);

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

#if 0 // Not used anymore
//
// RicochetGob implementation
//

bool RicochetGob::Init(WCoord wx, WCoord wy, word wfAnm, const char *pszAniName, AnimationData *panid, 
		StateMachineId smidNotify, const char *pszName)
{
	if (!AnimGob::Init(wx, wy, wfAnm, pszAniName, panid, GetRandom() % 3, smidNotify, pszName))
		return false;

	m_fNew = true;
	return true;
}

bool RicochetGob::OnStripDone()
{
	if (m_fNew) {
		m_fNew = false;
		SetAnimationStrip(&m_ani, GetRandom() % 3);
		return false;
	} else {
		return true;
	}
}
#endif

//
// BulletGob implementation
//

AnimationData *BulletGob::s_panidShot = NULL;

// this will actually create & fire
BulletGob *CreateBulletGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget, bool fEagle)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	BulletGob *pgob = new BulletGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(BulletGob::s_panidShot, wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget, kwcTile)) {
		delete pgob;
		return NULL;
	}

	pgob->Launch();
	return pgob;
}

bool BulletGob::InitClass(IniReader *pini)
{
	s_panidShot = LoadAnimationData("bullet.anir");
	if (s_panidShot == NULL)
		return false;
	return true;
}

void BulletGob::ExitClass()
{
	delete s_panidShot;
	s_panidShot = NULL;
}

GobType BulletGob::GetType()
{
	return kgtBullet;
}

#if defined(DEBUG_HELPERS)
char *BulletGob::GetName()
{
	return "Bullet";
}
#endif

} // namespace wi