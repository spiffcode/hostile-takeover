#ifndef __SIDE_H__
#define __SIDE_H__

// Sides

#define ksideNeutral 0
#define kside1 1
#define kside2 2
#define kside3 3
#define kside4 4
#define kcSides 5

#define knIntelligenceHuman 0
#define knIntelligenceComputer 1
#define knIntelligenceComputerOvermind 2
#define knIntelligenceComputerNeutral 3

// NOTE: kut values can not be changed without breaking old levels

#define kutNone -1

// Infantry Units (NOTE: this is the order they will appear in the build form)
#define kutShortRangeInfantry 0
#define kutLongRangeInfantry 1
#define kutTakeoverSpecialist 2

// Vehicle Units (NOTE: this is the order they will appear in the build form)
#define kutMachineGunVehicle 3
#define kutLightTank 4
#define kutRocketVehicle 5
#define kutMediumTank 6
#define kutGalaxMiner 7
#define kutMobileHeadquarters 8

// Structures (NOTE: this is the order they will appear in the build form)
#define kutReactor 9
#define kutProcessor 10
#define kutWarehouse 11
#define kutHumanResourceCenter 12
#define kutVehicleTransportStation 13
#define kutRadar 14
#define kutResearchCenter 15
#define kutHeadquarters 16

// Special structures that like to kill
#define kutMachineGunTower 17
#define kutRocketTower 18

#define kutAndy 19
#define kutArtillery 20
#define kutReplicator 21
#define kutFox 22

#define kutMax 23

#endif // __SIDE_H__
