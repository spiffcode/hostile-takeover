#include "ht.h"

namespace wi {

// Implementation Notes:
// - Trigger manager:
//		* loads & manages triggers
//		* has framework for trigger eval
//		* maintains per-side evaluation order.
// - Trigger
//		* holder for conditions and actions
//		* understands 'blocking' actions
//
// Triggers get evaluated in per-side order
// Individual triggers can have actions that are in mid-execution
//
// Specific conditions and actions are each typed out.

//
// TriggerMgr
//

TriggerMgr::TriggerMgr()
{
	m_ctgr = 0;
	m_atgr = NULL;
	memset(m_mpSide2nTrigger, 0xff, sizeof(m_mpSide2nTrigger));
	memset(m_asidmCondition, 0, sizeof(m_asidmCondition));
	memset(m_abSwitch, 0, sizeof(m_abSwitch));
	m_cTimers = 0;
	m_fEnabled = true;
}

TriggerMgr::~TriggerMgr()
{
	delete[] m_atgr;
	m_ctgr = 0;
}

bool TriggerMgr::Init(IniReader *pini)
{
	// Get trigger count

	int cArgs = pini->GetPropertyValue("Triggers", "Count", "%d", &m_ctgr);
	if (cArgs != 1)
		return false;

	// Allocate trigger storage, one chunk

	Assert(m_atgr == NULL);
	m_atgr = new Trigger[m_ctgr];
	Assert(m_atgr != NULL, "out of memory!");
	if (m_atgr == NULL)
		return false;

	// Skip the Count property

	char szProp[128];
	FindProp find;
	if (!pini->FindNextProperty(&find, "Triggers", szProp, sizeof(szProp)))
		return false;
#ifdef _DEBUG
	if (strcmp(szProp, "Count") != 0)
		return false;
#endif

	// Initialize the triggers

	for (int ntgr = 0; ntgr < m_ctgr; ntgr++) {
		// Next property name should be "T"

		if (!pini->FindNextProperty(&find, "Triggers", szProp, sizeof(szProp)))
			return false;
		if (strcmp(szProp, "T") != 0)
			return false;

		// Remember per-side assignments and indexes

		int cArgs = pini->GetPropertyValue(&find, "%s", &szProp);
		if (cArgs != 1)
			return false;
		if (!AssignTriggerSides(ntgr, szProp))
			return false;

		// Initialize this trigger

		if (!m_atgr[ntgr].Init(pini, &find))
			return false;
	}

	m_tLastUpdate = gsim.GetTickCount();

#ifdef DEBUG_HELPERS
	FindProp findSwitch;
	int isw = 0;
	while (pini->FindNextProperty(&findSwitch, "Switches", szProp, sizeof(szProp))) {
		strncpyz(m_aszSwitchNames[isw], szProp, sizeof(m_aszSwitchNames[0]));
		isw++;
		Assert(isw <= kcSwitchMax);
	}

	extern void UpdateTriggerViewer();
	UpdateTriggerViewer();
#endif
	return true;
}

#define knVerTriggerMgrState 2
bool TriggerMgr::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerTriggerMgrState)
		return false;

	// Read Triggers

	Trigger *ptgr = m_atgr;
	int i; // thank you WinCE compiler for this
	for (i = 0; i < this->m_ctgr; i++, ptgr++)
		ptgr->LoadState(pstm);

	// Read Switch state

	pstm->ReadBytesRLE(m_abSwitch, sizeof(m_abSwitch));

	// Read one-update conditions

	pstm->ReadBytesRLE((byte *)m_asidmCondition, sizeof(m_asidmCondition));

	// Read timer countdown values

	for (i = 0; i < m_cTimers; i++)
		m_actCountdown[i] = pstm->ReadDword();
	m_tLastUpdate = pstm->ReadDword();

	// load the game countdown timer

	m_cdt.LoadState(pstm);

	return pstm->IsSuccess();
}

bool TriggerMgr::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerTriggerMgrState);

	// Write Triggers

	Trigger *ptgr = m_atgr;
	int i; // Thank you WinCE compiler for this
	for (i = 0; i < this->m_ctgr; i++, ptgr++)
		ptgr->SaveState(pstm);

	// Write Switch state

	pstm->WriteBytesRLE(m_abSwitch, sizeof(m_abSwitch));

	// Write one-update conditions

	pstm->WriteBytesRLE((byte *)m_asidmCondition, sizeof(m_asidmCondition));

	// Write timer countdown values

	for (i = 0; i < m_cTimers; i++)
		pstm->WriteDword(m_actCountdown[i]);
	pstm->WriteDword(m_tLastUpdate);

	m_cdt.SaveState(pstm);

	return pstm->IsSuccess();
}

bool TriggerMgr::AssignTriggerSides(int ntgr, char *psz)
{
	Assert(ntgr >= 0 && ntgr < m_ctgr);

	while (true) {
		int side;
		int nIndex;
		int nch = 0;
		int cArgs = IniScanf(psz, "%d:%d,%+", &side, &nIndex, &nch);
		if (cArgs == 0)
			break;
		if (cArgs != 2 && cArgs != 3)
			return false;
		psz += nch;
		Assert(side >= ksideNeutral && side < kcSides);
		Assert(nIndex >= 0 && nIndex < kcTriggersPerSide);
		if (nIndex < kcTriggersPerSide)
			m_mpSide2nTrigger[side][nIndex] = (byte)ntgr;
		if (cArgs == 2)
			break;
	}

	return true;
}

void TriggerMgr::Update()
{
	if (!m_fEnabled)
		return;
	
	m_cdt.Update();

	// Update all the trigger timers

	long t = gsim.GetTickCount();
	long dt = t - m_tLastUpdate;
	m_tLastUpdate = t;

	long *pct = m_actCountdown;
	int i;	// thank you WinCE compiler for this
	for (i = 0; i < m_cTimers; i++)
		if (*pct != kctTimerNotStarted)
			*pct++ -= dt;

	// For each side...

	for (Side side = ksideNeutral; side < kcSides; side++) {
		// Execute per-side triggers in the per-side specified order 

		byte *pntgr = &m_mpSide2nTrigger[side][0];
		for (; *pntgr != 0xff; pntgr++) {
			Assert(*pntgr < m_ctgr);

            // Subsequent triggers may have been disabled by the prior trigger
            // (e.g., End Mission action)

			if (m_fEnabled)
				m_atgr[*pntgr].Execute(side);
		}
	}

	// Clear one-Update conditions

	memset(m_asidmCondition, 0, sizeof(m_asidmCondition));

	// Reset any underflowed timers

	pct = m_actCountdown;
	for (i = 0; i < m_cTimers; i++) {
		if (*pct <= 0 && *pct != kctTimerNotStarted)
			*pct += m_actPeriod[i];
		pct++;
	}

#ifdef DEBUG_HELPERS
	extern void UpdateTriggerViewer();
	UpdateTriggerViewer();
#endif
}

void TriggerMgr::SetConditionTrue(int nCondition, SideMask sidm)
{
	m_asidmCondition[nCondition] |= sidm;
}

bool TriggerMgr::IsConditionTrue(int nCondition, SideMask sidm)
{
	return (m_asidmCondition[nCondition] & sidm) != 0;
}

//
// Trigger
//

Trigger::Trigger()
{
	m_pcdn = NULL;
	m_pactn = NULL;
	memset(m_apactnLast, 0, sizeof(m_apactnLast));
	memset(m_afArmed, 1, sizeof(m_afArmed));
}

Trigger::~Trigger()
{
	// Delete Conditions

	Condition *pcdn = m_pcdn;
	while (pcdn != NULL) {
		Condition *pcdnNext = pcdn->m_pcdnNext;
		delete pcdn;
		pcdn = pcdnNext;
	}

	// Delete Actions

	TriggerAction *pactn = m_pactn;
	while (pactn != NULL) {
		TriggerAction *pactnNext = pactn->m_pactnNext;
		delete pactn;
		pactn = pactnNext;
	}
}

#define knVerTriggerState 1
bool Trigger::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerTriggerState)
		return false;

	// Read the last (in-progress) action (if any) for each side

	for (int i = 0; i < kcSides; i++) {
		char nAction = pstm->ReadByte();
		if (nAction != -1) {
			TriggerAction *pactn = m_pactn;
			for (int j = 0; j < nAction; j++, pactn = pactn->m_pactnNext);
			m_apactnLast[i] = pactn;
		}
	}

	// Read the per-side armed flags

	pstm->ReadBytesRLE((byte *)m_afArmed, sizeof(m_afArmed));

	// Give each Action a chance to read its state

	for (TriggerAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext)
		pactn->LoadState(pstm);

	return pstm->IsSuccess();
}

bool Trigger::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerTriggerState);

	// Map m_apactnLast into an index or -1 for no last action

	for (int i = 0; i < kcSides; i++) {
		if (m_apactnLast[i] == NULL) {
			pstm->WriteByte((byte)-1);
		} else {
			char nAction = 0;
			for (TriggerAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext, nAction++)
				if (pactn == m_apactnLast[i])
					break;
			pstm->WriteByte(nAction);
		}
	}
			
	// Write the per-side armed flags

	pstm->WriteBytesRLE((byte *)m_afArmed, sizeof(m_afArmed));

	// Give each Action a chance to write its state

	for (TriggerAction *pactn = m_pactn; pactn != NULL; pactn = pactn->m_pactnNext)
		pactn->SaveState(pstm);

	return pstm->IsSuccess();
}

bool Trigger::Init(IniReader *pini, FindProp *pfind)
{
	// Property names
	// 'T' means start of new trigger
	// 'C' means condition
	// 'A' means action

	char szProp[16];
	FindProp findLast;
	findLast.Assign(pfind);
	while (pini->FindNextProperty(pfind, "Triggers", szProp, sizeof(szProp))) {
		switch (szProp[0]) {
		case 'T':
			// Return the last PropFind since TriggerMgr wants to find a 'T' next

			pfind->Assign(&findLast);
			return true;

		case 'C':
			if (!LoadCondition(pini, pfind))
				return false;
			break;

		case 'A':
			if (!LoadAction(pini, pfind))
				return false;
			break;

		default:
			Assert(false);
			break;
		}

		findLast.Assign(pfind);
	}

	return true;
}

// Execute will either resume at a blocking action or check conditions for true-ness and then
// perform actions. An action just needs to return false to "block" and be resumed later.

void Trigger::Execute(Side side, bool fForce)
{
	// Is this trigger still executing an action for this side?

	TriggerAction *pactnNext = m_apactnLast[side];
	if (pactnNext == NULL) {
		// If not armed, then this trigger no longer executes for this side

		if (!fForce) {
			if (!m_afArmed[side])
				return;

			// It's armed so we check conditions. Return if any condition is not true.

			for (Condition *pcdn = m_pcdn; pcdn != NULL; pcdn = pcdn->m_pcdnNext) {
				if (!pcdn->IsTrue(side))
					return;
			}
		}

		// The conditions are met so we're going to perform the actions. Disarm the
		// trigger so its actions only happen once, unless a PreserveTriggerAction 
		// is used to rearm it.

		m_afArmed[side] = false;
	}

	// Start executing actions. If pactnNext != NULL, start with that action

	if (pactnNext == NULL)
		pactnNext = m_pactn;

	while (pactnNext != NULL) {
		// If this action doesn't complete, remember that and return

		m_apactnLast[side] = pactnNext;

#ifdef DEBUG_HELPERS
		extern void UpdateTriggerViewer();
		UpdateTriggerViewer();
#endif
		if (!pactnNext->Perform(this, side))
			return;

		// Action complete, continue to next action

		m_apactnLast[side] = NULL;	// clear out 'in-progress' action
		pactnNext = pactnNext->m_pactnNext;
	}
}

void Trigger::SetCurrentActionComplete(Side side)
{
	m_apactnLast[side] = NULL;	// clear out 'in-progress' action
}

void Trigger::Arm(Side side)
{
	m_afArmed[side] = true;
}

//
// Condition/Action parameter types
//

bool QualifiedNumber::Parse(char **ppsz) 
{
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%d,%ld,%+", &m_nQualifier, &m_nNumber, &nch);
	*ppsz += nch;
	return cArgs >= 2;
}

bool QualifiedNumber::Compare(long nNumber) 
{
	switch (m_nQualifier) {
	case knQualifierAtLeast:
		return nNumber >= m_nNumber;

	case knQualifierAtMost:
		return nNumber <= m_nNumber;

	case knQualifierExactly:
		return nNumber == m_nNumber;
	}

	return false;
}

// countdown timer class

CountdownTimer::CountdownTimer()
{
	m_szFormat[0] = 0;
	m_secs = 0;
	m_tLast = 0;
	m_wf = 0;
}

#define knVerCountdownTimerState 1
bool CountdownTimer::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerCountdownTimerState)
		return false;

	m_secs = (short)pstm->ReadWord();
	m_wf = pstm->ReadWord();
	m_tLast = pstm->ReadDword();
	pstm->ReadString(m_szFormat, sizeof(m_szFormat));

	return pstm->IsSuccess();
}

bool CountdownTimer::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerCountdownTimerState);
	pstm->WriteWord(m_secs);

	word wf = ggame.GetSimUIForm()->GetControlPtr(kidcCountdown)->GetFlags();
	m_wf = (wf & kfCtlVisible) ? (m_wf | kfCtVisibleAtStart) : (m_wf & ~kfCtVisibleAtStart);

	pstm->WriteWord(m_wf);
	pstm->WriteDword(m_tLast);
	pstm->WriteString(m_szFormat);


	return pstm->IsSuccess();
}

void CountdownTimer::SetTimer(int csecs, char *pszFormatString)
{
	m_secs = csecs;
	strncpyz(m_szFormat, pszFormatString, sizeof(m_szFormat));
}

void CountdownTimer::StartTimer(bool fStart)
{
	m_wf = (m_wf & ~kfCtRunning);
	if (fStart) {
		m_wf |= kfCtRunning;
		m_tLast = gsim.GetTickCount();
		UpdateString();
	}
}

void CountdownTimer::ShowTimer(bool fShow)
{
	ggame.GetSimUIForm()->GetControlPtr(kidcCountdown)->Show(fShow);
}

bool CountdownTimer::GetTimer(int *psecs)
{
	 *psecs = m_secs;
	 return (m_wf & kfCtRunning) != 0;
}

void CountdownTimer::Update()
{
	if (m_wf & kfCtRunning && (m_secs > 0))
	{
		long t = gsim.GetTickCount();
		long dt = t - m_tLast;
		if (dt > 100)	// greater than 1 second
		{
			m_secs -= 1;
			m_tLast += 100;

			UpdateString();
		}
	}
}

void CountdownTimer::UpdateString()
{
	char szT[50];
	char szTemp[15];

	// bummer! Palm does not support this format string
	//sprintf(szTemp, "%.2d:%.2d", m_secsCountdown / 60, m_secsCountdown % 60);

	int cMins = m_secs / 60;
	int cSecs = m_secs % 60;
	int cBytes = sprintf(szTemp, "%s%d:%s%d", cMins > 9 ? "" : "0", cMins, cSecs > 9 ? "" : "0", cSecs);

	sprintf(szT, m_szFormat, szTemp);
	LabelControl *plbl = (LabelControl *)ggame.GetSimUIForm()->GetControlPtr(kidcCountdown);
	plbl->SetText(szT);
}

//
// Parsers, helpers & interesting datatypes
// UNDONE: a scanf-like general parser to make condition/action parsers super easy to write
// E.g. for AreaContainsUnitsCondition: 
// CaParse("dqud", &m_nArea, &m_qnum, &m_um, &m_nCaSideMask)
//

SideMask GetSideMaskFromCaSideMask(Side sideCur, word wfCaSideMask)
{
	Assert(knCaSideSide1 == kside1 && knCaSideSide2 == kside2 && knCaSideSide3 == kside3 && knCaSideSide4 == kside4);

	// TODO: this approach is expensive at run-time

	SideMask sidm = wfCaSideMask & ksidmAll;
	if (wfCaSideMask & (1 << knCaSideAllSides)) {
		sidm |= ksidmAll;
	} else {
		if (wfCaSideMask & (1 << knCaSideAllies)) {
			Player *pplr = gplrm.GetPlayer(sideCur);
			sidm |= pplr->GetAllies();
		}
		if (wfCaSideMask & (1 << knCaSideEnemies)) {
			Player *pplr = gplrm.GetPlayer(sideCur);
			sidm |= ksidmAll & ~pplr->GetAllies();
		}
		if (wfCaSideMask & (1 << knCaSideCurrentSide))
			sidm |= GetSideMask(sideCur);
	}

	return sidm;
}

int GetPlayersListFromCaSideMask(Side sideCur, word wfMask, Player **applr)
{
	SideMask sidm = GetSideMaskFromCaSideMask(sideCur, wfMask);

	Player *pplr = NULL;
	int i = 0;
	while (true) {
		pplr = gplrm.GetNextPlayer(pplr);
		if (pplr == NULL)
			break;
		if (sidm & GetSideMask(pplr->GetSide()))
			applr[i++] = pplr;
	}

	return i;
}

bool ParseNumber(char **ppsz, int *pn)
{
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%d,%+", pn, &nch);

	// If this is the last of a comma separated list of items set the
	// string pointer to point to the zero-terminator so subsequent
	// calls will fail.

	if (cArgs == 1) {
		*ppsz += strlen(*ppsz);
	} else {
		*ppsz += nch;
	}
	return cArgs >= 1;
}

bool ParseLong(char **ppsz, long *pn)
{
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%ld,%+", pn, &nch);

	// If this is the last of a comma separated list of items set the
	// string pointer to point to the zero-terminator so subsequent
	// calls will fail.

	if (cArgs == 1) {
		*ppsz += strlen(*ppsz);
	} else {
		*ppsz += nch;
	}
	return cArgs >= 1;
}

bool ParseArea(char **ppsz, int *pn)
{
	if (!ParseNumber(ppsz, pn))
		return false;

	Assert(*pn >= knAreaLastDiscovery, "Detected reference to invalid (deleted?) Area");
	return true;
}

bool ParseUnitMask(char **ppsz, UnitMask *pum)
{
	*pum = 0;
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%ld,%+", pum, &nch);
	*ppsz += nch;
	return cArgs >= 1;
}

bool ParseUpgradeMask(char **ppsz, UpgradeMask *pupgm)
{
	int upgm = 0;
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%d,%+", &upgm, &nch);
	*ppsz += nch;
	*pupgm = (word)upgm;
	return cArgs >= 1;
}

bool ParseString(char **ppsz, char *psz)
{
	int nch = 0;
	int cArgs = IniScanf(*ppsz, "%s,%+", psz, &nch);

	// If this is the last of a comma separated list of items set the
	// string pointer to point to the zero-terminator so subsequent
	// calls will fail.

	if (cArgs == 1) {
		*ppsz += strlen(*ppsz);
	} else {
		*ppsz += nch;
	}
	return cArgs >= 1;
}

} // namespace wi
