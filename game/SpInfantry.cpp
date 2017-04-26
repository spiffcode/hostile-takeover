#include "ht.h"

namespace wi {

static SpConsts gConsts;

#if defined(DEBUG_HELPERS)
char *SpInfantryGob::GetName()
{
	return "SpInfantry";
}
#endif

static int s_anIdleStripIndices[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
static int s_anMovingStripIndices[16] = { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };

bool SpInfantryGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtTakeoverSpecialist;
	gConsts.ut = kutTakeoverSpecialist;
	gConsts.umPrerequisites = kumResearchCenter;
	gConsts.upgmPrerequisites = kupgmAdvancedHRC;

	// Initialize the frame indices arrays

	gConsts.anFiringStripIndices = s_anIdleStripIndices;
	gConsts.anMovingStripIndices = s_anMovingStripIndices;
	gConsts.anIdleStripIndices = s_anIdleStripIndices;

	// Sound effects

	gConsts.sfxImpact = ksfxNothing;
	gConsts.sfxFire = ksfxNothing;
	gConsts.sfxStructureCaptured = ksfxTakeoverSpecialistStructureCaptured;

	gConsts.sfxcDestroyed = ksfxcInfantryDestroyed;
	gConsts.sfxcSelect = ksfxcMajor01Select;
	gConsts.sfxcMove = ksfxcMajor01Move;
	gConsts.sfxcAttack = ksfxcMajor01Attack;

	return MobileUnitGob::InitClass(&gConsts, pini);
}

void SpInfantryGob::ExitClass()
{
	MobileUnitGob::ExitClass(&gConsts);
}

SpInfantryGob::SpInfantryGob() : MobileUnitGob(&gConsts)
{
}

bool SpInfantryGob::IsValidTarget(Gob *pgobTarget)
{
	// only active, takeoverable structures that don't belong to us already

	if ((pgobTarget->GetFlags() & (kfGobStructure | kfGobActive)) != (kfGobStructure | kfGobActive) 
			|| IsAlly(pgobTarget->GetSide()))
		return false;

	// Takeover returns false if the structure isn't takeoverable

	StructGob *pstru = (StructGob *)pgobTarget;
	return pstru->IsTakeoverable(GetOwner());
}

bool SpInfantryGob::Fire(UnitGob *puntTarget, WCoord wx, WCoord wy, WCoord wdx, WCoord wdy)
{
	// can takeover if within firing range and structure
	// meets the takeover criteria 

	if (!IsValidTarget(puntTarget))
		return false;

	// MobileUnitGob handles rotating towards the target and firing delay

	if (!MobileUnitGob::Fire(puntTarget, wx, wy, wdx, wdy))
		return false;

	StructGob *pstru = (StructGob *)puntTarget;
	pstru->Takeover(m_pplr);
	if (m_pplr == gpplrLocal)
		gsndm.PlaySfx(m_pspc->sfxStructureCaptured);

	// SpI only gets to takeover once, destroy ourselves w/o dying

	Deactivate();
	gsmm.PostMsg(kmidDelete, m_gid, m_gid);

	return false;
}

void SpInfantryGob::Idle()
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

int SpInfantryGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	// This is needed at global scope to handle the kmidDelete sent by SpInfantryGob::Fire

	OnMsg(kmidDelete)
		Delete();
		return knDeleted;

	State(kstDying)
		OnEnter
			Deactivate();
			m_ff ^= kfGobLayerDepthSorted | kfGobLayerSurfaceDecal;

			gsndm.PlaySfx(SfxFromCategory(m_pmuntc->sfxcDestroyed));
			m_ani.Start("die 3", kfAniIgnoreFirstAdvance);
			MarkRedraw();

			// remove corpse after 10 seconds

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

} // namespace wi