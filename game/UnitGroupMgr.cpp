#include "ht.h"

namespace wi {

//
// UnitGroupMgr
//

UnitGroupMgr::UnitGroupMgr()
{
	m_cug = 0;
	m_aug = NULL;
}

UnitGroupMgr::~UnitGroupMgr()
{
	delete[] m_aug;
	m_aug = NULL;
	m_cug = 0;
}

bool UnitGroupMgr::Init(IniReader *pini)
{
	// Get trigger count

	int cArgs = pini->GetPropertyValue("General", "UnitGroupCount", "%d", &m_cug);
	if (cArgs != 1)
		return false;

	// Allocate UnitGroup storage, one chunk

	Assert(m_aug == NULL);
	m_aug = new UnitGroup[m_cug];
	Assert(m_aug != NULL, "out of memory!");
	if (m_aug == NULL)
		return false;

	for (int iug = 0; iug < m_cug; iug++) {
		if (!m_aug[iug].Init(pini, iug)) {
			delete m_aug;
			m_aug = NULL;
			return false;
		}
	}

	return true;
}

#define knVerUnitGroupMgrState 1
bool UnitGroupMgr::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerUnitGroupMgrState)
		return false;

	for (int i = 0; i < m_cug; i++)
		m_aug[i].LoadState(pstm);

	return pstm->IsSuccess();
}

bool UnitGroupMgr::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerUnitGroupMgrState);

	for (int i = 0; i < m_cug; i++)
		m_aug[i].SaveState(pstm);

	return pstm->IsSuccess();
}

void UnitGroupMgr::ActivateUnitGroup(int iug)
{
	// If the group is already active leave it alone

	if ((m_aug[iug].GetFlags() & kfUgActive) != 0)
		return;

	m_aug[iug].Activate();
}

// Rob expects random groups to be activated in a fashion that is
// not truly random. The idea is that groups should activate randomly
// but not repeat until all have been activated (aka shuffle)

void UnitGroupMgr::ActivateRandomUnitGroup()
{
	// Figure out how many inactive random groups there are

	int cug = 0, iug, cugNotRecentlyActive = 0;
	UnitGroup *pug = m_aug;
	for (iug = 0; iug < m_cug; iug++, pug++) {
		word wf = pug->GetFlags();
		if ((wf & (kfUgActive | kfUgRandomGroup)) == kfUgRandomGroup) {
			if (wf & kfUgNotRecentlyActivated)
				cugNotRecentlyActive++;
			cug++;
		}
	}

	if (cug == 0)
		return;		// nothing to activate

	// Everyone has been activated recently, start afresh

	if (cugNotRecentlyActive == 0) {
		pug = m_aug;
		for (iug = 0; iug < m_cug; iug++, pug++) {
			word wf = pug->GetFlags();
			if ((wf & (kfUgActive | kfUgRandomGroup)) == kfUgRandomGroup)
				pug->SetFlags(wf | kfUgNotRecentlyActivated);
		}
	} else {
		cug = cugNotRecentlyActive;
	}

	// Pick one of the random groups

	cug = (GetRandom() % cug);

	// Search back through again to find the picked group

	pug = m_aug;
	for (iug = 0; true; pug++) {
		if ((pug->GetFlags() & (kfUgActive | kfUgRandomGroup | kfUgNotRecentlyActivated)) == (kfUgRandomGroup | kfUgNotRecentlyActivated)) {
			if (iug == cug) {
				pug->Activate();
				return;
			}
			iug++;
		}
	}

	// Activate it

	pug->Activate();
}

// OPT: this doesn't need to be called every Simulation::Update

void UnitGroupMgr::Update()
{
	UnitGroup *pug = m_aug;
	for (int iug = 0; iug < m_cug; iug++, pug++)
		if (pug->GetFlags() & kfUgActive)
			pug->Update();

#ifdef DEBUG_HELPERS
	extern void UpdateUnitGroupViewer();
	UpdateUnitGroupViewer();
#endif
}

bool UnitGroupMgr::CreateAtLevelLoadGroups()
{
	UnitGroup *pug = m_aug;
	for (int iug = 0; iug < m_cug; iug++, pug++)
		if (pug->GetFlags() & kfUgCreateAtLevelLoad)
			pug->Activate();
	return true;
}

UnitGroup *UnitGroupMgr::GetUnitGroup(Gid gid)
{
	UnitGroup *pug = m_aug;
	for (int iug = 0; iug < m_cug; iug++, pug++) {
		if (!(pug->GetFlags() & kfUgActive))
			continue;

		int cule = pug->GetUnitCount();
		UnitListEntry *pule = pug->GetUnitList();
		for (int iule = 0; iule < cule; iule++, pule++) {
			if (pule->gid == gid && (pule->bf & kfUleBuilt))
				return pug;
		}
	}

	return NULL;
}

//
// UnitGroup
//

UnitGroup::UnitGroup()
{
#ifdef DEBUG
	m_szName[0] = 0;
#endif
	m_pactn = NULL;
	m_pactnLast = NULL;
	m_wf = kfUgNotRecentlyActivated;
	m_cule = 0;
	m_aule = NULL;
	m_pplr = NULL;
	m_wfMunt = 0;
	m_nSpawnArea = -1;
}

UnitGroup::~UnitGroup()
{
	delete m_aule;

	// Delete Actions

	UnitGroupAction *pactn = m_pactn;
	while (pactn != NULL) {
		UnitGroupAction *pactnNext = pactn->m_pactnNext;
		delete pactn;
		pactn = pactnNext;
	}
}

#define knVerUnitGroupState 5
bool UnitGroup::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerUnitGroupState)
		return false;

	// Read the last (in-progress) action (if any)

	char nAction = pstm->ReadByte();
	if (nAction != -1) {
		UnitGroupAction *pactn = m_pactn;
		for (int j = 0; j < nAction; j++, pactn = pactn->m_pactnNext);
		m_pactnLast = pactn;
	}

	// Read the UnitGroup flags

	m_wf = pstm->ReadWord();

	// Read the UnitListEntries

	int cule = pstm->ReadWord();
	if (cule != m_cule) {
		UnitListEntry *aule = new UnitListEntry[cule];
		Assert(aule != NULL, "out of memory!");
		if (aule != NULL) {
			delete m_aule;
			m_aule = aule;
			m_cule = cule;
		}
	}

	for (int i = 0; i < m_cule; i++) {
		m_aule[i].ut = pstm->ReadWord();
		m_aule[i].gid = pstm->ReadWord();
		m_aule[i].bf = pstm->ReadByte();
	}

	// Give each Action a chance to Read its state

	for (UnitGroupAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext)
		pactn->LoadState(pstm);

	m_wfMunt = pstm->ReadWord();

	return pstm->IsSuccess();
}

bool UnitGroup::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerUnitGroupState);

	// Map m_pactnLast into an index or -1 for no last action

	if (m_pactnLast == NULL) {
		pstm->WriteByte((byte)-1);
	} else {
		char nAction = 0;
		for (UnitGroupAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext, nAction++)
			if (pactn == m_pactnLast)
				break;
		pstm->WriteByte(nAction);
	}

	// Write the UnitGroup flags

	pstm->WriteWord(m_wf);

	// Write the UnitListEntries because some may have been added dynamically

	pstm->WriteWord(m_cule);
	for (int i = 0; i < m_cule; i++) {
		pstm->WriteWord(m_aule[i].ut);
		pstm->WriteWord(m_aule[i].gid);
		pstm->WriteByte(m_aule[i].bf);
	}

	// Give each Action a chance to write its state

	for (UnitGroupAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext)
		pactn->SaveState(pstm);

	pstm->WriteWord(m_wfMunt);

	return pstm->IsSuccess();
}

bool UnitGroup::Init(IniReader *pini, int iug)
{
	char szUnitGroup[50];
	sprintf(szUnitGroup, "UnitGroup %d", iug);

	char szProp[32];
	FindProp find;
	while (pini->FindNextProperty(&find, szUnitGroup, szProp, sizeof(szProp))) {
		switch (szProp[0]) {
		case 'S':	// Side or Spawn or SpawnArea
			{
				int n;
				pini->GetPropertyValue(&find, "%d", &n);

				switch (szProp[1]) {
				case 'i':	// Side
					// Read the UnitGroup's side and map it to its owning Player

					Assert(strcmp(szProp, "Side") == 0);
					m_pplr = gplrm.GetPlayer(n);
					break;

				case 'p':	// Spawn or SpawnArea
					if (strcmp(szProp, "SpawnArea") == 0)
						m_nSpawnArea = n;	// SpawnArea
					else
						m_wf |= n != 0 ? kfUgSpawn : 0;	// Spawn
					break;
				}
			}
			break;

		case 'L':	// LoopForever
			{
				Assert(strcmp(szProp, "LoopForever") == 0);
				int n;
				pini->GetPropertyValue(&find, "%d", &n);
				if (n != 0)
					m_wf |= kfUgLoopForever;
			}
			break;

		case 'R':	// RandomGroup or ReplaceUnits
			{
				int n;
				pini->GetPropertyValue(&find, "%d", &n);

				switch (szProp[1]) {
				case 'a':	// RandomGroup
					Assert(strcmp(szProp, "RandomGroup") == 0);
					if (n != 0)
						m_wf |= kfUgRandomGroup;
					break;

				case 'e':	// ReplaceUnits (aka "Recreate if destroyed")
					Assert(strcmp(szProp, "ReplaceGroup") == 0);
					if (n != 0)
						m_wf |= kfUgReplaceGroup;
					break;
				}
			}
			break;

		case 'C':	// CreateAtLevelLoad
			{
				Assert(strcmp(szProp, "CreateAtLevelLoad") == 0);
				int n;
				pini->GetPropertyValue(&find, "%d", &n);
				if (n != 0)
					m_wf |= kfUgCreateAtLevelLoad;
			}
			break;

		case 'U':
			{
				// Expand Units string to an array of UnitListEntries

				Assert(strcmp(szProp, "Units") == 0);
				char szT[256];
				pini->GetPropertyValue(&find, "%s", szT);

				char *psz = szT;
				if (!ParseNumber(&psz, &m_cule))
					return false;

				m_aule = new UnitListEntry[m_cule];
				Assert(m_aule != NULL, "out of memory!");
				if (m_aule == NULL)
					break;

				UnitListEntry *pule = m_aule;
				int nUnitType;
				while (ParseNumber(&psz, &nUnitType)) {
					int cUnits;
					if (!ParseNumber(&psz, &cUnits))
						return false;

					for (int i = 0; i < cUnits; i++, pule++) {
						pule->ut = (UnitType)nUnitType;
						pule->gid = kgidNull;
						pule->bf = 0;
					}
				}
				Assert(pule - m_aule == m_cule);
			}
			break;

		case 'A':
			if (szProp[1] == 'g') {
				int nAggressiveness;
				pini->GetPropertyValue(&find, "%d", &nAggressiveness);
				m_wfMunt = AggBitsFromAgg(nAggressiveness);
			} else {
				if (!LoadAction(pini, &find))
					return false;
			}
			break;

		case 'H':
			Assert(strcmp(szProp, "Health") == 0);
			pini->GetPropertyValue(&find, "%d", &m_nHealth);
			break;

#ifdef DEBUG
		case 'N':	// Name
			Assert(strcmp(szProp, "Name") == 0);
			pini->GetPropertyValue(&find, "%s", m_szName);
			break;
#endif

		default:
			Assert(false);
			break;
		}
	}

	// If this group is to be spawned it better have a spawn area

	Assert((m_wf & kfUgSpawn) == 0 || m_nSpawnArea != -1);

	return true;
}

void UnitGroup::OnBuilt(UnitGob *punt)
{
	// Scan the list of desired Units and slot in this new one.
	// As we scan keep track of whether this group has all the
	// units it needs yet.

	bool fNeedsUnit = false;
	UnitType ut = punt->GetUnitType();
	UnitListEntry *pule = m_aule;
	for (int iule = 0; iule < m_cule; iule++, pule++) {
		if (pule->bf & kfUleBuilt)
			continue;
		if (pule->ut != ut) {
			fNeedsUnit = true;
			continue;
		}

		// This UnitGroup wants this unit type. Retain it and set any special
		// flags it requires.

		pule->gid = punt->GetId();
		pule->bf |= kfUleBuilt;
		if (punt->GetFlags() & kfGobMobileUnit) {
			MobileUnitGob *pmunt = (MobileUnitGob *)punt;
			word wf = (pmunt->GetMobileUnitFlags() & ~kfMuntAggressivenessBits) | m_wfMunt;

			// Human-controlled units don't attack while following move orders

			if (!(m_pplr->GetFlags() & kfPlrComputer))
				wf &= ~kfMuntAttackEnemiesWhenMoving;
			pmunt->SetMobileUnitFlags(wf);
		}
		punt->SetHealth((fix)(((long)m_nHealth * punt->GetConsts()->GetArmorStrength()) / 100));
		
		// Keep scanning so fNeedsUnit will be determined but set ut = kutNone
		// so no more matches will be found.

		ut = kutNone;
	}

	if (!fNeedsUnit) 
		m_wf &= ~kfUgNeedsUnit;
}

void UnitGroup::AddUnit(UnitGob *punt, bool fReplicant)
{
	UnitListEntry *auleNew = new UnitListEntry[m_cule + 1];
	Assert(auleNew != NULL, "out of memory!");
	if (auleNew == NULL)
		return;

	memcpy(auleNew, m_aule, sizeof(UnitListEntry) * m_cule);
	delete m_aule;
	m_aule = auleNew;
	m_aule[m_cule].gid = punt->GetId();
	m_aule[m_cule].ut = punt->GetConsts()->ut;
	m_aule[m_cule].bf = fReplicant ? kfUleBuilt | kfUleReplicant : kfUleBuilt;
	m_cule++;
}

void UnitGroup::RemoveReplicants()
{
	UnitListEntry *pule = m_aule;
	for (int iule = 0; iule < m_cule; iule++, pule++) {

		// We require that constant unit list entries and Replicant unit list 
		// entries are not intermingled

		if (pule->bf & kfUleReplicant) {
#ifdef DEBUG
			for (int i = iule; i < m_cule; i++, pule++)
				Assert(pule->bf & kfUleReplicant);
#endif
			m_cule = iule;
			return;
		}
	}
}

void UnitGroup::Activate()
{
	// Can't let an active group be reactivated
	Assert((m_wf & kfUgActive) == 0);

	m_wf = (m_wf | kfUgNeedsUnit | kfUgActive | kfUgActivatedBefore) & ~(kfUgWaitingToReplace | kfUgNotRecentlyActivated);
	m_pactnLast = NULL;

	// Make sure human control players don't auto-attack while following move orders

	word wfMuntFilter;
	if (!(m_pplr->GetFlags() & kfPlrComputer))
		wfMuntFilter = ~kfMuntAttackEnemiesWhenMoving;
	else
		wfMuntFilter = 0xffff;

	if (m_wf & kfUgSpawn) {

		// Spawn MobileUnits in the center of the Area, Stuctures at the area's upper-left

		TRect trc;
		ggobm.GetAreaRect(m_nSpawnArea, &trc);
		Point pt;
		trc.GetCenter(&pt);
		
		UnitListEntry *pule = m_aule;
		for (int iule = 0; iule < m_cule; iule++, pule++) {
			UnitGob *punt = NULL;
			UnitConsts *puntc = gapuntc[pule->ut];
			fix fxHealth = (fix)(((long)m_nHealth * puntc->GetArmorStrength()) / 100);
			if ((1UL << pule->ut) & kumStructures) {
				// Check to see if limits have been reached

				if (!ggobm.IsBelowLimit(knLimitStruct, m_pplr))
					continue;

				// Is the space required by the structure free?

				StructConsts *pstruc = (StructConsts *)puntc;
				if (gsim.GetLevel()->GetTerrainMap()->IsOccupied(trc.left, trc.top, pstruc->ctxReserve, pstruc->ctyReserve, kbfStructure | kbfMobileUnit))
					continue; // no
				punt = (UnitGob *)CreateGob(pstruc->gt);
				Assert(punt->GetFlags() & kfGobStructure);
				if (punt == NULL)
					continue;
				punt->Init(WcFromTc(trc.left), WcFromTc(trc.top), m_pplr, fxHealth, 0, NULL);
			} else {
				// Check to see if limits have been reached

				if (!ggobm.IsBelowLimit(knLimitMobileUnit, m_pplr))
					continue;
				punt = (UnitGob *)CreateGob(puntc->gt);
				if (punt == NULL)
					continue;
				WPoint wpt;
				FindNearestFreeTile(pt.x, pt.y, &wpt);
				punt->Init(wpt.wx, wpt.wy, m_pplr, fxHealth, 0, NULL);
				Assert(punt->GetFlags() & kfGobMobileUnit);
				MobileUnitGob *pmunt = (MobileUnitGob *)punt;
				pmunt->SetMobileUnitFlags((pmunt->GetMobileUnitFlags() & ~kfMuntAggressivenessBits) | (m_wfMunt & wfMuntFilter));
			}

			pule->gid = punt->GetId();
			pule->bf |= kfUleBuilt;
		}
		m_wf &= ~kfUgNeedsUnit;

	} else {
		BuildMgr *pbldm = gsim.GetBuildMgr();
		UnitListEntry *pule = m_aule;
		for (int iule = 0; iule < m_cule; iule++, pule++) {
			pule->gid = kgidNull;
			pule->bf &= ~kfUleBuilt;
			pbldm->BuildUnit(pule->ut, this, m_nSpawnArea);
		}
	}

	Update();
}

void UnitGroup::Update()
{
	// Clear out dead units and if they're all dead deactivate or recreate the group.
	// We only count active units except with miners, we'll count inactive ones
	// because they're probably in the processor.

	bool fAllGone = true;
	UnitListEntry *pule = m_aule;
	for (int iule = 0; iule < m_cule; iule++, pule++) {
		if (pule->gid != kgidNull) {
			if (ggobm.GetGob(pule->gid, pule->ut != kutGalaxMiner) != NULL)
				fAllGone = false;
			else
				pule->gid = kgidNull;
		}
	}

	// Still needing a unit?

	if (m_wf & kfUgNeedsUnit)
		return;

	// If all Units are gone deactivate the group

	if (fAllGone) {
		m_wf &= ~kfUgActive;

		// Reset all actions to prepare for reexecution if/when this
		// UnitGroup is recreated.

		UnitGroupAction *pactn = m_pactn;
		while (pactn != NULL) {
			pactn->Reset();
			pactn = pactn->m_pactnNext;
		}

		// Clean out Replicants

		RemoveReplicants();

		// Reactivate groups that want it

		if (m_wf & kfUgReplaceGroup)
			Activate();
		return;
	}

	if (m_wf & kfUgWaitingToReplace) {
		// kfUgReplaceGroups are kept around after they finish executing
		// all their actions so they can be recreated when all their members
		// are destroyed.

		return;
	}

	// Is this UnitGroup still executing an action?

	UnitGroupAction *pactnNext = m_pactnLast;

	do {
		// Start executing actions. If pactnNext != NULL, start with that action

		if (pactnNext == NULL)
			pactnNext = m_pactn;

		while (pactnNext != NULL) {
			// If this action doesn't complete, remember that and return

			if (!pactnNext->Perform(this)) {
				m_pactnLast = pactnNext;
				return;
			}

			// Action complete, continue to next action

			m_pactnLast = NULL;	// clear out 'in-progress' action
			pactnNext = pactnNext->m_pactnNext;
		}
	} while (m_wf & kfUgLoopForever);

	// All actions done, deactivate the group unless we have to keep it around
	// for "Recreate if destroyed", i.e., it is a kfUgReplaceGroup

	if (m_wf & kfUgReplaceGroup)
		m_wf |= kfUgWaitingToReplace;
	else {
		RemoveReplicants();
		m_wf &= ~kfUgActive;
	}
}

bool UnitGroup::LoadAction(IniReader *pini, FindProp *pfind)
{
	char sz[128];
	sz[0] = 0;
	int nAction;
	int cArgs = pini->GetPropertyValue(pfind, "%d,%s", &nAction, sz);
	if (cArgs == 0)
		return false;
	Assert(strlen(sz) + 1 < sizeof(sz));

	UnitGroupAction *pactn;
	switch (nAction) {
	case knWaitUnitGroupAction:
		pactn = new WaitUnitGroupAction();
		break;

	case knSetSwitchUnitGroupAction:
		pactn = new SetSwitchUnitGroupAction();
		break;

	case knMoveUnitGroupAction:
		pactn = new MoveUnitGroupAction();
		break;

	case knAttackUnitGroupAction:
		pactn = new AttackUnitGroupAction();
		break;

	case knGuardUnitGroupAction:
		pactn = new GuardUnitGroupAction();
		break;

	case knGuardVicinityUnitGroupAction:
		pactn = new GuardVicinityUnitGroupAction();
		break;

	case knMineUnitGroupAction:
		pactn = new MineUnitGroupAction();
		break;

	default:
		Assert(false);
		break;
	}

	// Init it, error if that failed

	Assert(pactn != NULL, "out of memory!");
	if (!pactn->Init(sz)) {
		delete pactn;
		return false;
	}

	// Link it in last

	UnitGroupAction **ppactn = &m_pactn;
	while ((*ppactn) != NULL)
		ppactn = &((*ppactn)->m_pactnNext);
	*ppactn = pactn;

	return true;
}

UnitGroupAction::UnitGroupAction()
{
	m_pactnNext = NULL;
}

bool UnitGroupAction::LoadState(Stream *pstm)
{
	return true;
}

bool UnitGroupAction::SaveState(Stream *pstm)
{
	return true;
}

// Placeholder to be overridden

void UnitGroupAction::Reset()
{
}

// WaitUnitGroupAction

WaitUnitGroupAction::WaitUnitGroupAction()
{
	Reset();
}

#define knVerWaitUnitGroupActionState 1
bool WaitUnitGroupAction::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerWaitUnitGroupActionState)
		return false;

	m_fWaiting = pstm->ReadByte() != 0;
	m_tStart = pstm->ReadDword();
	
	return pstm->IsSuccess();
}

bool WaitUnitGroupAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerWaitUnitGroupActionState);
	pstm->WriteByte(m_fWaiting);
	pstm->WriteDword(m_tStart);

	return pstm->IsSuccess();
}

bool WaitUnitGroupAction::Init(char *psz)
{
	int cSecs;
	if (!ParseNumber(&psz, &cSecs))
		return false;
	m_ctWait = cSecs * 100;
	return true;
}

void WaitUnitGroupAction::Reset()
{
	m_fWaiting = false;
}

bool WaitUnitGroupAction::Perform(UnitGroup *pug)
{
	// If in the middle of waiting, check if the wait is over.

	long t = gsim.GetTickCount();
	if (m_fWaiting) {
		// Wait over? If so, return true

		if (t - m_tStart >= m_ctWait) {
			m_fWaiting = false;
			return true;
		}

		// Wait not over, return false

		return false;
	}

	// Starting the wait, assume wait not over

	m_fWaiting = true;
	m_tStart = t;
	return false;
}

#ifdef DEBUG_HELPERS
char *WaitUnitGroupAction::ToString() 
{
	sprintf(s_szDebugHelpers, "Wait %d [%d]", m_ctWait / 100, (gsim.GetTickCount() - m_tStart) / 100);
	return s_szDebugHelpers;
}
#endif

// SetSwitchUnitGroupAction

bool SetSwitchUnitGroupAction::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_iSwitch))
		return false;
	Assert(m_iSwitch < kcSwitchMax);

	int nOnOff;
	if (!ParseNumber(&psz, &nOnOff))
		return false;
	m_fOn = nOnOff == 1;
	return true;
}

bool SetSwitchUnitGroupAction::Perform(UnitGroup *pug)
{
	gsim.GetLevel()->GetTriggerMgr()->SetSwitch(m_iSwitch, m_fOn);
	return true;
}

// MoveUnitGroupAction
//
// UnitGroup Move <Area> action:
// All members of the group head to the same point.
//
// 1. Send each member of the group to the center of the specified Area
// UNDONE: 1a. if a member decides to go on attack, all members should attack the same target
// 2. Wait until all (living) members of the group are idle (no commands pending and in kstGuard)
// 3. Action is complete
//
// How units behave while moving (e.g., movement rate, whether they attack 
// nearby enemies) is up to them but influenced by the group's aggressiveness 
// level.

#define knVerMoveUnitGroupActionState 1
bool MoveUnitGroupAction::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerMoveUnitGroupActionState)
		return false;

	m_fWaiting = pstm->ReadByte() != 0;
	
	return pstm->IsSuccess();
}

bool MoveUnitGroupAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerMoveUnitGroupActionState);
	pstm->WriteByte(m_fWaiting);

	return pstm->IsSuccess();
}

bool MoveUnitGroupAction::Init(char *psz)
{
	Reset();
	return ParseArea(&psz, &m_nArea);
}

void MoveUnitGroupAction::Reset()
{
	m_fWaiting = false;
}

bool MoveUnitGroupAction::Perform(UnitGroup *pug)
{
	// First time through

	if (!m_fWaiting) {
		TRect trc;
		Level *plvl = gsim.GetLevel();
		ggobm.GetAreaRect(m_nArea, &trc, pug->GetOwner()->GetSide());

		// Figure out min speed

		UnitListEntry *pule = pug->GetUnitList();
		WCoord wcMoveDistPerUpdate = kwcMax;
		int iule;
		for (iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {
			// Ignore dead units

			UnitGob *punt = (UnitGob *)ggobm.GetGob(pule->gid);
			if (punt == NULL)
				continue;
			Assert((punt->GetFlags() & kfGobMobileUnit) != 0);
			MobileUnitGob *pmunt = (MobileUnitGob *)punt;

			// Get min speed per update

			MobileUnitConsts *pmuntc = (MobileUnitConsts *)pmunt->GetConsts();
			if (pmuntc->GetMoveDistPerUpdate() < wcMoveDistPerUpdate)
				wcMoveDistPerUpdate = pmuntc->GetMoveDistPerUpdate();
		}

		// Send commands

		TCoord tcRadius = RadiusFromUnitCount(pug->GetUnitCount()) + 1;
		pule = pug->GetUnitList();
		for (iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

			// Ignore dead units

			UnitGob *punt = (UnitGob *)ggobm.GetGob(pule->gid);
			if (punt == NULL)
				continue;
			Assert((punt->GetFlags() & kfGobMobileUnit) != 0);
			MobileUnitGob *pmunt = (MobileUnitGob *)punt;

			// Tell it where to go.

			Point ptCenter;
			trc.GetCenter(&ptCenter);
			SendMoveAction(pmunt->GetId(), WcFromTc(ptCenter.x), WcFromTc(ptCenter.y), tcRadius, wcMoveDistPerUpdate);
		}

		m_fWaiting = true;
		return false;

	} else {
		m_fWaiting = false;
		UnitListEntry *pule = pug->GetUnitList();
		for (int iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

			// Ignore dead units

			UnitGob *punt = (UnitGob *)ggobm.GetGob(pule->gid);
			if (punt == NULL)
				continue;

			// Is it trying?

			if (!punt->IsIdle()) {
				m_fWaiting = true;
				break;
			}
		}
	}

	return !m_fWaiting;
}

#ifdef DEBUG_HELPERS
char *MoveUnitGroupAction::ToString() 
{
	sprintf(s_szDebugHelpers, "Move to \"%s\"", ggobm.GetAreaName(m_nArea));
	return s_szDebugHelpers;
}
#endif

// UnitGroup Attack <UnitMask> for <seconds> action:
// Each member of the group attacks the nearest enemy Unit that matches
// the UnitMask. When the enemy is destroyed a new target is selected and
// attacked. The action is complete when <seconds> have elapsed.
// 
// 1. Select a target
// 2. Send each Unit an Attack Action against the target
// 3. When target is destroyed go to step #1
// UNDONE: 3a. What if there are no targets? Incorporate some sort of a pause to keep from scanning each update.
// 4. Action is complete when <seconds> have elapsed

#define knVerAttackUnitGroupActionState 1
bool AttackUnitGroupAction::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerAttackUnitGroupActionState)
		return false;

	m_fWaiting = pstm->ReadByte() != 0;
	m_tStart = pstm->ReadDword();
	m_gidTarget = pstm->ReadWord();
	
	return pstm->IsSuccess();
}

bool AttackUnitGroupAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerAttackUnitGroupActionState);
	pstm->WriteByte(m_fWaiting);
	pstm->WriteDword(m_tStart);
	pstm->WriteWord(m_gidTarget);

	return pstm->IsSuccess();
}

bool AttackUnitGroupAction::Init(char *psz)
{
	Reset();

	if (!ParseUnitMask(&psz, &m_um))
		return false;
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	int cSecs;
	if (!ParseNumber(&psz, &cSecs))
		return false;
	m_ctWait = cSecs * 100;
	return true;
}

void AttackUnitGroupAction::Reset()
{
	m_fWaiting = false;
}

bool AttackUnitGroupAction::Perform(UnitGroup *pug)
{
	// If in the middle of waiting, check if the wait is over.

	long t = gsim.GetTickCount();
	if (m_fWaiting) {
		// Wait over? If so, return true

		if (t - m_tStart >= m_ctWait) {
			m_fWaiting = false;
			return true;
		}
	} else {

		// Starting the action. Reset the wait start.

		m_fWaiting = true;
		m_tStart = t;
		m_gidTarget = kgidNull;
	}

	// Is a target needed? If so, find one and direct everyone to attack it.

	SideMask sidmTarget = GetSideMaskFromCaSideMask(pug->GetOwner()->GetSide(), m_wfCaSideMask);
	UnitGob *puntTarget = (UnitGob *)ggobm.GetGob(m_gidTarget);
	if (puntTarget != NULL) {

		// Possibly the Unit has been taken over and is not owned by a side
		// we are attacking any more.

		if (!(GetSideMask(puntTarget->GetSide()) & sidmTarget))
			puntTarget = NULL;
	}

	// We have a target and everyone should already be attacking it so keep waiting.

	if (puntTarget != NULL)
		return false;

	// Find a new target

	m_gidTarget = kgidNull;

	// Find the nearest Unit that matches the target SideMask and UnitMask

	// Calc group centroid

	UnitListEntry *pule = pug->GetUnitList();
	TCoord txGroupCenter = 0, tyGroupCenter = 0;
	int cpunt = 0;
	int iule;
	for (iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

		// Ignore dead units

		UnitGob *punt = (UnitGob *)ggobm.GetGob(pule->gid);
		if (punt == NULL)
			continue;

		TPoint tpt;
		punt->GetTilePosition(&tpt);
		txGroupCenter += tpt.tx;
		tyGroupCenter += tpt.ty;
		cpunt++;
	}
	txGroupCenter /= cpunt;
	tyGroupCenter /= cpunt;

	word nDist = 0x7fff;
	puntTarget = (UnitGob *)ggobm.GetFirstGob();
	for (; puntTarget != NULL; puntTarget = (UnitGob *)ggobm.GetNextGob(puntTarget)) {
		dword ff = puntTarget->GetFlags();
		if ((ff & (kfGobUnit | kfGobActive)) != (kfGobUnit | kfGobActive))
			continue;

		if (!(puntTarget->GetConsts()->um & m_um))
			continue;

		if (!(GetSideMask(puntTarget->GetSide()) & sidmTarget))
			continue;

		// Compute distance to this potential target.

		TPoint tpt;
		puntTarget->GetTilePosition(&tpt);
		TCoord dtx = tpt.tx - txGroupCenter;
		TCoord dty = tpt.ty - tyGroupCenter;
		word nDistT = dtx * dtx + dty * dty;	// should not exceed 8192 for 64x64 maps
		Assert(nDistT < 8192);

		// Keep track of the nearest target.

		if (nDistT < nDist) {
			m_gidTarget = puntTarget->GetId();
			nDist = nDistT;
		}
	}

	// No valid targets found?

	if (m_gidTarget == kgidNull) {
		// UNDONE: wait for awhile
		return false;
	}

	puntTarget = (UnitGob *)ggobm.GetGob(m_gidTarget);

	// Have everyone attack it

	Message msgT;
	msgT.smidSender = ksmidNull;
	puntTarget->GetAttackPoint(&msgT.AttackCommand.wptTarget);
	msgT.AttackCommand.gidTarget = m_gidTarget;
	msgT.AttackCommand.wptTargetCenter = msgT.AttackCommand.wptTarget;
	msgT.AttackCommand.tcTargetRadius = 0;
	msgT.AttackCommand.wcMoveDistPerUpdate = 0;

	pule = pug->GetUnitList();
	for (iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

		// Ignore dead units

		MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(pule->gid);
		if (pmunt == NULL)
			continue;
		Assert((pmunt->GetFlags() & kfGobMobileUnit) != 0);

		// Tell it to attack the fresh target

		// The MobileUnitGob's state machine slams this member of the message 
		// so we must reinitialize it.

		msgT.mid = kmidAttackAction;

		msgT.smidReceiver = pmunt->GetId();
		gsmm.SendMsg(&msgT);
	}
	
	// Keep 'waiting' (until all group members are dead or the attack time runs out).

	return false;
}

#ifdef DEBUG_HELPERS
char *AttackUnitGroupAction::ToString() 
{
	sprintf(s_szDebugHelpers, "Attack %s for %d [%d]", PszFromUnitMask(m_um), m_ctWait / 100, (gsim.GetTickCount() - m_tStart) / 100);
	return s_szDebugHelpers;
}
#endif

// UnitGroup Guard action:
// All members of the group stand still and guard as per the group's 
// aggressiveness level.
//
// 1. Send a Guard command to each member of the group
// 2. Action is complete

#define knVerGuardUnitGroupActionState 1
bool GuardUnitGroupAction::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerGuardUnitGroupActionState)
		return false;

	m_fWaiting = pstm->ReadByte() != 0;
	m_tStart = pstm->ReadDword();
	
	return pstm->IsSuccess();
}

bool GuardUnitGroupAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerGuardUnitGroupActionState);
	pstm->WriteByte(m_fWaiting);
	pstm->WriteDword(m_tStart);

	return pstm->IsSuccess();
}

bool GuardUnitGroupAction::Init(char *psz)
{
	Reset();

	int cSecs;
	if (!ParseNumber(&psz, &cSecs))
		return false;
	m_ctWait = cSecs * 100;
	return true;
}

void GuardUnitGroupAction::Reset()
{
	m_fWaiting = false;
}

bool GuardUnitGroupAction::Perform(UnitGroup *pug)
{
	// If in the middle of waiting, check if the wait is over.

	long t = gsim.GetTickCount();
	if (m_fWaiting) {
		// Wait over? If so, return true

		if (t - m_tStart >= m_ctWait) {
			m_fWaiting = false;
			return true;
		}
	} else {

		// Starting the action. Reset the wait start.

		m_fWaiting = true;
		m_tStart = t;

		// Have everyone Guard

		Message msgT;
		msgT.smidSender = ksmidNull;

		UnitListEntry *pule = pug->GetUnitList();
		for (int iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

			// Ignore dead units

			MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(pule->gid);
			if (pmunt == NULL)
				continue;
			Assert((pmunt->GetFlags() & kfGobMobileUnit) != 0);

			// Tell it to Guard

			// The MobileUnitGob's state machine slams this member of the message 
			// so we must reinitialize it.

			msgT.mid = kmidGuardAction;

			msgT.smidReceiver = pmunt->GetId();
			gsmm.SendMsg(&msgT);
		}
	}

	// Keep 'waiting' (until all group members are dead or the Guard time runs out).

	return false;
}

// UnitGroup GuardArea <Area> action:
// All members of the group stand still and guard the area as per the group's
// aggressiveness level. If an enemy unit enters the area the group will go
// after it. All members of the group go after the same target.
//
// 1. Send a GuardArea command to each member of the group
// 2. Action is complete

// UNDONE:


// MineUnitGroupAction

bool MineUnitGroupAction::Init(char *psz)
{
	return true;
}

bool MineUnitGroupAction::Perform(UnitGroup *pug)
{
	UnitListEntry *pule = pug->GetUnitList();
	for (int iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

		// Ignore dead units

		MinerGob *pmnr = (MinerGob *)ggobm.GetGob(pule->gid);
		if (pmnr == NULL)
			continue;
		if (pmnr->GetType() != kgtGalaxMiner)
			continue;

		SendMineCommand(pmnr->GetId(), kwxInvalid, kwxInvalid);
	}

	// No waiting

	return true;
}

// UnitGroup GuardViciniyt action:
// All members of the group stand still and guard as per the group's 
// aggressiveness level.
//
// 1. Send a Guard command to each member of the group
// 2. Action is complete

#define knVerGuardVicinityUnitGroupActionState 1
bool GuardVicinityUnitGroupAction::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerGuardVicinityUnitGroupActionState)
		return false;

	m_fWaiting = pstm->ReadByte() != 0;
	m_tStart = pstm->ReadDword();
	
	return pstm->IsSuccess();
}

bool GuardVicinityUnitGroupAction::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerGuardVicinityUnitGroupActionState);
	pstm->WriteByte(m_fWaiting);
	pstm->WriteDword(m_tStart);

	return pstm->IsSuccess();
}

bool GuardVicinityUnitGroupAction::Init(char *psz)
{
	Reset();

	int cSecs;
	if (!ParseNumber(&psz, &cSecs))
		return false;
	m_ctWait = cSecs * 100;
	return true;
}

void GuardVicinityUnitGroupAction::Reset()
{
	m_fWaiting = false;
}

bool GuardVicinityUnitGroupAction::Perform(UnitGroup *pug)
{
	// If in the middle of waiting, check if the wait is over.

	long t = gsim.GetTickCount();
	if (m_fWaiting) {
		// Wait over? If so, return true

		if (t - m_tStart >= m_ctWait) {
			m_fWaiting = false;
			return true;
		}
	} else {

		// Starting the action. Reset the wait start.

		m_fWaiting = true;
		m_tStart = t;

		// Have everyone Guard

		Message msgT;
		msgT.smidSender = ksmidNull;

		UnitListEntry *pule = pug->GetUnitList();
		for (int iule = 0; iule < pug->GetUnitCount(); iule++, pule++) {

			// Ignore dead units

			MobileUnitGob *pmunt = (MobileUnitGob *)ggobm.GetGob(pule->gid);
			if (pmunt == NULL)
				continue;
			Assert((pmunt->GetFlags() & kfGobMobileUnit) != 0);

			// Tell it to Guard

			// The MobileUnitGob's state machine slams this member of the message 
			// so we must reinitialize it.

			msgT.mid = kmidGuardVicinityAction;

			msgT.smidReceiver = pmunt->GetId();
			gsmm.SendMsg(&msgT);
		}
	}

	// Keep 'waiting' (until all group members are dead or the Guard time runs out).

	return false;
}

} // namespace wi