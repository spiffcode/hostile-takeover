#include "ht.h"

namespace wi {

bool Trigger::LoadCondition(IniReader *pini, FindProp *pfind)
{
	char sz[128];
	sz[0] = 0;
	int nCondition;
	int cArgs = pini->GetPropertyValue(pfind, "%d,%s", &nCondition, sz);
	if (cArgs == 0)
		return false;
	
	Condition *pcdn = NULL;
	switch (nCondition) {
	case knMissionLoadedCondition:
		pcdn = new MissionLoadedCondition();
		break;

	case knCreditsCondition:
		pcdn = new CreditsCondition();
		break;

	case knOwnsUnitsCondition:
		pcdn = new OwnsUnitsCondition();
		break;

	case knAreaContainsUnitsCondition:
		pcdn = new AreaContainsUnitsCondition();
		break;

	case knPlaceStructureModeCondition:
		pcdn = new PlaceStructureModeCondition();
		break;

	case knMinerCantFindGalaxiteCondition:
		pcdn = new MinerCantFindGalaxiteCondition();
		break;

	case knGalaxiteCapacityReachedCondition:
		pcdn = new GalaxiteCapacityReachedCondition();
		break;

	case knElapsedTimeCondition:
		pcdn = new ElapsedTimeCondition();
		break;

	case knSwitchCondition:
		pcdn = new SwitchCondition();
		break;

	case knPeriodicTimerCondition:
		pcdn = new PeriodicTimerCondition();
		break;

	case knDiscoversSideCondition:
		pcdn = new DiscoversSideCondition();
		break;

	case knCountdownTimerCondition:
		pcdn = new CountdownTimerCondition();
		break;

	case knTestPvarCondition:
		pcdn = new TestPvarCondition();
		break;

	case knHasUpgradesCondition:
		pcdn = new HasUpgradesCondition();
		break;

#ifdef UNDONE
	case knUnitDestroyedCondition:
		pcdn = new UnitDestroyedCondition();
		break;

	case knDeathsCondition:
		pcdn = new DeathsCondition();
		break;

	case knMobileHQDeployableCondition:
		pcdn = new MobileHQDeployableCondition();
		break;

	case knMobileHQDeployedCondition:
		pcdn = new MobileHQDeployedCondition();
		break;

	case knUnitSeesUnitCondition:
		pcdn = new UnitSeesUnitCondition();
		break;

	case knUnitDestroyedCondition:
		pcdn = new UnitDestroyedCondition();
		break;
#endif

	default:
		Assert(false);
		break;
	}
	
	// Init it, error if that failed

	Assert(pcdn != NULL, "out of memory!");
	if (!pcdn->Init(sz)) {
		delete pcdn;
		return false;
	}

	// Link it in last

	Condition **ppcdn = &m_pcdn;
	while ((*ppcdn) != NULL)
		ppcdn = &((*ppcdn)->m_pcdnNext);
	*ppcdn = pcdn;

	return true;
}

// Condition base class implementation

Condition::Condition()
{
	m_pcdnNext = NULL;
}

bool Condition::Init(char *psz)
{
	return true;
}

#ifdef DEBUG_HELPERS
bool Condition::SafeIsTrue(Side side)
{
	return IsTrue(side);
}
#endif

// MissionLoadedCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
MissionLoadedCondition::MissionLoadedCondition()
{
}

bool MissionLoadedCondition::IsTrue(Side side)
{
	return true;
}

// CreditsCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
CreditsCondition::CreditsCondition()
{
}

bool CreditsCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return m_qnum.Parse(&psz);
}

bool CreditsCondition::IsTrue(Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);

	// Add up a total credits for all the sides in question

	long cCredits = 0;
	for (int n = 0; n < cplrs; n++)
		cCredits += applr[n]->GetCredits();

	// Does the total credit count meet our threshold?

	return m_qnum.Compare(cCredits);
}

// OwnsUnitsCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
OwnsUnitsCondition::OwnsUnitsCondition()
{
}

bool OwnsUnitsCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	if (!m_qnum.Parse(&psz))
		return false;
	return ParseUnitMask(&psz, &m_um);
}

bool OwnsUnitsCondition::IsTrue(Side side)
{
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);

	// Add up the number of Units owned by these players

	int cunt = 0;
	for (int n = 0; n < cplrs; n++)
		cunt += applr[n]->GetUnitActiveCountFromMask(m_um);

	// Does the total Unit count meet our threshold?

	return m_qnum.Compare(cunt);
}

// AreaContainsUnitsCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
AreaContainsUnitsCondition::AreaContainsUnitsCondition()
{
	m_nVersionLevel = gsim.GetLevel()->GetVersion();
}

bool AreaContainsUnitsCondition::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_nArea))
		return false;
	if (!m_qnum.Parse(&psz))
		return false;
	if (!ParseUnitMask(&psz, &m_um))
		return false;
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return true;
}

bool AreaContainsUnitsCondition::IsTrue(Side side)
{
	// Find the Gobs in the Area.
	
	SideMask sidm = GetSideMaskFromCaSideMask(side, m_wfCaSideMask);

	// This logic has an error. It is here for backwards compatibility.
	// CheckUnitsInArea returns true if a desired unit type and a desired
	// side are in an area, but not if desired unit type is of desired side.

	if (m_nVersionLevel == 0) {
		switch (m_qnum.m_nNumber) {
		case 0:
			if (m_qnum.m_nQualifier == knQualifierExactly)
				return !ggobm.CheckUnitsInArea(m_nArea, sidm, m_um);
			break;

		case 1:
			if (m_qnum.m_nQualifier == knQualifierAtLeast)
				return ggobm.CheckUnitsInArea(m_nArea, sidm, m_um);
			break;
		}
	}

	// Count gobs in area

	Enum enm;
	int cgobsMatch = 0;
	while (ggobm.EnumGobsInArea(&enm, m_nArea, sidm, m_um) != NULL)
		cgobsMatch++;

	// Does the total Unit count meet our threshold?

	return m_qnum.Compare(cgobsMatch);
}

#ifdef DEBUG_HELPERS
char *AreaContainsUnitsCondition::ToString() 
{
	sprintf(s_szDebugHelpers, "area \"%s\" contains %s %s owned by %s", ggobm.GetAreaName(m_nArea), m_qnum.ToString(), PszFromUnitMask(m_um), PszFromCaSideMask(m_wfCaSideMask));
	return s_szDebugHelpers;
}
#endif

// PlaceStructureModeCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
PlaceStructureModeCondition::PlaceStructureModeCondition()
{
}

bool PlaceStructureModeCondition::IsTrue(Side side)
{
	return gpfrmPlace->GetOwner() != NULL;
}

// MinerCantFindGalaxiteCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
MinerCantFindGalaxiteCondition::MinerCantFindGalaxiteCondition()
{
}

bool MinerCantFindGalaxiteCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return true;
}

bool MinerCantFindGalaxiteCondition::IsTrue(Side side)
{
	// Determine the Sides we will be testing against

	SideMask sidmTest = GetSideMaskFromCaSideMask(side, m_wfCaSideMask);

	return gsim.GetLevel()->GetTriggerMgr()->IsConditionTrue(knMinerCantFindGalaxiteCondition, sidmTest);
}

// GalaxiteCapacityReachedCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
GalaxiteCapacityReachedCondition::GalaxiteCapacityReachedCondition()
{
}

bool GalaxiteCapacityReachedCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;
	return true;
}

bool GalaxiteCapacityReachedCondition::IsTrue(Side side)
{
	// Determine the Sides we will be testing against

	SideMask sidmTest = GetSideMaskFromCaSideMask(side, m_wfCaSideMask);

	return gsim.GetLevel()->GetTriggerMgr()->IsConditionTrue(knGalaxiteCapacityReachedCondition, sidmTest);
}

// ElapsedTimeCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
ElapsedTimeCondition::ElapsedTimeCondition()
{
}

bool ElapsedTimeCondition::Init(char *psz)
{
	return m_qnum.Parse(&psz);
}

bool ElapsedTimeCondition::IsTrue(Side side)
{
	return m_qnum.Compare(gsim.GetTickCount() / 100);
}

#ifdef DEBUG_HELPERS
char *ElapsedTimeCondition::ToString() 
{
	sprintf(s_szDebugHelpers, "elapsed time [%d] is %s", gsim.GetTickCount() / 100, m_qnum.ToString());
	return s_szDebugHelpers;
}
#endif

// SwitchCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
SwitchCondition::SwitchCondition()
{
}

bool SwitchCondition::Init(char *psz)
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

bool SwitchCondition::IsTrue(Side side)
{
	return gsim.GetLevel()->GetTriggerMgr()->GetSwitch(m_iSwitch) == m_fOn;
}

#ifdef DEBUG_HELPERS
char *SwitchCondition::ToString()
{
	sprintf(s_szDebugHelpers, "switch \"%s\" is %s", gsim.GetLevel()->GetTriggerMgr()->GetSwitchName(m_iSwitch), 
			m_fOn ? "on" : "off");
	return s_szDebugHelpers;
}
#endif

// PeriodicTimerCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
PeriodicTimerCondition::PeriodicTimerCondition()
{
}

bool PeriodicTimerCondition::Init(char *psz)
{
	int csec;
	if (!ParseNumber(&psz, &csec))
		return false;

	m_iTimer = gsim.GetLevel()->GetTriggerMgr()->AddPeriodicTimer(csec * 100);
	return true;
}

// Periodic timers only start counting the first time they are tested.
// This way they can sit 'below' other conditions and have deterministic
// firing times.

bool PeriodicTimerCondition::IsTrue(Side side)
{
	TriggerMgr *ptgrm = gsim.GetLevel()->GetTriggerMgr();
	ptgrm->StartPeriodicTimer(m_iTimer);
	return ptgrm->IsPeriodicTimerTriggered(m_iTimer);
}

#ifdef DEBUG_HELPERS
bool PeriodicTimerCondition::SafeIsTrue(Side side)
{
	return gsim.GetLevel()->GetTriggerMgr()->IsPeriodicTimerTriggered(m_iTimer);
}

char *PeriodicTimerCondition::ToString() 
{
	TriggerMgr *ptgrm = gsim.GetLevel()->GetTriggerMgr();
	long ctPeriod = ptgrm->GetTimerPeriod(m_iTimer);
	long ctCountdown = ptgrm->GetTimerCountdown(m_iTimer);
	if (ctCountdown == kctTimerNotStarted)
		ctCountdown = ctPeriod + 100;
	sprintf(s_szDebugHelpers, "every %d [%d] seconds", ctPeriod / 100, (ctPeriod - ctCountdown) / 100);
	return s_szDebugHelpers;
}
#endif

// CountdownTimerCondition

bool CountdownTimerCondition::Init(char *psz)
{
	return m_qnum.Parse(&psz);
}

bool CountdownTimerCondition::IsTrue(Side side)
{
	int csecs;
	bool fStarted;
	fStarted = gsim.GetLevel()->GetTriggerMgr()->GetCountdownTimer()->GetTimer(&csecs);
	if (fStarted)
		return m_qnum.Compare(csecs);
	return false;
}


#if 0
// UnitDestroyedCondition

bool UnitDestroyedCondition::Init(char *psz)
{
	if (!ParseNumber(&psz, &m_iUnitDestroyed))
		return false;
	Assert(m_iUnitDestroyed < kcUnitDestroyedMax);

	int nOnOff;
	if (!ParseNumber(&psz, &nOnOff))
		return false;
	m_fOn = nOnOff == 1;
	return true;
}

bool UnitDestroyedCondition::IsTrue(Side side)
{
	return gsim.GetLevel()->GetTriggerMgr()->GetUnitDestroyed(m_iUnitDestroyed) == m_fOn;
}
#endif

// DiscoversSideCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
DiscoversSideCondition::DiscoversSideCondition()
{
}

bool DiscoversSideCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMaskA = nCaSideMask;

	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMaskB = nCaSideMask;
	return true;
}

bool DiscoversSideCondition::IsTrue(Side side)
{
	SideMask sidmTotalDiscovered = 0;
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMaskA, applr);
	for (int n = 0; n < cplrs; n++)
		sidmTotalDiscovered |= applr[n]->GetDiscoveredSides();

	return (sidmTotalDiscovered & GetSideMaskFromCaSideMask(side, m_wfCaSideMaskB)) != 0;
}

// TestPvarCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
TestPvarCondition::TestPvarCondition()
{
}

bool TestPvarCondition::Init(char *psz)
{
	if (!ParseString(&psz, m_szName))
		return false;
	Assert(strlen(m_szName) < kcbPvarNameMax);
	return m_qnum.Parse(&psz);
}

bool TestPvarCondition::IsTrue(Side side)
{
	int nValue = 0;
	char szT[kcbPvarValueMax];
	if (ggame.GetVar(m_szName, szT, sizeof(szT)))
		nValue = atoi(szT);

	return m_qnum.Compare(nValue);
}

// HasUpgradesCondition

// REMOVE_SOMEDAY: this is here to keep m68k-gcc from generating a default 
// constructor in the .text section
HasUpgradesCondition::HasUpgradesCondition()
{
}

bool HasUpgradesCondition::Init(char *psz)
{
	int nCaSideMask;
	if (!ParseNumber(&psz, &nCaSideMask))
		return false;
	m_wfCaSideMask = nCaSideMask;

	if (!ParseUpgradeMask(&psz, &m_upgm))
		return false;
	return true;
}

bool HasUpgradesCondition::IsTrue(Side side)
{
	UpgradeMask upgm = 0;
	Player *applr[kcSides];
	int cplrs = GetPlayersListFromCaSideMask(side, m_wfCaSideMask, applr);
	for (int n = 0; n < cplrs; n++)
		upgm |= applr[n]->GetUpgradeMask();

	return (upgm & m_upgm) != 0;
}

} // namespace wi