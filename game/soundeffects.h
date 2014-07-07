// This file is read by schemer. This file can be editted as desired. Edits will require rebuilds of data.
// Change disabled to enabled when sound effect is in use in the game.
// Note: name deprecation is supported. Replace "none" with old names for loading old files

enum Sfx { // sfx
	ksfxNothing, // disabled; none
	ksfxGalaxiteProcessorAbortRepair, // enabled; none
	ksfxGalaxiteProcessorDamaged, // enabled; none
	ksfxGalaxiteProcessorDestroyed, // enabled; none
	ksfxGalaxiteProcessorDoorOpening, // enabled; none
	ksfxGalaxiteProcessorDoorClosing, // enabled; none
	ksfxGalaxiteProcessorRepair, // enabled; none
	ksfxGalaxiteProcessorSelect, // enabled; none
	ksfxGalaxiteWarehouseAbortRepair, // enabled; none
	ksfxGalaxiteWarehouseDamaged, // enabled; none
	ksfxGalaxiteWarehouseDestroyed, // enabled; none
	ksfxGalaxiteWarehouseRepair, // enabled; none
	ksfxGalaxiteWarehouseSelect, // enabled; none
	ksfxGalaxiteWarehouseTooFull, // enabled; none
	ksfxGalaxMinerMine, // enabled; none
	ksfxGalaxMinerUnderAttack, // enabled; none
	ksfxGalaxMinerDeliver, // enabled; none
	ksfxGameBaseUnderAttack, // enabled; none
	ksfxGameCreditsDecreasing, // enabled; none
	ksfxGameCreditsIncreasing, // enabled; none
	ksfxGameLevelResults, // disabled; none
	ksfxGameLoseLevel, // enabled; none
	ksfxGameNewVehicleOptions, // enabled; none
	ksfxGameNewRecruitOptions, // enabled; none
	ksfxGameNewStructureOptions, // enabled; none
	ksfxGameWinLevel, // enabled; none
	ksfxGuiBuildMenuHide, // disabled; none
	ksfxGuiBuildMenuSelectItem, // disabled; none
	ksfxGuiBuildMenuShow, // disabled; none
	ksfxGuiButtonTap, // enabled; none
	ksfxGuiCheckBoxTap, // enabled; none
	ksfxGuiEditBoxCharacterEntered, // disabled; none
	ksfxGuiFormHide, // enabled; none
	ksfxGuiFormShow, // enabled; none
	ksfxGuiMissionTextCharOutput, // disabled; none
	ksfxGuiScrollingListSelectItem, // enabled; none
	ksfxHeadquartersAbortConstruction, // enabled; none
	ksfxHeadquartersAbortRepair, // enabled; none
	ksfxHeadquartersConstruct, // enabled; none
	ksfxHeadquartersDamaged, // enabled; none
	ksfxHeadquartersDestroyed, // enabled; none
	ksfxHeadquartersRepair, // enabled; none
	ksfxHeadquartersSelect, // enabled; none
	ksfxHeadquartersStructureReady, // enabled; none
	ksfxHumanResourceCenterAbortRecruiting, // enabled; none
	ksfxHumanResourceCenterAbortRepair, // enabled; none
	ksfxHumanResourceCenterDamaged, // enabled; none
	ksfxHumanResourceCenterDestroyed, // enabled; none
	ksfxHumanResourceCenterRecruit, // enabled; none
	ksfxHumanResourceCenterRepair, // enabled; none
	ksfxHumanResourceCenterSelect, // enabled; none
	ksfxHumanResourceCenterUnitReady, // enabled; none
	ksfxLightTankFire, // enabled; none
	ksfxLightTankImpact, // enabled; none
	ksfxMachineGunInfantryFire, // enabled; none
	ksfxMachineGunTowerAbortRepair, // enabled; none
	ksfxMachineGunTowerAttack, // disabled; none
	ksfxMachineGunTowerDamaged, // enabled; none
	ksfxMachineGunTowerDestroyed, // enabled; none
	ksfxMachineGunTowerFire, // enabled; none
	ksfxMachineGunTowerRepair, // enabled; none
	ksfxMachineGunTowerSelect, // enabled; none
	ksfxMachineGunVehicleFire, // enabled; none
	ksfxMediumTankFire, // enabled; none
	ksfxMediumTankImpact, // enabled; none
	ksfxMobileHeadquartersDeploy, // enabled; none
	ksfxRadarAbortRepair, // enabled; none
	ksfxRadarDamaged, // enabled; none
	ksfxRadarDestroyed, // enabled; none
	ksfxRadarRepair, // enabled; none
	ksfxRadarSelect, // enabled; none
	ksfxReactorAbortRepair, // enabled; none
	ksfxReactorDamaged, // enabled; none
	ksfxReactorDestroyed, // enabled; none
	ksfxReactorPowerTooLow, // enabled; none
	ksfxReactorRepair, // enabled; none
	ksfxReactorSelect, // enabled; none
	ksfxResearchCenterAbortRepair, // enabled; none
	ksfxResearchCenterDamaged, // enabled; none
	ksfxResearchCenterDestroyed, // enabled; none
	ksfxResearchCenterRepair, // enabled; none
	ksfxResearchCenterSelect, // enabled; none
	ksfxRocketInfantryFire, // enabled; none
	ksfxRocketInfantryImpact, // enabled; none
	ksfxRocketTowerAbortRepair, // enabled; none
	ksfxRocketTowerAttack, // disabled; none
	ksfxRocketTowerDamaged, // enabled; none
	ksfxRocketTowerDestroyed, // enabled; none
	ksfxRocketTowerFire, // enabled; none
	ksfxRocketTowerImpact, // enabled; none
	ksfxRocketTowerRepair, // enabled; none
	ksfxRocketTowerSelect, // enabled; none
	ksfxRocketVehicleFire, // enabled; none
	ksfxRocketVehicleImpact, // enabled; none
	ksfxTakeoverSpecialistStructureCaptured, // enabled; none
	ksfxVehicleTransportStationAbortManufacture, // enabled; none
	ksfxVehicleTransportStationAbortRepair, // enabled; none
	ksfxVehicleTransportStationDamaged, // enabled; none
	ksfxVehicleTransportStationDestroyed, // enabled; none
	ksfxVehicleTransportStationManufacture, // enabled; none
	ksfxVehicleTransportStationRepair, // enabled; none
	ksfxVehicleTransportStationSelect, // enabled; none
	ksfxVehicleTransportStationVehicleReady, // enabled; none
	ksfxVehicleDestroyed, // enabled; none
	ksfxInfantryDestroyed0, // enabled; none
	ksfxInfantryDestroyed1, // enabled; none
	ksfxInfantryDestroyed2, // enabled; none
	ksfxInfantryDestroyed3, // enabled; none
	ksfxInfantryDestroyed4, // enabled; none
	ksfxMale01Select0, // enabled; none
	ksfxMale01Select1, // enabled; none
	ksfxMale01Select2, // enabled; none
	ksfxMale01Select3, // enabled; none
	ksfxMale01Move0, // enabled; none
	ksfxMale01Move1, // enabled; none
	ksfxMale01Move2, // enabled; none
	ksfxMale01Move3, // enabled; none
	ksfxMale01Attack0, // enabled; none
	ksfxMale01Attack1, // enabled; none
	ksfxMale01Attack2, // enabled; none
	ksfxMale01Attack3, // enabled; none
	ksfxMale03Select0, // enabled; none
	ksfxMale03Select1, // enabled; none
	ksfxMale03Select2, // enabled; none
	ksfxMale03Select3, // enabled; none
	ksfxMale03Move0, // enabled; none
	ksfxMale03Move1, // enabled; none
	ksfxMale03Move2, // enabled; none
	ksfxMale03Move3, // enabled; none
	ksfxMale03Attack0, // enabled; none
	ksfxMale03Attack1, // enabled; none
	ksfxMale03Attack2, // enabled; none
	ksfxMale03Attack3, // enabled; none
	ksfxMale06Select0, // enabled; none
	ksfxMale06Select1, // enabled; none
	ksfxMale06Select2, // enabled; none
	ksfxMale06Select3, // enabled; none
	ksfxMale06Move0, // enabled; none
	ksfxMale06Move1, // enabled; none
	ksfxMale06Move2, // enabled; none
	ksfxMale06Move3, // enabled; none
	ksfxMale06Attack0, // enabled; none
	ksfxMale06Attack1, // enabled; none
	ksfxMale06Attack2, // enabled; none
	ksfxMale06Attack3, // enabled; none
	ksfxMajor01Select0, // enabled; none
	ksfxMajor01Select1, // enabled; none
	ksfxMajor01Select2, // enabled; none
	ksfxMajor01Select3, // enabled; none
	ksfxMajor01Move0, // enabled; none
	ksfxMajor01Move1, // enabled; none
	ksfxMajor01Move2, // enabled; none
	ksfxMajor01Move3, // enabled; none
	ksfxMajor01Attack0, // enabled; none
	ksfxMajor01Attack1, // enabled; none
	ksfxMajor01Attack2, // enabled; none
	ksfxMajor01Attack3, // enabled; none
	ksfxMajor02Select0, // enabled; none
	ksfxMajor02Select1, // enabled; none
	ksfxMajor02Select2, // enabled; none
	ksfxMajor02Select3, // enabled; none
	ksfxMajor02Move0, // enabled; none
	ksfxMajor02Move1, // enabled; none
	ksfxMajor02Move2, // enabled; none
	ksfxMajor02Move3, // enabled; none
	ksfxMajor02Attack0, // enabled; none
	ksfxMajor02Attack1, // enabled; none
	ksfxMajor02Attack2, // enabled; none
	ksfxMajor02Attack3, // enabled; none
	ksfxAndySelect, // enabled; none
	ksfxAndyMove, // enabled; none
	ksfxAndyAttack, // enabled; none
	ksfxAndyFire, //enabled; none
	ksfxAndyDestroyed, // enabled; none
	ksfxReplicatorOn, // enabled; none
	ksfxReplicatorOff, // enabled; none
	ksfxReplicatorBuild, // enabled; none
	ksfxActivatorOn, // enabled; none
	ksfxActivatorOff, // enabled; none
	ksfxArtilleryFire, // enabled; none
	ksfxArtilleryImpact, // enabled; none
	ksfxHappyEnding, //enabled; none
	ksfxReplicatorDestroyed, //enabled; none
	ksfxFoxDestroyed, //endabled; none
};
// **sfx for triggers - keep grouped after HappyEnding, also add in res.h
// stop
// needed sfx
//	ksfxResearchCenterUpgrade
//  ksfxResearchCenterAbortUpgrade

enum SfxCategory {
	ksfxcNothing = -1,
	ksfxcInfantryDestroyed,
	ksfxcVehicleDestroyed,
	ksfxcMale01Select,
	ksfxcMale01Move,
	ksfxcMale01Attack,
	ksfxcMale03Select,
	ksfxcMale03Move,
	ksfxcMale03Attack,
	ksfxcMale06Select,
	ksfxcMale06Move,
	ksfxcMale06Attack,
	ksfxcMajor01Select,
	ksfxcMajor01Move,
	ksfxcMajor01Attack,
	ksfxcMajor02Select,
	ksfxcMajor02Move,
	ksfxcMajor02Attack,
	ksfxcAndySelect,
	ksfxcAndyMove,
	ksfxcAndyAttack,
	ksfxcAndyDestroyed,
	ksfxcFoxDestroyed,
};
