#include "ht.h"

namespace wi {

//
// VtsGob implementation
//

static MobileUnitBuilderConsts gConsts;
MobileUnitBuildForm *VtsGob::s_pfrmBuild = NULL;


//
// Gob methods
//

bool VtsGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtVehicleTransportStation;
	gConsts.ut = kutVehicleTransportStation;
	gConsts.umPrerequisites = kumReactor;
	gConsts.umCanBuild = kumVehicles;
	gConsts.wf |= kfUntcMobileUnitBuilder;

	// Preload the Vts's Build form

	s_pfrmBuild = new MobileUnitBuildForm();
	if (s_pfrmBuild == NULL)
		return false;

	if (!s_pfrmBuild->Init(gpmfrmm, gpiniForms, kidfBuildVehicle))
		return false;
	gpmfrmm->RemoveForm(s_pfrmBuild);

	// Sound effects

	gConsts.sfxUnitBuildAbort = ksfxVehicleTransportStationAbortManufacture;
	gConsts.sfxUnitBuild = ksfxVehicleTransportStationManufacture;
	gConsts.sfxUnitReady = ksfxVehicleTransportStationVehicleReady;
	gConsts.sfxAbortRepair = ksfxVehicleTransportStationAbortRepair;
	gConsts.sfxDamaged = ksfxVehicleTransportStationDamaged;
	gConsts.sfxDestroyed = ksfxVehicleTransportStationDestroyed;
	gConsts.sfxRepair = ksfxVehicleTransportStationRepair;
	gConsts.sfxSelect = ksfxVehicleTransportStationSelect;

	// MobileUnitBuilderConsts

	gConsts.fUpgrade = kfUpgradeVts;
	gConsts.fUpgradeInProgress = kfUpgradeVtsInProgress;
	gConsts.pfrmBuild = s_pfrmBuild;

	return BuilderGob::InitClass(&gConsts, pini);
}

void VtsGob::ExitClass()
{
	BuilderGob::ExitClass(&gConsts);
	delete s_pfrmBuild;
	s_pfrmBuild = NULL;
}

VtsGob::VtsGob() : MobileUnitBuilderGob(&gConsts)
{
}

} // namespace wi