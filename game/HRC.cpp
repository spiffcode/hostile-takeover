#include "ht.h"

namespace wi {

//
// HrcGob implementation
//

static MobileUnitBuilderConsts gConsts;
MobileUnitBuildForm *HrcGob::s_pfrmBuild = NULL;

#if defined(DEBUG_HELPERS)
char *HrcGob::GetName()
{
	return "HRC";
}
#endif

bool HrcGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtHumanResourceCenter;
	gConsts.ut = kutHumanResourceCenter;
	gConsts.umPrerequisites = kumReactor;
	gConsts.umCanBuild = kumInfantry & ~(kumAndy | kumFox);
	gConsts.wf |= kfUntcMobileUnitBuilder;

	// Preload the Hrc's build form

	s_pfrmBuild = new MobileUnitBuildForm();
	if (s_pfrmBuild == NULL)
		return false;

	if (!s_pfrmBuild->Init(gpmfrmm, gpiniForms, kidfBuildInfantry))
		return false;
    gpmfrmm->RemoveForm(s_pfrmBuild);

	// Sound effects

	gConsts.sfxUnitBuildAbort = ksfxHumanResourceCenterAbortRecruiting;
	gConsts.sfxUnitBuild = ksfxHumanResourceCenterRecruit;
	gConsts.sfxUnitReady = ksfxHumanResourceCenterUnitReady;
	gConsts.sfxAbortRepair = ksfxHumanResourceCenterAbortRepair;
	gConsts.sfxDamaged = ksfxHumanResourceCenterDamaged;
	gConsts.sfxDestroyed = ksfxHumanResourceCenterDestroyed;
	gConsts.sfxRepair = ksfxHumanResourceCenterRepair;
	gConsts.sfxSelect = ksfxHumanResourceCenterSelect;

	// MobileUnitBuilderConsts

	gConsts.fUpgrade = kfUpgradeHrc;
	gConsts.fUpgradeInProgress = kfUpgradeHrcInProgress;
	gConsts.pfrmBuild = s_pfrmBuild;

	return BuilderGob::InitClass(&gConsts, pini);
}

void HrcGob::ExitClass()
{
	BuilderGob::ExitClass(&gConsts);
	delete s_pfrmBuild;
	s_pfrmBuild = NULL;
}

HrcGob::HrcGob() : MobileUnitBuilderGob(&gConsts)
{
}

} // namespace wi