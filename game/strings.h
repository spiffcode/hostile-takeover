#ifndef __STRINGS_H__
#define __STRINGS_H__

#ifndef __OBJC__ // SDL.h ends up including this when it is trying to include the CRT strings.h

namespace wi {

enum Strings {
	kidsMinerUnderAttack, // Miner Under Attack!
	kidsMinerNeedsGalaxite, // Bullpup can't find Galaxite!
	kidsLowPower, // Low Power!
	kidsWarehouseTooFull, // Need More Storage!
	kidsBaseUnderAttack, // Base Under Attack!
	kidsNewVehicleOptions, // New Vehicle Options!
	kidsNewRecruitOptions, // New Recruit Options!
	kidsNewStructureOptions, // New Structure Options!
	kidsUnitLimitReached, // Unit Limit Reached!
	kidsBuildingLimitReached, // Building Limit Reached!
	kidsRankChallenger, // Challenger
	kidsRank0,  // Mining Operations Trainee
	kidsRank1,  // Mining Operations Specialist
	kidsRank2,  // Certified Basic Miner
	kidsRank3,  // Mining & Security Specialist
	kidsRank4,  // Certified Secure Miner
	kidsRank5,  // Senior Secure Miner
	kidsRank6,  // Lead Secure Miner
	kidsRank7,  // Secure Mining Site Supervisor
	kidsRank8,  // Secure Mining Supervisor
	kidsRank9,  // Secure Mining Base Commander
	kidsRank10, // Jr. Director of Secure Mining
	kidsRank11, // ACME Up-and-Comer
	kidsRank12, // Operations Fast-Tracker
	kidsRank13, // Operations Management Trainee
	kidsRank14, // Operations Manager
	kidsRank15, // Operations Middle-Manager
	kidsRank16, // Operations Bureaucrat
	kidsRank17, // Being Groomed For Executive
	kidsRank18, // Operations Executive
	kidsRank19, // Director of Icarus Operations
	kidsRank20, // Director of ACME Ops Resources
	kidsRank21, // Vice President of Operations
	kidsRank22, // Executive VP of Operations
	kidsRank23, // Senior Exec. VP of Operations
	kidsRank24, // ACME Chief Operations Officer
	kidsExitHost, // You are hosting the multiplayer game in progress. Are you sure you want to stop?
	kidsExitClient, // You cannot return to a multiplayer game in progress. Are you sure you want to leave?
};

} // namespace wi

#endif // ndef __OBJC__
#endif // ndef __STRINGS_H__
