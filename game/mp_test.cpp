#include "ht.h"

#ifdef MP_STRESS

bool gfMPStress;
int gnMPPos;

int FindGobs(dword ffGob, WCoord wxNear, WCoord wyNear, State st, bool fAlly, int cgob, Gob **apgob)
{
	Gob *apgobSort[kcpgobMax];
	int anDistSort[kcpgobMax];
	int cgobSort = 0;
	int cgobReturn = 0;

	for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
		if ((pgob->GetFlags() & ffGob) != ffGob)
			continue;
		if (st != kstReservedNull && pgob->m_st != st)
			continue;
		if (pgob->GetFlags() & kfGobUnit) {
			UnitGob *punt = (UnitGob *)pgob;
			if (fAlly) {
				if (!punt->IsAlly(gpplrLocal->GetSide()))
					continue;
			} else {
				if (punt->IsAlly(gpplrLocal->GetSide()))
					continue;
			}
		}

		if (wxNear == kwxInvalid) {
			apgob[cgobReturn++] = pgob;
			if (cgobReturn == cgob)
				break;
		} else {
			// Remember so we can sort

			WPoint wpt;
			pgob->GetPosition(&wpt);
			apgobSort[cgobSort] = pgob;
			anDistSort[cgobSort] = (wxNear - wpt.wx) * (wxNear - wpt.wx) + (wyNear - wpt.wy) * (wyNear - wpt.wy);
			cgobSort++;
		}
	}

	if (wxNear == kwxInvalid)
		return cgobReturn;

	// Sort what we've found

	for (int i = cgobSort - 1; i >= 0; i--) {
		for (int j = 1; j <= i; j++) {
			if (anDistSort[j - 1] > anDistSort[j]) {
				Gob *pgobT = apgobSort[j];
				apgobSort[j] = apgobSort[j - 1];
				apgobSort[j - 1] = pgobT;
				int nT = anDistSort[j];
				anDistSort[j] = anDistSort[j - 1];
				anDistSort[j - 1] = nT;
			}
		}
	}

	// Copy the first cgob's worth

	int igob = 0;
	for (; igob < cgob && igob < cgobSort; igob++)
		apgob[igob] = apgobSort[igob];
	return igob;
}

bool MPWait(int *pcupdWait, int cupdInterval)
{
	if (*pcupdWait == 0) {
		*pcupdWait = cupdInterval;
		return false;
	}
	(*pcupdWait)--;
	return true;
}

void MPMineStressUpdate()
{
	static int s_cupdWait;
	if (MPWait(&s_cupdWait, 36))
		return;

	// Find all the miners, see what they are doing. If moving or guard state, give it a mine command

	for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
		if (pgob->GetType() != kgtGalaxMiner || !(pgob->GetFlags() & kfGobActive))
			continue;
		MinerGob *pgobMiner = (MinerGob *)pgob;
		if (pgobMiner->m_fHidden)
			continue;
		if (pgobMiner->m_st != kstGuard && pgobMiner->m_st != kstMove)
			continue;

		Message msg;
		memset(&msg, 0, sizeof(msg));
		msg.mid = kmidMineCommand;
		msg.MineCommand.gidTarget = kgidNull;
		msg.MineCommand.wptTarget.wx = kwxInvalid;
		msg.MineCommand.wptTarget.wy = kwxInvalid;
		msg.smidReceiver = pgobMiner->GetId();
		gcmdq.Enqueue(&msg);
	}
}

void MPAttackStressUpdate()
{
	static int s_cupdWait;
	if (MPWait(&s_cupdWait, 72))
		return;

	// Find an enemy - mobile first

#if 0
	Gob *apgobEnemy[2];
	int cEnemies = FindGobs(kfGobUnit | kfGobActive | kfGobMobileUnit, kwxInvalid, kwxInvalid, kstReservedNull, false, ARRAYSIZE(apgobEnemy), apgobEnemy);
	if (cEnemies == 0) {
		// Find an enemy - anything

		cEnemies = FindGobs(kfGobUnit | kfGobActive, kwxInvalid, kwxInvalid, kstReservedNull, false, ARRAYSIZE(apgobEnemy), apgobEnemy);
		if (cEnemies == 0)
			return;
	}
#else
	Gob *apgobEnemy[2];
	int cEnemies = FindGobs(kfGobUnit | kfGobActive, kwxInvalid, kwxInvalid, kstReservedNull, false, ARRAYSIZE(apgobEnemy), apgobEnemy);
	if (cEnemies == 0)
		return;
#endif

	// Attack the headquarters only as a last resort

	UnitGob *puntEnemy = (UnitGob *)apgobEnemy[0];
	if (puntEnemy->GetUnitType() == kutHeadquarters) {
		if (cEnemies > 1)
			puntEnemy = (UnitGob *)apgobEnemy[1];
	}

	// Find a number of grouped allies that are idle

	WPoint wpt;
	puntEnemy->GetPosition(&wpt);
	Gob *apgob[kcpgobMax];
	int cUnits = FindGobs(kfGobMobileUnit | kfGobActive, wpt.wx, wpt.wy, kstGuard, true, ARRAYSIZE(apgob), apgob);
	if (cUnits == 0)
		return;

	// Send these units to attack this enemy

#define kcpgobAttack 25

	int cAttacking = 0;
	for (int n = 0; n < cUnits && cAttacking <= kcpgobAttack; n++) {
		// Limit # of attackers. Already attacking?

		Gob *pgob = apgob[n];
		if (pgob->m_st == kstAttack) {
			cAttacking++;
			continue;
		}

		// Queue attack command

		Message msgT;
		msgT.mid = kmidAttackCommand;
		msgT.smidSender = apgob[n]->GetId();
		msgT.smidReceiver = apgob[n]->GetId();
		msgT.AttackCommand.wptTarget.wx = wpt.wx;
		msgT.AttackCommand.wptTarget.wy = wpt.wy;
		msgT.AttackCommand.gidTarget = puntEnemy->GetId();
		msgT.AttackCommand.wptTargetCenter.wx = wpt.wx;
		msgT.AttackCommand.wptTargetCenter.wy = wpt.wy;
		msgT.AttackCommand.tcTargetRadius = 1;
		msgT.AttackCommand.wcMoveDistPerUpdate = 0;
		gcmdq.Enqueue(&msgT);
		cAttacking++;
	}
}

void MPMoveStressUpdate()
{
	static int s_cupdWait;
	if (MPWait(&s_cupdWait, 24))
		return;

	// Find a number of grouped allies that are idle

	Gob *apgob[1];
	int cUnits = FindGobs(kfGobMobileUnit | kfGobActive, kwxInvalid, kwxInvalid, kstGuard, true, ARRAYSIZE(apgob), apgob);
	if (cUnits == 0)
		return;

	// Send this unit to some random place that won't cause an attack

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	TCoord tx = GetAsyncRandom() % ctx;
	TCoord ty = GetAsyncRandom() % cty;
	WPoint wpt;
	FindNearestFreeTile(tx, ty, &wpt);

	// Queue move command

	Message msgT;
	msgT.mid = kmidMoveCommand;
	msgT.smidSender = apgob[0]->GetId();
	msgT.smidReceiver = apgob[0]->GetId();
	msgT.MoveCommand.wptTarget.wx = wpt.wx;
	msgT.MoveCommand.wptTarget.wy = wpt.wy;
	msgT.MoveCommand.gidTarget = kgidNull;
	msgT.MoveCommand.wptTargetCenter.wx = wpt.wx;
	msgT.MoveCommand.wptTargetCenter.wy = wpt.wy;
	msgT.MoveCommand.tcTargetRadius = 1;
	msgT.MoveCommand.wcMoveDistPerUpdate = 0;
	gcmdq.Enqueue(&msgT);
}

struct UnitsMaintain { // unm
	UnitType utBuilder;
	int cUnits;
};

//Max unit counts
//#define kcStructGobsHumanMax 55
//#define kcStructGobsComputerMax 72
//#define kcMuntGobsHumanMax 88
//#define kcMuntGobsComputerMax 117

UnitsMaintain gaunm[] = {
	{ kutHumanResourceCenter, 5}, // kutShortRangeInfantry
	{ kutHumanResourceCenter, 10}, // kutLongRangeInfantry
	{ kutHumanResourceCenter, 4}, // kutTakeoverSpecialist
	{ kutVehicleTransportStation, 5}, // kutMachineGunVehicle
	{ kutVehicleTransportStation, 10}, // kutLightTank
	{ kutVehicleTransportStation, 10}, // kutRocketVehicle
	{ kutVehicleTransportStation, 10}, // kutMediumTank
	{ kutVehicleTransportStation, 4}, // kutGalaxMiner
	{ kutVehicleTransportStation, 2}, // kutMobileHeadquarters
	{ kutHeadquarters, 10}, // kutReactor
	{ kutHeadquarters, 1}, // kutProcessor
	{ kutHeadquarters, 8}, // kutWarehouse
	{ kutHeadquarters, 2}, // kutHumanResourceCenter
	{ kutHeadquarters, 2}, // kutVehicleTransportStation
	{ kutHeadquarters, 1}, // kutRadar
	{ kutHeadquarters, 1}, // kutResearchCenter
	{ kutHeadquarters, 1}, // kutHeadquarters
	{ kutHeadquarters, 10}, // kutMachineGunTower
	{ kutHeadquarters, 10}, // kutRocketTower
	{ kutHumanResourceCenter, 1}, // kutAndy
	{ kutVehicleTransportStation, 5}, // kutArtillery
	{ kutHeadquarters, 0}, // kutReplicator
	{ kutHumanResourceCenter, 5}, // kutFox
};

WPoint FindInitPositionForStructure(BuilderGob *pbldr, int ctxReserve, int ctyReserve)
{
	WPoint wpt;
	pbldr->GetPosition(&wpt);
	TCoord txStart = TcFromWc(wpt.wx);
	TCoord tyStart = TcFromWc(wpt.wy);
	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);

	for (TCoord ty = 0; ty < cty - ctyReserve; ty++) {
		for (TCoord tx = 0; tx < ctx - ctxReserve; tx++) {
			TCoord txT = (tx + txStart) % ctx;
			TCoord tyT = (ty + tyStart) % cty;

			bool fOpen = true;
			for (TCoord tyS = 0; tyS < ctyReserve; tyS++) {
				for (TCoord txS = 0; txS < ctxReserve; txS++) {
					if (!IsTileFree(txS + txT, tyS + tyT, kbfReserved | kbfStructure | kbfMobileUnit)) {
						fOpen = false;
						break;
					}
				}
				if (!fOpen)
					break;
			}
			if (fOpen) {
				wpt.wx = WcFromTc(txT);
				wpt.wy = WcFromTc(tyT);
				return wpt;
			}
		}
	}

	return wpt;
}

void BuildUnits(UnitType ut, int cUnits, UnitType utBuilder, int *acUnitCapacityRemaining)
{
	// Find all instances of the desired unit type either built and active or queued to be built

	int cUnitsActive = gpplrLocal->GetUnitCount(ut);
	if (cUnitsActive >= cUnits)
		return;

	// Find all builders of type utBuilder

	int cUnitsQueued = 0;
	UnitGob *apunt[kcpgobMax];
	UnitGob **ppuntT = apunt;
	for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
		if ((pgob->GetFlags() & (kfGobActive | kfGobStructure)) != (kfGobActive | kfGobStructure))
			continue;
		UnitGob *punt = (UnitGob *)pgob;
		if (punt->GetOwner() != gpplrLocal)
			continue;
		if (punt->GetUnitType() != utBuilder)
			continue;

		// # of queue units?

		BuilderGob *pbldr = ((BuilderGob *)punt);
		cUnitsQueued += pbldr->m_bq.GetUnitCount(ut);
		*ppuntT++ = punt;
	}

	// If # of active and queued units are met, return

	int cUnitsBuild = cUnits - (cUnitsActive + cUnitsQueued);
	if (cUnitsBuild <= 0)
		return;

	// If no builders for this unit type, wait for one to be built

	int cBuilders = ppuntT - apunt;
	if (cBuilders == 0)
		return;

	// Build!

	for (int n = 0; n < cBuilders; n++) {
		// Is this builder full?

		BuilderGob *pbldr = (BuilderGob *)apunt[n];
		int *pcOpen = &acUnitCapacityRemaining[pbldr->GetId() / sizeof(Gob *)];
		if (*pcOpen == 0)
			continue;

		while (*pcOpen > 0 && cUnitsBuild > 0) {
			// Find coordinates for this unit
			
			WPoint wpt;
			if (!((1UL << ut) & kumStructures)) {
				pbldr->FindInitPosition(&wpt);
			} else {
				StructConsts *pstruc = (StructConsts *)gapuntc[ut];
				wpt = FindInitPositionForStructure(pbldr, pstruc->ctxReserve, pstruc->ctyReserve);
			}

			Message msg;
			memset(&msg, 0, sizeof(msg));
			msg.smidSender = ksmidNull;
			msg.mid = kmidBuildOtherCommand;
			msg.BuildOtherCommand.ut = (UnitType)ut;
			msg.BuildOtherCommand.wpt.wx = wpt.wx;
			msg.BuildOtherCommand.wpt.wy = wpt.wy;
			msg.smidReceiver = pbldr->GetId();
			gcmdq.Enqueue(&msg);

			(*pcOpen)--;
			cUnitsBuild--;
		}
		if (cUnitsBuild == 0)
			break;
	}
}

void MPBuildStressUpdate()
{
	static int s_cupdWait;
	if (MPWait(&s_cupdWait, 36))
		return;

	// Get the capacities of all builder gobs. Need to calc this up front since capacities don't change
	// when build orders are queued

	int acUnitCapacityRemaining[kcpgobMax + 1];
	memset(acUnitCapacityRemaining, 0, sizeof(acUnitCapacityRemaining));
	for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
		if ((pgob->GetFlags() & (kfGobActive | kfGobStructure)) != (kfGobActive | kfGobStructure))
			continue;
		UnitGob *punt = (UnitGob *)pgob;
		if (!(punt->GetConsts()->wf & (kfUntcStructureBuilder | kfUntcMobileUnitBuilder)))
			continue;
		BuilderGob *pbldr = (BuilderGob *)punt;
		acUnitCapacityRemaining[pbldr->GetId() / sizeof(Gob *)] = pbldr->m_bq.GetRemainingCapacity();
	}

	// First ensure the builders are accounted for

	BuildUnits(kutHeadquarters, 1, kutHeadquarters, acUnitCapacityRemaining);
	for (int ut = 0; ut < ARRAYSIZE(gaunm); ut++) {
		if (gaunm[ut].cUnits == 0)
			continue;
		UnitType utBuilder = gaunm[ut].utBuilder;
		BuildUnits(utBuilder, gaunm[utBuilder].cUnits, kutHeadquarters, acUnitCapacityRemaining);
	}

	// Now ensure other units are accounted for. Enumerate in a random order

	UnitType autOrder[kutMax];
	memset(autOrder, 0xff, sizeof(autOrder));
	for (int ut = 0; ut < ARRAYSIZE(autOrder); ut++) {
		int n;
		while (true) {
			n = GetAsyncRandom() % kutMax;
			if (autOrder[n] == kutNone)
				break;
		}
		autOrder[n] = ut;
	}

	for (int n = 0; n < ARRAYSIZE(autOrder); n++) {
		UnitType ut = autOrder[n];
		if (gaunm[ut].cUnits == 0)
			continue;
		if (ut == kutHeadquarters)
			continue;
		BuildUnits(ut, gaunm[ut].cUnits, gaunm[ut].utBuilder, acUnitCapacityRemaining);
	}

	// UNDONE: kmidAbortBuildOtherCommand
}

void MPStressUpdate()
{
	if (!gfMPStress)
		return;

	MPMineStressUpdate(); // kmidMineCommand
	MPAttackStressUpdate(); // kmidAttackCommand
	MPMoveStressUpdate(); // kmidMoveCommand
	MPBuildStressUpdate(); // kmidBuildOtherCommand, kmidAbortBuildOtherCommand

#if 0
	MPUpgradeStressUpdate(); // kmidUpgradeCommand, kmidAbortUpgradeCommand
	MPTransformStressUpdate(); // kmidTransformCommand
	MPSelfDestructStressUpdate(); // kmidSelfDestructCommand
	MPRepairStressUpdate(); // kmidRepairCommand
#endif
}
#endif

#ifdef MP_DEBUG_SHAREDMEM

HANDLE ghProcessServer;
HANDLE ghSharedMem;
SharedMemWindow *gpsmw;
bool gfMPServer;
Side gsideCurrent;
Gid ggidCurrent;
long gcupdCurrent;
GobType ggtCurrent;

#ifndef DEBUG
#error Currently requires DEBUG since Assert is used functionally
#endif

void MPInitSharedMemoryWindow(bool fServer)
{
	if (fServer) {
		ghSharedMem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 8192, "wi_shared_mem");
		if (ghSharedMem == NULL)
			return;
		gpsmw = (SharedMemWindow *)MapViewOfFile(ghSharedMem, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		gpsmw->pidProcess = GetCurrentProcessId();
#ifdef DETECT_SYNC_ERRORS
		gpsmw->fDetectSyncErrors = gfLockStep;
#else
		gpsmw->fDetectSyncErrors = false;
#endif
	} else {
		ghSharedMem = OpenFileMapping(FILE_MAP_READ, FALSE, "wi_shared_mem");
		if (ghSharedMem == NULL)
			return;
		gpsmw = (SharedMemWindow *)MapViewOfFile(ghSharedMem, FILE_MAP_READ, 0, 0, 0);
		ghProcessServer = OpenProcess(PROCESS_VM_READ, FALSE, gpsmw->pidProcess);
	}
	gfMPServer = fServer;
}

void MPExitSharedMemoryWindow(bool fAmServer)
{
	UnmapViewOfFile(gpsmw);
	gpsmw = NULL;
	CloseHandle(ghSharedMem);
	ghSharedMem = NULL;
	if (!gfMPServer) {
		CloseHandle(ghProcessServer);
		ghProcessServer = NULL;
	}
}

void MPUpdateState()
{
	if (gfMPServer)
		memcpy(gpsmw->apgobMaster, ggobm.m_apgobMaster, (kcpgobMax + 1) * sizeof(Gob *));
}

void MPValidateState()
{
	if (!gfMPServer && gpsmw->fDetectSyncErrors) {
		for (Gob *pgob = ggobm.GetFirstGob(); pgob != NULL; pgob = ggobm.GetNextGob(pgob)) {
			if (!(pgob->GetFlags() & kfGobUnit))
				continue;
			UnitGob *punt = (UnitGob *)pgob;
			punt->MPValidate();
		}
	}
}

void MPCopyMem(void *pvTo, void *pvFrom, int cb)
{
	Assert(!gfMPServer);
	SIZE_T cbRead;
	BOOL f = ReadProcessMemory(ghProcessServer, pvFrom, pvTo, cb, &cbRead);
	Assert(f && cb == cbRead);
}

Gob *MPGetGobPtr(Gid gid)
{
	Assert(!gfMPServer);
	Assert(gid >= 0 && gid < (kcpgobMax + 1) * sizeof(Gob *));
	Gob *pgob = *((Gob **)((byte *)gpsmw->apgobMaster  + (gid)));
	Assert(((dword)pgob & 1) == 0);
	return pgob;
}	

void MPValidateMemory(void *pvRemote, void *pvLocal, int cb)
{
	if (gfMPServer)
		return;

	byte abT[512];
	Assert(cb <= sizeof(abT));

	SIZE_T cbRead;
	BOOL f = ReadProcessMemory(ghProcessServer, pvRemote, abT, cb, &cbRead);
	Assert(f && cb == cbRead);
	Assert(memcmp(abT, pvLocal, cb) == 0);
}

byte gabRemoteCache[16 * 1024];
void *gpvRemoteCache;
int gcbRemoteCache;
long gcupdRemoteCache;
bool gfInsideUpdate;

void MPValidateMember2(void *pvLocal, void *pvRemote, int cbThis, int cbOffsetMem, int cbMem)
{
	if (gfMPServer)
		return;

	// If inside an update, check state directly

	if (gfInsideUpdate) {
		MPValidateMemory(((byte *)pvLocal) + cbOffsetMem, ((byte *)pvRemote) + cbOffsetMem, cbMem);
		return;
	}

	// Update our speedup cache

	if (gcupdRemoteCache == 0 || gcupdCurrent != gcupdRemoteCache || pvRemote != gpvRemoteCache || cbOffsetMem + cbMem > gcbRemoteCache) {
		// Update the cache

		Assert(cbOffsetMem + cbMem <= cbThis);
		MPCopyMem(gabRemoteCache, pvRemote, cbThis);
		gpvRemoteCache = pvRemote;
		gcbRemoteCache = cbThis;
		gcupdRemoteCache = gcupdCurrent;
	}

	// Check memory

	Assert(memcmp(&gabRemoteCache[cbOffsetMem], &((byte *)pvLocal)[cbOffsetMem], cbMem) == 0);
}
#endif
