#include "ht.h"

namespace wi {

static MobileUnitConsts gmuntcAndy;

#if defined(DEBUG_HELPERS)
char *AndyGob::GetName()
{
	return "Andy";
}
#endif

static int s_anFiringStripIndices[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
static int s_anMovingStripIndices[16] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48 };
static int s_anIdleStripIndices[16] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool AndyGob::InitClass(IniReader *pini)
{
	gmuntcAndy.gt = kgtAndy;
	gmuntcAndy.ut = kutAndy;
	gmuntcAndy.wf |= kfUntcNotifyEnemyNearby | kfUntcLargeDefog;

	// Initialize the frame indices arrays

	gmuntcAndy.anFiringStripIndices = s_anFiringStripIndices;
	gmuntcAndy.anMovingStripIndices = s_anMovingStripIndices;
	gmuntcAndy.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gmuntcAndy.sfxFire = ksfxAndyFire;
	gmuntcAndy.sfxImpact = ksfxNothing;
	gmuntcAndy.sfxcDestroyed = ksfxcAndyDestroyed;
	gmuntcAndy.sfxcSelect = ksfxcAndySelect;
	gmuntcAndy.sfxcMove = ksfxcAndyMove;
	gmuntcAndy.sfxcAttack = ksfxcAndyAttack;

	return MobileUnitGob::InitClass(&gmuntcAndy, pini);
}

void AndyGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gmuntcAndy);
}

AndyGob::AndyGob() : MobileUnitGob(&gmuntcAndy)
{
}

// For the benefit of Fox

AndyGob::AndyGob(MobileUnitConsts *pmuntc) : MobileUnitGob(pmuntc)
{
}

bool AndyGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	bool f = MobileUnitGob::Init(wx, wy, pplr, fxHealth, ff, pszName);

	// Start the healing process

	gsmm.SendDelayedMsg(kmidHeal, 100, m_gid, m_gid);	// 1 second

	return f;
}

bool AndyGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
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

	CreateAndyShotGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sound

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

void AndyGob::Idle()
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

int AndyGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine

	// Andy recovers full health in 1 minute

	OnMsg(kmidHeal)
		if (m_ff & kfGobActive)
			gsmm.SendDelayedMsg(kmidHeal, 100, m_gid, m_gid);	// 1 second

		fix fxArmorStrength = m_pmuntc->GetArmorStrength();
		if (m_fxHealth < fxArmorStrength) {
			// Add 1/60th of total health
			fix fxHealth = addfx(m_fxHealth, divfx(fxArmorStrength, itofx(60)));
			if (fxHealth > fxArmorStrength)
				fxHealth = fxArmorStrength;
			SetHealth(fxHealth);
		}
		
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


//
// Fox is a clone of Andy with tweaked GobTemplates.ini parameters
//

static MobileUnitConsts gmuntcFox;

#if defined(DEBUG_HELPERS)
char *FoxGob::GetName()
{
	return "Fox";
}
#endif

FoxGob::FoxGob() : AndyGob(&gmuntcFox)
{
}

bool FoxGob::InitClass(IniReader *pini)
{
	gmuntcFox.gt = kgtFox;
	gmuntcFox.ut = kutFox;
	gmuntcFox.wf |= kfUntcNotifyEnemyNearby | kfUntcLargeDefog;

	// Initialize the frame indices arrays

	gmuntcFox.anFiringStripIndices = s_anFiringStripIndices;
	gmuntcFox.anMovingStripIndices = s_anMovingStripIndices;
	gmuntcFox.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects


	//gmuntcFox.sfxFire = ksfxAndyFire; he never shoots

	gmuntcFox.sfxFire = ksfxLightTankFire;
	gmuntcFox.sfxImpact = ksfxNothing;

	gmuntcFox.sfxcDestroyed = ksfxcFoxDestroyed;
	gmuntcFox.sfxcSelect = ksfxcMale03Select;
	gmuntcFox.sfxcMove = ksfxcMale03Move;
	gmuntcFox.sfxcAttack = ksfxcMale03Attack;

	return MobileUnitGob::InitClass(&gmuntcFox, pini);
}

void FoxGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gmuntcFox);
}

//
// AndyShotGob implementation
//

AnimationData *AndyShotGob::s_panidShot = NULL;

// this will actually create & fire
AndyShotGob *CreateAndyShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget, bool fEagle)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	AndyShotGob *pgob = new AndyShotGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(AndyShotGob::s_panidShot, wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget, kwcTile)) {
		delete pgob;
		return NULL;
	}

	pgob->Launch();
	return pgob;
}

bool AndyShotGob::InitClass(IniReader *pini)
{
	s_panidShot = LoadAnimationData("andyshot.anir");
	if (s_panidShot == NULL)
		return false;
	return true;
}

void AndyShotGob::ExitClass()
{
	delete s_panidShot;
	s_panidShot = NULL;
}

GobType AndyShotGob::GetType()
{
	return kgtAndyShot;
}

#if defined(DEBUG_HELPERS)
char *AndyShotGob::GetName()
{
	return "AndyShot";
}
#endif

} // namespace wi