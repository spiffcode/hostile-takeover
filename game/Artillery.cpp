#include "ht.h"

namespace wi {

static MobileUnitConsts gConsts;

#if defined(DEBUG_HELPERS)
char *ArtilleryGob::GetName()
{
	return "Artillery";
}
#endif

static int s_anFiringStripIndices[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
static int s_anMovingStripIndices[16] = { 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };

bool ArtilleryGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtArtillery;
	gConsts.ut = kutArtillery;
	gConsts.upgmPrerequisites = kupgmAdvancedVTS;
	gConsts.wf |= kfUntcNotifyEnemyNearby | kfUntcLargeDefog;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anFiringStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anMovingStripIndices;

	// Sound effects

	// 

	gConsts.sfxFire = ksfxArtilleryFire;
	gConsts.sfxImpact = ksfxArtilleryImpact;
	gConsts.sfxcDestroyed = ksfxcVehicleDestroyed;
	gConsts.sfxcSelect = ksfxcMajor01Select;
	gConsts.sfxcMove = ksfxcMajor01Move;
	gConsts.sfxcAttack = ksfxcMajor01Attack;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void ArtilleryGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
}

ArtilleryGob::ArtilleryGob() : MobileUnitGob(&gConsts)
{
}

bool ArtilleryGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// UNDONE: artillery should rotate slower than most units
	// MobileUnitGob handles rotating towards the target, firing delay, 
	// and starting the fire animation

	if (!MobileUnitGob::Fire(puntTarget, wx, wy, wdx, wdy))
		return false;

	// Fire off the shot!

	WCoord wdxRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	WCoord wdyRnd = ((GetRandom() & 7) - 3) * kwcTile16th;
	Point ptSpecial;
	m_ani.GetSpecialPoint(&ptSpecial, 1);	// frame #1 (second frame) has the firing point

	CreateArtilleryShotGob(m_wx + WcFromPc(ptSpecial.x), m_wy + WcFromPc(ptSpecial.y), wx + wdxRnd, wy + wdyRnd, 
			GetDamageTo(puntTarget), m_gid, puntTarget->GetId());

	// Play sound

	gsndm.PlaySfx(m_pmuntc->sfxFire);

	return true;
}

#if 0
int ArtilleryGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine

EndStateMachineInherit(MobileUnitGob)
}
#endif

//
// ArtilleryShotGob implementation
//

AnimationData *ArtilleryShotGob::s_panidShot = NULL;

// this will actually create & fire
ArtilleryShotGob *CreateArtilleryShotGob(WCoord wx, WCoord wy, WCoord wxTarget, WCoord wyTarget, int nDamage, Gid gidOwner, Gid gidTarget)
{
	if (!ggobm.IsBelowLimit(knLimitSupport))
		return NULL;

	ArtilleryShotGob *pgob = new ArtilleryShotGob();
	Assert(pgob != NULL, "out of memory!");
	if (pgob == NULL)
		return NULL;

	if (!pgob->Init(ArtilleryShotGob::s_panidShot, wx, wy, wxTarget, wyTarget, nDamage, gidOwner, gidTarget, kwcTile / 2)) {
		delete pgob;
		return NULL;
	}

	pgob->Launch();
	return pgob;
}

bool ArtilleryShotGob::InitClass(IniReader *pini)
{
	s_panidShot = LoadAnimationData("artilleryshot.anir");
	if (s_panidShot == NULL)
		return false;
	return true;
}

void ArtilleryShotGob::ExitClass()
{
	delete s_panidShot;
	s_panidShot = NULL;
}

GobType ArtilleryShotGob::GetType()
{
	return kgtArtilleryShot;
}

#if defined(DEBUG_HELPERS)
char *ArtilleryShotGob::GetName()
{
	return "ArtilleryShot";
}
#endif

} // namespace wi