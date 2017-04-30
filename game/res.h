#define kidfDefault 1000
#define kidfAreYouSure 1001
#define kidfEmpty 1002
#define kidfLoseSummary 1003
#define kidfGameOptions 1004
#define kidfTileMapTest 1005
#define kidfContinueGame 1006
#define kidfHrcMenu 1008
#define kidfPickLevel 1009
#define kidfWinSummary 1010
#define kidfBuildInfantry 1011
#define kidfResearchMenu 1012
#define kidfStructMenu 1013
#define kidfVtsMenu 1014
#define kidfBuildVehicle 1015
#define kidfHqMenu 1016
#define kidfBuildStructure 1017
#define kidfPlaceStructure 1018
#define kidfTestOptions 1019
#define kidfStartup 1021
#define kidfPickTransport 1022
#define kidfHostMultiplayer 1024
#define kidfObjectives 1025
#define kidfGameStart 1026
#define kidfChangeDisplayMode 1028
#define kidfMessageBox 1029
#define kidfUnitMenu 1030
#define kidfMobileHqMenu 1031
#define kidfMinerMenu 1032
#define kidfMemoryUse 1033
#define kidfRoom 1034
#define kidfCreateGame 1035
#define kidfSimUI 1036
#define kidfInputUI 1037
#define kidfLoadGame 1038
#define kidfSaveGame 1039
#define kidfEcomLarge 1040
#define kidfEcomSmall 1041
#define kidfHelp 1042
#define kidfPlay 1043
#define kidfInGameMenu 1044
#define kidfUpgrade 1045
#define kidfMiniMap 1046
#define kidfCutScene 1047
#define kidfInGameOptions 1049
#define kidfSoundOptions 1050
#define kidfColorOptions 1051
#define kidfDisplayOptions 1052
#define kidfPerformanceOptions 1053
#define kidfGobCount 1054
#define kidfDrmCode 1055
#define kidfDrmKey 1056
#define kidfRegisterNow 1057
#define kidfMessageBoxQuery 1058
#define kidfInputPanel 1059
#define kidfDeleteMissionPack 1060
#define kidfLoading 1061
#define kidfWaiting 1062
#define kidfMultiplayerLoseSummary 1063
#define kidfMultiplayerWinSummary 1064
#define kidfMultiplayerObjectives 1065
#define kidfBluetoothPatchQuery 1066
#define kidfRoomWide 1067
#define kidfCreateGameWide 1068
#define kidfSelectMissionWide 1069
#define kidfDownloadMissionPackWide 1070
#define kidfDownloadBox 1071
#define kidfHelpWide 1072
#define kidfLobby 1073
#define kidfLogin 1074
#define kidfCreateRoom 1075
#define kidfChooseServer 1076
#define kidfAddOnSingleMulti 1077

#define kifntDefault 0
#define kifntShadow 1
#define kifntButton 2
#define kifntTitle 3
#define kifntEcom 4
#define kifntHud 5
#define kcFonts 6

#define kidcOk 2000
#define kidcCancel 2001
#define kidcRunTests 2002
#define kidcMiniMap 2003
#define kidcHelp 2004
#define kidcDefault 2005
#define kidcAlert 2006

// Gob types

// WARNING: the value and order of these constants cannot be
// changed without updating 'M'

#define kgtNone 0
#define kgtShortRangeInfantry 1
#define kgtLongRangeInfantry 2
#define kgtHumanResourceCenter 3
#define kgtSurfaceDecal 4
#define kgtScenery 5
#define kgtAnimation 6
#define kgtReactor 7
#define kgtProcessor 8
#define kgtStructure 9
#define kgtUnit 10
#define kgtGalaxMiner 11
#define kgtHeadquarters 12
#define kgtResearchCenter 13
#define kgtVehicleTransportStation 14
#define kgtRadar 15
#define kgtLightTank 16
#define kgtMediumTank 17
#define kgtMachineGunVehicle 18
#define kgtRocketVehicle 19
#define kgtTakeoverSpecialist 20
#define kgtWarehouse 21
#define kgtMobileHeadquarters 22
#define kgtOvermind 23
#define kgtTankShot 24
#define kgtRocket 25
#define kgtMachineGunTower 26
#define kgtRocketTower 27
#define kgtScorch 28
#define kgtSmoke 29
#define kgtPuff 30
#define kgtBullet 31
#define kgtArtillery 32
#define kgtArtilleryShot 33
#define kgtAndy 34
#define kgtReplicator 35
#define kgtActivator 36
#define kgtFox 37
#define kgtAndyShot 38

#include "../mpshared/side.h"

// Startup form constants

#define kidcPlaySinglePlayer 1001
#define kidcPlayMultiPlayer 1002
#define kidcPlay 1003
#define kidcSetupGame 1004
#define kidcBuyMe 1005
#define kidcCredits 1006
#define kidcExitGame 1007
#define kidcLoadSavedGame 1008
#define kidcPlayDemo 1500
#define kidcPlayMission 1010
#define kidcVersion 1011
#define kidcDownloadMissions 1501
#define kidcForums 1502
#define kidcLeaderboard 1503

// Play Solo form constants

#define kidcBeginNewGame 1012
#define kidcPlayChallengeLevel 1013
#define kidcPlayStoryMission 1014
#define kidcPlaybackGame 1015
//#define kidcLoadSavedGame 1008

// SimUIForm constants

#define kidcStatusLabel 1001
#define kidcFps 1004
#define kidcObjective 1005
#define kidcCountdown 1006

// InputUIForm constants

#define kidcMenuButton 1002
#define kidcGraffitiScroll 1012
#define kidcAppsSilkButton 1013
#define kidcMenuSilkButton 1014
#define kidcCalcSilkButton 1015
#define kidcFindSilkButton 1016
#define kidcCreditsLabel 1018
#define kidcPower 1019

// InGameMenuForm constants

#define kidcSaveGame 1001
#define kidcLoadGame 1002
#define kidcOptions 1003
#define kidcRestartMission 1004
#define kidcAbortMission 1005
#define kidcObjectives 1006
//#define kidcHelp 1006

// TestOptionsForm constants

#define kidcDrawPaths 1002
#define kidcOvermind 1003
#define kidcClearFog 1004
#define kidcDrawLines 1005
#define kidcShowStats 1006
#define kidcShowFPS 1007
#define kidcSuspendUpdates 1008
#define kidcMaxRepaint 1009
#define kidcAutosave 1010
#define kidcMemoryUse 1011
#define kidcLockStep 1012
#define kidcDrawUpdateRects 1013
//#define kidcGraphStats 1014
#define kidcGodMode 1015
//#define kidcFlashEnemyTargets 1016
//#define kidcHelp 1017
#define kidcNextPage 1018
#define kidcPrevPage 1019
#define kidcIndex 1020
#define kidcBack 1021
#define kidcGobCount 1022
#define kidcBreak 1023
#define kidcStylusUI 1024
#define kidcMoveIndicator 1025
#define kidcHoldSelect 1026

// MemoryUseForm constants

#define kidcDynDbInitial 1001
#define kidcMmgrDynDbReserve 1002
#define kidcDynUse 1003
#define kidcMmgrUse 1004
#define kidcCacheUse 1005
#define kidcClearCache 1006
#define kidcLimitCache 1007
#define kidcAdd10KCache 1008
#define kidcSub10KCache 1009
#define kidcCacheLimit 1010

// Game Over form constants

#define kidcMessage 1001

// Login form constants

#define kidcAnonymous 1200
#define kidcLogin 1201
#define kidcRegister 1202
#define kidcUpdateAccount 1203

// Lobby form constants

#define kidcRoomList 1200
#define kidcJoinRoom 1201
#define kidcNewRoom 1202
#define kidcSignOut 1203
#define kidcLurkerCount 1204

// CreateRoom constants

#define kidcRoomNameLabel 1200
#define kidcRoomName 1201
#define kidcRoomNamePanel 1202
#define kidcPrivate 1203

// PlayMultiplayer form constants

#define kidcJoinGame 1001
#define kidcNewGame 1002
#define kidcGameList 1003
#define kidcChat 1904
#define kidcPlayerName 1005
#define kidcSearching 1006
#define kidcPlayerNamePanel 1007
#define kidcPlayerNameLabel 1008
#define kidcStatus 1009
#define kidcSpecify 1010
#define kidcNoGames 1011

// CreateNewMultiplayer form constants

#define kidcPassword 1101
#define kidcMapList 1102

// MeetingForm constants

#define kidcGameName 1001
#define kidcMapName 1002
#define kidcNumPlayers 1003
#define kidcPlayerList 1004
#define kidcGameNameLabel 1005
#define kidcGameNamePanel 1006
#define kidcPasswordLabel 1107
#define kidcPasswordPanel 1108
#define kidcAddress 1009
#define kidcAddressLabel 1010

// PickLevelForm constants

#define kidcLevelList 1001
#define kidcCategories 1500
#define kidcAddOnMessage 1501
#define kidcStoryList 1502
#define kidcChallengeList 1503
#define kidcAddOnList 1504

// DownloadMissionPack constants

#define kidcMissionPackList 1001
#define kidcMissionPackInfo 1002
#define kidcNumMissions 1010
#define kidcDiscuss 1011

// PickTransportForm constants

#define kidcNoTransportsAvailable 1001
#define kidcTransport1 1101
#define kidcTransport2 1102
#define kidcTransport3 1103
#define kidcTransport4 1104
#define kidcTransport5 1105
#define kidcTransport6 1106

// UnitMenu constants

#define kidcTitle 1200
#define kidcCantTransform 1201

// Relocatable UnitMenu buttons

#define kidcRelocButtonMin 1300

#define kidcRepair 1300
#define kidcSelfDestruct 1301
#define kidcAbortBuild 1302
#define kidcAbortRepair 1303
#define kidcBuild 1304
#define kidcAbortUpgrade 1304
#define kidcResearch 1305
#define kidcTransform 1306
#define kidcDeliver 1306

#define kidcRelocButtonMax 1307

// UnitBuildForm constants

#define kidcName 1001
#define kidcCost 1002
#define kidcMoveRate 1003
#define kidcMoveRateMeter 1103
#define kidcArmorStrength 1004
#define kidcArmorStrengthMeter 1104
#define kidcWeaponStrength 1005
#define kidcWeaponStrengthMeter 1105
#define kidcWeaponRange 1006
#define kidcWeaponRangeMeter 1106
#define kidcList 1007
#define kidcOrder 1008
#define kidcCancelOrder 1009
#define kidcDescription 1010
#define kidcCostMeter 1101
#define kidcLimitReached 1200

// BuildStructure form constants (reuses many BuildVehicle form constants)

#define kidcPowerSupply 1011
#define kidcPowerSupplyMeter 1111
#define kidcPowerDemand 1012
#define kidcPowerDemandMeter 1112

// EcomForm constants

//#define kidcMessage 1001
#define kidcFrom 1011
#define kidcTo 1012
#define kidcFromBitmap 1014
#define kidcToBitmap 1015
#define kidcEcomText 1016

// CutSceneForm constants

//#define kidcMessage 1001
#define kidcBitmap 1002

// Upgrade form constants

#define kidcPrerequisites 1201
#define kidcPrerequisitesLabel 1202
#define kidcCostLabel 1203

// Objectives form constants

#define kidcMissionTitle 1001
#define kidcMissionResult 1002
#define kidcStatistics 1003
//#define kidcRestartMission 1004
//#define kidcAbortMission 1005
#define kidcObjectiveText1 1101
#define kidcObjectiveText2 1102
#define kidcObjectiveText3 1103
#define kidcObjectiveText4 1104
#define kidcObjectiveStatus1 1201
#define kidcObjectiveStatus2 1202
#define kidcObjectiveStatus3 1203
#define kidcObjectiveStatus4 1204
#define kidcPage1 1300
#define kidcObjectiveInfo 1301
#define kidcPage2 1400
#define kidcMobileUnitsKilled 1401
#define kidcStructuresKilled 1402
#define kidcMobileUnitsLost 1403
#define kidcStructuresLost 1404
#define kidcCreditsAction 1405
#define kidcGameTime 1406
#define kidcInfo 1407
#define kidcRankTitle 1408
#define kidcPage3 1500

// GameOptions constants

#define kidcInGameOptions 1001
#define kidcSoundOptions 1002
#define kidcPerformanceOptions 1003
#define kidcColorOptions 1004
#define kidcDisplayOptions 1005
#define kidcDeleteMissionPack 1007

// InGameOptions constants

#define kidcGameSpeed 1201
#define kidcGameSpeedLabel 1202
#define kidcLassoSelection 1003
#define kidcEasy 1004
#define kidcNormal 1005
#define kidcHard 1006
#define kidcMuteSound 1007
#define kidc1Select2Scroll 1203
#define kidcScrollSpeed 1204
#define kidcScrollSpeedLabel 1205
#define kidcMaxFPS 1206
#define kidcMaxFPSLabel 1207

// SoundOptions constants

#define kidcVol 1001
#define kidcVolLabel 1002
#define kidcMute 1003
#define kidcVolumeString 1004

// ColorOptions constants

#define kidcHueLabelString 1001
#define kidcHue 1002
#define kidcHueLabel 1003
#define kidcSatLabelString 1004
#define kidcSat 1005
#define kidcSatLabel 1006
#define kidcLumLabelString 1007
#define kidcLum 1008
#define kidcLumLabel 1009

// DisplayOptions constants

#define kidcModesList 1001

// PerformanceOptions constants

#define kidcRocketShots 1001
#define kidcRocketTrails 1002
#define kidcRocketImpacts 1003
#define kidcShots 1004
#define kidcShotImpacts 1005
#define kidcSelectionBrackets 1006
#define kidcSmoke 1007
#define kidcEnemyDamageIndicator 1008
#define kidcScorchMarks 1009
#define kidcSymbolFlashing 1010

// kidfChooseServer
#define kidcServerList 1300
#define kidcServerName 1301
#define kidcServerLocation 1302
#define kidcServerStatus 1303
#define kidcRefresh 1304

// DRM Code

#define kidcCode 1001
#define kidcEnterKey 1002
//#define kidcPlayDemo 1003

// DRM Key

#define kidc0 1010
#define kidc1 1011
#define kidc2 1012
#define kidc3 1013
#define kidc4 1014
#define kidc5 1015
#define kidc6 1016
#define kidc7 1017
#define kidc8 1018
#define kidc9 1019
#define kidcA 1020
#define kidcB 1021
#define kidcC 1022
#define kidcD 1023
#define kidcE 1024
#define kidcF 1025
#define kidcBackspace 1026
#define kidcKey 1027

// Input Panel

#define kidcInputLabel 1010
#define kidcInputEdit 1011

// Colors

#define kiclrBlack 0
#define kiclrWhite 1
#define kiclrRed 2
#define kiclrGreen 3
#define kiclrYellow 4
#define kiclrSide1 5
#define kiclrSide2 6
#define kiclrSide3 7
#define kiclrSide4 8
#define kiclrButtonFill 9
#define kiclrButtonBorder 10
#define kiclrMenuBack 11
#define kiclrFormBackground 12
#define kiclrMiniMapBorder 13
#define kiclrGalaxite 14
#define kiclrButtonFillHighlight 15
#define kiclrMediumGray 16
#define kiclrBlueSideFirst 17
#define kiclr0BlueSide 17
#define kiclr1BlueSide 18
#define kiclr2BlueSide 19
#define kiclr3BlueSide 20
#define kiclr4BlueSide 21
#define kiclrBlueSideLast 21
#define kiclrRedSideFirst 22
#define kiclr0RedSide 22
#define kiclr1RedSide 23
#define kiclr2RedSide 24
#define kiclr3RedSide 25
#define kiclr4RedSide 26
#define kiclrRedSideLast 26
#define kiclrYellowSideFirst 27
#define kiclr0YellowSide 27
#define kiclr1YellowSide 28
#define kiclr2YellowSide 29
#define kiclr3YellowSide 30
#define kiclr4YellowSide 31
#define kiclrYellowSideLast 31
#define kiclrCyanSideFirst 32
#define kiclr0CyanSide 32
#define kiclr1CyanSide 33
#define kiclr2CyanSide 34
#define kiclr3CyanSide 35
#define kiclr4CyanSide 36
#define kiclrCyanSideLast 36
#define kiclrListBackground 37
#define kiclrListBorder 38
#define kiclrJana 39
#define kiclrAndy 40
#define kiclrOlstrom 41
#define kiclrFox 42
#define kiclrSideNeutral 43
#define kiclrNeutralSideFirst 43
#define kiclr0NeutralSide 43
#define kiclr1NeutralSide 44
#define kiclr2NeutralSide 45
#define kiclr3NeutralSide 46
#define kiclr4NeutralSide 47
#define kiclrNeutralSideLast 47
#define kiclrFullnessIndicator 48

// ARM code resource ids

#define kidrArmCode 1

// Datatypes used for conditions & actions

#define knCaTypeNumber 0
#define knCaTypeQualifiedNumber 1
#define knCaTypeSide 2
#define knCaTypeCounter 3
#define knCaTypeUnit 4
#define knCaTypeModifier 5
#define knCaTypeResource 6
#define knCaTypeWinLose 7
#define knCaTypeOnOff 8
#define knCaTypeCharacter 9
#define knCaTypeText 10
#define knCaTypeRichText 11
#define knCaTypeUnitTypes 12
#define knCaTypeSwitch 13
#define knCaTypeArea 14

// Related enumerations

#define knQualifierAtLeast 0
#define knQualifierAtMost 1
#define knQualifierExactly 2

#define knCaSideNeutral 0
#define knCaSideSide1 1
#define knCaSideSide2 2
#define knCaSideSide3 3
#define knCaSideSide4 4
#define knCaSideEnemies 5
#define knCaSideAllies 6
#define knCaSideAllSides 7
#define knCaSideCurrentSide 8

#define knCounterTypeNone 0
#define knCounterTypeTotal 1
#define knCounterTypeUnits 2
#define knCounterTypeBuildings 3
#define knCounterTypeUnitsAndBuildings 4
#define knCounterTypeKills 5
#define knCounterTypeCustom 6

#define knModifierTypeSet 0
#define knModifierTypeAdd 1
#define knModifierTypeSubtract 2

#define knWinLoseTypeNone 0
#define knWinLoseTypeWin 1
#define knWinLoseTypeLose 2

#define knOnOffTypeNone 0
#define knOnOffTypeOn 1
#define knOnOffTypeOff 2

#define knSmallLargeTypeNone 0
#define knSmallLargeTypeSmallBottom 1
#define knSmallLargeTypeLarge 2
#define knSmallLargeTypeSmallTop 3

#define knMoreCloseTypeNone 0
#define knMoreCloseTypeMore 1
#define knMoreCloseTypeClose 2

#define knModifyCountdownTypeNone 0
#define knModifyCountdownTypeStop 1
#define knModifyCountdownTypeResume 2
#define knModifyCountdownTypeHide 3
#define knModifyCountdownTypeShow 4

#define knCharacterTypeNone 0
#define knCharacterTypeAndy 1
#define knCharacterTypeJana 2
#define knCharacterTypeOlstrom 3
#define knCharacterTypeFox 4
#define knCharacterTypeACME_Security 5
#define knCharacterTypeOMNI_Security 6
#define knCharacterTypeAnonymous 7
#define knCharacterTypeBlank 8

#define knAggressivenessCoward 0
#define knAggressivenessPacifist 1
#define knAggressivenessSelfDefense 2
#define knAggressivenessDefender 3
#define knAggressivenessPitbull 4

#define knModifyNumberTypeNone 0
#define knModifyNumberTypeSet 1
#define knModifyNumberTypeAdd 2
#define knModifyNumberTypeSubtract 3

// Conditions

#define knMissionLoadedCondition 0
#define knCreditsCondition 1
#define knAreaContainsUnitsCondition 2
#define knGalaxiteCapacityReachedCondition 3
#define knMobileHQDeployedCondition 4
#define knPlaceStructureModeCondition 5
#define knElapsedTimeCondition 6
#define knOwnsUnitsCondition 7
#define knMinerCantFindGalaxiteCondition 8
#define knUnitSeesUnitCondition 9
#define knUnitDestroyedCondition 10
#define knSwitchCondition 11
#define knPeriodicTimerCondition 12
#define knDiscoversSideCondition 13
#define knCountdownTimerCondition 14
#define knCounterCondition 15
#define knTestPvarCondition 16
#define knHasUpgradesCondition 17
#define knConditionMax 18

// TriggerActions

#define knSetResourcesTriggerAction 0
#define knSetAllowedUnitsTriggerAction 1
#define knEcomTriggerAction 2
#define knSetObjectiveTriggerAction 3
#define knSetNextMissionTriggerAction 4
#define knEndMissionTriggerAction 5
#define knWaitTriggerAction 6
#define knSetSwitchTriggerAction 7
#define knSetPlayerControlsTriggerAction 8
#define knPreserveTriggerTriggerAction 9
#define knCenterViewTriggerAction 10
#define knPanViewTriggerAction 11
#define knDefogAreaTriggerAction 12
#define knMoveUnitTriggerAction 13
#define knTargetUnitTriggerAction 14
#define knCreateUnitGroupTriggerAction 15
// THis is deliberately the same value as knCreateUnitGroupTriggerAction
#define knCreateUnitAtAreaTriggerAction 15
#define knHuntTriggerAction 16
#define knCreateRandomUnitGroupTriggerAction 17
#define knAlliesTriggerAction 18
#define knStartCountdownTimerTriggerAction 19
#define knModifyCountdownTimerTriggerAction 20
#define knRepairTriggerAction 21
#define knEnableReplicatorTriggerAction 22
#define knModifyCreditsTriggerAction 23
#define knModifyCounterTriggerAction 24
#define knMoveUnitsInAreaTriggerAction 25
#define knSetFormalObjectiveTextTriggerAction 26
#define knSetFormalObjectiveStatusTriggerAction 27
#define knShowObjectivesTriggerAction 28
#define knSetFormalObjectiveInfoTriggerAction 29
#define knCutSceneTriggerAction 30
#define knJumpToMissionTriggerAction 31
#define knModifyPvarTriggerAction 32
#define knSetPvarTextTriggerAction 33
#define knShowAlertTriggerAction 34
#define knSetAllowedUpgradesTriggerAction 35
#define knSetUpgradesTriggerAction 36

// UnitGroupActions

#define knWaitUnitGroupAction 0
#define knSetSwitchUnitGroupAction 1
#define knMoveUnitGroupAction 2
#define knAttackUnitGroupAction 3
#define knGuardUnitGroupAction 4
#define knMineUnitGroupAction 5
#define knGuardVicinityUnitGroupAction 6

// UnitActions

#define knGuardUnitAction 0
#define knGuardVicinityUnitAction 1
#define knGuardAreaUnitAction 2
#define knMoveUnitAction 3
#define knHuntEnemiesUnitAction 4
#define knMineUnitAction 5

// sound effects for triggers

#define knsfxHappyEnding 0
#define knsfxExplosion 1
