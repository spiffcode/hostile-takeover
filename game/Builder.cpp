#include "ht.h"

namespace wi {

const short kcyBuildProgress = 3;

//===========================================================================
// BuilderGob implementation
//

bool BuilderGob::InitClass(BuilderConsts *pbldrc, IniReader *pini)
{
	if (!StructGob::InitClass(pbldrc, pini))
		return false;

	char szTemplate[10];
	itoa(pbldrc->gt, szTemplate, 10);

	// Required properties

	if (pini->GetPropertyValue(szTemplate, "BuildRate", "%d", &pbldrc->nBuildRate) != 1)
		return false;

	return true;
}

void BuilderGob::ExitClass(BuilderConsts *pbldrc)
{
	StructGob::ExitClass(pbldrc);
}

BuilderGob::BuilderGob(BuilderConsts *pbldrc) : StructGob(pbldrc)
{
	m_gidBuildVisible = kgidNull;
	m_fxHealthBuilding = itofx(0);
	m_nCreditsSpentOnBuilding = 0;
	m_nCostRemainder = 0;
	m_iLastSelection = 0;
	m_tptRally.tx = m_tptRally.ty = ktxInvalid;
}

BuilderGob::~BuilderGob()
{
}

bool BuilderGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	if (!StructGob::Init(wx, wy, pplr, fxHealth, ff, pszName))
		return false;

	// Initalize the rally point for units built by this Builder

	WRect wrc;
	GetTilePaddedWRect(&wrc);

	// Center spot "at the bottom" middle

	m_tptRally.tx = TcFromWc(wrc.left + wrc.Width() / 3);
	m_tptRally.ty = TcFromWc(wrc.bottom);
	return true;
}

#define knVerBuilderGobState 4
bool BuilderGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerBuilderGobState)
		return false;
	m_bq.LoadState(pstm);
	if (!m_bq.IsEmpty()) {
		m_gidBuildVisible = pstm->ReadWord();
		m_fxHealthBuilding = pstm->ReadWord();
		m_nCreditsSpentOnBuilding = pstm->ReadWord();
		m_nCostRemainder = pstm->ReadWord();
		m_tptRally.tx = pstm->ReadWord();
		m_tptRally.ty = pstm->ReadWord();
	}
	return StructGob::LoadState(pstm);
}

bool BuilderGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerBuilderGobState);
	m_bq.SaveState(pstm);
	if (!m_bq.IsEmpty()) {
		pstm->WriteWord(m_gidBuildVisible);
		pstm->WriteWord(m_fxHealthBuilding);
		pstm->WriteWord(m_nCreditsSpentOnBuilding);
		pstm->WriteWord(m_nCostRemainder);
		pstm->WriteWord(m_tptRally.tx);
		pstm->WriteWord(m_tptRally.ty);
	}
	return StructGob::SaveState(pstm);
}

void BuilderGob::Deactivate()
{
	// half-baked buildings blow when builder goes bye-bye
	// either through takeover or being destroyed
	// on Deactivate is the only way to respond to being destroyed
	// by the enemy

	AbortBuild();

	StructGob::Deactivate();
}

int BuilderGob::GetGlobalQueuedCount(Player *pplr, word wfTest)
{
	int cQueued = 0;
	word wfPlr = (pplr->GetFlags() & kfPlrComputer);
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (!(pgobT->GetFlags() & kfGobUnit))
			continue;
		UnitGob *puntT = (UnitGob *)pgobT;
		if ((puntT->GetOwner()->GetFlags() & kfPlrComputer) != wfPlr)
			continue;
		if (!(puntT->GetConsts()->wf & wfTest))
			continue;
		BuilderGob *pbldr = (BuilderGob *)puntT;
		cQueued += pbldr->m_bq.GetUnitCount(kutNone);
	}

	return cQueued;
}

int BuilderGob::GetQueuedCount(Player *pplr, word wfTest)
{
	int cQueued = 0;
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (!(pgobT->GetFlags() & kfGobUnit))
			continue;
		UnitGob *puntT = (UnitGob *)pgobT;
		if (puntT->GetOwner() != pplr)
			continue;
		if (!(puntT->GetConsts()->wf & wfTest))
			continue;
		BuilderGob *pbldr = (BuilderGob *)puntT;
		cQueued += pbldr->m_bq.GetUnitCount(kutNone);
	}

	return cQueued;
}

void BuilderGob::Build(UnitType ut, Gid gid)
{
	// put it on the public build queue

	m_bq.Enqueue(ut);
	m_gidBuildVisible = gid;

	// The build queue is acted on during Update

	m_unvl.MinSkip();
}

void BuilderGob::AbortBuild(bool fRefundCreditsSpentOnBuilding, UnitType ut)
{
    // this needs to be called by a queued message if triggered by the player
    // because it affects credits, especially if they're recycling called by
    // kmidAbortBuildOtherCommand and by deactivate & takeover refunds
    // CreditsSpent by parameter, and tells self-destruct to not refund half
    // the value of the building 
    // we're either aborting the build of a specific unit (chosen by the user)
    // or we're flushing the queue (due to deactivate, takeover or structure
    // build abort)

	// empty queue case

	if (m_bq.IsEmpty())
		return;

	bool fCurrentConstruction = true;

	if (ut == kutNone) {

		// flush the queue. If we were building a structure, clean it up

		m_bq.Clear();

		StructGob *pgobBuild = (StructGob *)ggobm.GetGob(m_gidBuildVisible, false);
		if (pgobBuild != NULL) {
			pgobBuild->SetFlags(pgobBuild->GetFlags() & ~kfGobBeingBuilt);
			pgobBuild->SelfDestruct(false);
		}
	} else {
		Assert(m_gidBuildVisible == kgidNull);
		fCurrentConstruction = m_bq.RemoveUnit(ut);
	}

	if (fCurrentConstruction) {

		// we just aborted the current construction

		if (fRefundCreditsSpentOnBuilding) {
			m_pplr->SetCredits(m_pplr->GetCredits() + m_nCreditsSpentOnBuilding, false);
			m_pplr->ModifyTotalCreditsConsumed(-m_nCreditsSpentOnBuilding);
		}

		m_nCreditsSpentOnBuilding = 0;
		m_nCostRemainder = 0;
		m_fxHealthBuilding = itofx(0);
		WaitingForCredits(false);
	}	
}

void BuilderGob::DefUpdate()
{
	StructGob::DefUpdate();

	if (m_bq.IsEmpty())
		return;

	if (m_ff & kfGobBeingUpgraded) {

		// we'll pause building, but make sure we're showing credits correctly
		// if we were building when the upgrade started

 		long nCredits = m_pplr->GetCredits();
		WaitingForCredits(nCredits==0);
		return;
	}

	UnitConsts *puntc = gapuntc[m_bq.Peek()];
	UnitGob *pgobBuild = (UnitGob *)ggobm.GetGob(m_gidBuildVisible, false);

	// God can build things instantly and for free

	bool fBuild = false;
	if (gfGodMode && !ggame.IsMultiplayer()) {
		fBuild = true;
		WaitingForCredits(false);
	} else {

 		// CostPerUpdate is a fraction. We track its remainder and add it
 		// in the next update so we charge the right amount overall.

		int nCost = puntc->GetCost() + m_nCostRemainder;
		int cupdTimeToBuild = puntc->GetTimeToBuild();
		if (m_pplr->GetHandicap() & kfHcapDecreasedTimeToBuild)
			cupdTimeToBuild = ((cupdTimeToBuild * (100 + knDecreasedTimeToBuildPercent)) + 50) / 100; // +50 for rounding

		// if power is low and this building is so influenced, slow production
		// by inflating TimeToBuild

		if ((m_puntc->wf & kfUntcNotifyPowerLowHigh) && m_pplr->IsPowerLow()) {
			cupdTimeToBuild *= 2;  //TUNE:
		}

 		int nCostPerUpdate = nCost / cupdTimeToBuild;

 		int nCredits = m_pplr->GetCredits() - nCostPerUpdate;
 		if (nCredits >= 0) {
			WaitingForCredits(false);

 			m_pplr->SetCredits(nCredits, true);
			m_nCreditsSpentOnBuilding += nCostPerUpdate;
			m_nCostRemainder = nCost % cupdTimeToBuild;

 			fix fxHealthPerUpdate = puntc->GetArmorStrength() / cupdTimeToBuild;

			fix fxHealth = addfx(m_fxHealthBuilding, fxHealthPerUpdate);

			if (m_nCreditsSpentOnBuilding >= puntc->GetCost()) {
				// power changes during building can get us off by a credit
				//Assert(m_nCreditsSpentOnBuilding == puntc->GetCost());
				fxHealth = puntc->GetArmorStrength();
				fBuild = true;
			}
			m_fxHealthBuilding = fxHealth;
			if (pgobBuild != NULL)
				pgobBuild->SetHealth(m_fxHealthBuilding);

			// This gob needs to redraw its build status indicator

			MarkRedraw();
		} else {
			if (!(m_wfUnit & kfUnitRepairing)) {

				// we'll only manage this if there is not repair code doing so

				WaitingForCredits(true);
			}

			// pgobBuild still animates even when credits have run out

			if (pgobBuild != NULL)
				pgobBuild->MarkRedraw();
		}

		// to skip more intervals while not building would mean either A) it takes
		// up to kcupdSymbolFlashRate-1 updates for building to resume with a next update
		// or B) we need some kind of notification system to force an update. Notification system
		// is complex because builders don't drain credits to zero so can't tell just
		// from credits whether or not buildings need notifications.

		m_unvl.MinSkip();
	}

	if (fBuild) {
		if (pgobBuild != NULL) {
			pgobBuild->SetHealth(puntc->GetArmorStrength());
			pgobBuild->SetFlags(pgobBuild->GetFlags() & ~kfGobBeingBuilt);
		}

		// Kick off delivery of the new build

		if (m_pplr == gpplrLocal)
			gsndm.PlaySfx(m_pbldrc->sfxUnitReady);
		SetState(kstBuildOtherCompleting);
	}
}

bool BuilderGob::ShowingBuildProgress() {
    if (m_bq.IsEmpty()) {
        return false;
    }

    if (IsAlly(gpplrLocal->GetSide())) {
        return true;
    }

    if (gpplrLocal->GetHandicap() & kfHcapShowEnemyBuildProgress) {
        return true;
    }

    return false;
}

void BuilderGob::GetClippingBounds(Rect *prc) {
    StructGob::GetClippingBounds(prc);

    // Make sure build progress is in this, if it is visible
    if (ShowingBuildProgress()) {
        Rect rcT;
        rcT.FromWorldRect(&m_puntc->wrcUIBounds);
        rcT.Offset(PcFromWc(m_wx), PcFromWc(m_wy));
        prc->left = _min(prc->left, rcT.left);
        prc->right = _max(prc->right, rcT.right);
    }
}

void BuilderGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	if (nLayer == knLayerDepthSorted) {
        if (ShowingBuildProgress()) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
			DrawBuildProgressIndicator(pbm, &rcT, m_fxHealthBuilding, gapuntc[m_bq.Peek()]->GetArmorStrength());
		}
	}
}

void BuilderGob::DrawUpgradeEffect(DibBitmap *pbm, int xViewOrigin, int yViewOrigin)
{
	if (!(m_ff & kfGobActive))
		return;
	if (!IsAlly(gpplrLocal->GetSide()) && ((gpplrLocal->GetHandicap() & kfHcapShowEnemyBuildProgress) == 0))
		return;

	// UNDONE: hack-effect
	WRect wrcBounds;
	GetTilePaddedWRect(&wrcBounds);
	Rect rcBounds;
	rcBounds.FromWorldRect(&wrcBounds);
	rcBounds.Offset(-xViewOrigin, -yViewOrigin);
	int nT = _min(rcBounds.Width(), rcBounds.Height()) / 3;

	int cupd = gsim.GetUpdateCount();
	int n = (cupd % 4);
	Color clr = GetColor(kiclrGalaxite);
	if (n < 2) {
		Rect rcT = rcBounds;
		rcT.Inflate(-nT, -nT);
		DrawBorder(pbm, &rcT, 2, clr);
	} else {
		DrawBorder(pbm, &rcBounds, 2, clr);
	}

	if (!(m_ff & kfGobSelected)) {
		Rect rcT;
		rcT.FromWorldRect(&m_puntc->wrcUIBounds);
		rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
		DrawHealthIndicator(pbm, &rcT, m_fxHealth, m_puntc->GetArmorStrength());
	}
}

UnitGob *BuilderGob::GetBuiltGob()
{
    // Buildings are visible and added to the gob mgr when building. Vehicles
    // are not until the build is done.

	Assert(!m_bq.IsEmpty());
	UnitGob *pgobBuild = (UnitGob *)ggobm.GetGob(m_gidBuildVisible);
	if (pgobBuild != NULL)
		return pgobBuild;
	return (UnitGob *)CreateGob(gapuntc[m_bq.Peek()]->gt);
}

void BuilderGob::ClearBuiltGob()
{
	// if this assert goes off then an AbortBuild may have somehow snuck in
	// between setting kstBuildOtherCompleting and the derived class calling this

	Assert((m_nCreditsSpentOnBuilding > 0) || gfGodMode);

	m_bq.Dequeue();
	m_gidBuildVisible = kgidNull;
 	m_nCostRemainder = 0;
	m_nCreditsSpentOnBuilding = 0;
	m_fxHealthBuilding = itofx(0);
}

void BuilderGob::DrawBuildProgress(DibBitmap *pbm, Rect *prc)
{ 
	if (m_bq.IsEmpty())
		return;

	Color clr = GetColor(kiclrYellow);
	int cxWidth = prc->right - prc->left;
	int nLength = (cxWidth * (int)m_fxHealthBuilding) / gapuntc[m_bq.Peek()]->GetArmorStrength();
	pbm->Fill(prc->left, prc->top, nLength, kcyBuildProgress, clr);
}

void BuilderGob::SyncBuildQueue(BuildQueue *pbq)
{
	// operator is overloaded

	*pbq = m_bq;
}

// Note: s_aptInit has been fixed to not have a structure sized-hole

static char s_aptInit[] = {
	0, 0, 0, -1, -1, 0, 1, 0, 0, 1, -1, -1, 1, -1, -1, 1,
	1, 1, 0, -2, -2, 0, 2, 0, 0, 2, -1, -2, 1, -2, -2, -1,
	2, -1, -2, 1, 2, 1, -1, 2, 1, 2, -2, -2, 2, -2, -2, 2,
	2, 2, 0, -3, -3, 0, 3, 0, 0, 3, -1, -3, 1, -3, -3, -1,
	3, -1, -3, 1, 3, 1, -1, 3, 1, 3, -2, -3, 2, -3, -3, -2,
	3, -2, -3, 2, 3, 2, -2, 3, 2, 3, 0, -4, -4, 0, 4, 0,
	0, 4, -1, -4, 1, -4, -4, -1, 4, -1, -4, 1, 4, 1, -1, 4,
	1, 4, -3, -3, 3, -3, -3, 3, 3, 3, -2, -4, 2, -4, -4, -2,
	4, -2, -4, 2, 4, 2, -2, 4, 2, 4, 0, -5, -3, -4, 3, -4,
	-4, -3, 4, -3, -5, 0, 5, 0, -4, 3, 4, 3, -3, 4, 3, 4,
	0, 5, -1, -5, 1, -5, -5, -1, 5, -1, -5, 1, 5, 1, -1, 5,
	1, 5, -2, -5, 2, -5, -5, -2, 5, -2, -5, 2, 5, 2, -2, 5,
	2, 5, -4, -4, 4, -4, -4, 4, 4, 4, -3, -5, 3, -5, -5, -3,
	5, -3, -5, 3, 5, 3, -3, 5, 3, 5, 0, -6, -6, 0, 6, 0,
	0, 6, -1, -6, 1, -6, -6, -1, 6, -1, -6, 1, 6, 1, -1, 6,
	1, 6, -2, -6, 2, -6, -6, -2, 6, -2, -6, 2, 6, 2, -2, 6,
};

void FindNearestFreeTile(TCoord tx, TCoord ty, WPoint *pwpt, byte bf)
{
	int cpt = sizeof(s_aptInit) / 2;
	for (int npt = 0; npt < cpt; npt++) {
		int txT = tx + s_aptInit[npt * 2];
		int tyT = ty + s_aptInit[npt * 2 + 1];
		if (IsTileFree(txT, tyT, bf)) {
			pwpt->wx = WcFromTc(txT) + kwcTileHalf;
			pwpt->wy = WcFromTc(tyT) + kwcTileHalf;
			return;
		}
	}

	// Couldn't find anything good

	pwpt->wx = WcFromTc(tx) + kwcTileHalf;
	pwpt->wy = WcFromTc(ty) + kwcTileHalf;
}

void BuilderGob::FindInitPosition(WPoint *pwpt)
{
	WRect wrc;
	GetTilePaddedWRect(&wrc);

	// Center spot "at the bottom" middle

	TCoord tx = TcFromWc(wrc.left + wrc.Width() / 3);
	TCoord ty = TcFromWc(wrc.bottom);

	FindNearestFreeTile(tx, ty, pwpt);
}

//===========================================================================
// MobileUnitBuildForm implementation (used by both VTS and HRC but HQ has its own)

void MobileUnitBuildForm::SetOwner(BuilderGob *pbldr)
{
	// Position

	Size sizDib;
	m_pfrmm->GetDib()->GetSize(&sizDib);
	Rect rc;
	rc.left = ((sizDib.cx - m_rc.Width()) / 2) & ~1;
	rc.top = 0; // (sizDib.cy - m_rc.Height()) / 2;
	rc.right = rc.left + m_rc.Width();
	rc.bottom = rc.top + m_rc.Height();
	SetRect(&rc);

	m_wf |= kfFrmAutoTakedown | kfFrmNoEcom;
	m_pbldr = pbldr;
	m_pbldr->SyncBuildQueue(&m_bqPrivate);

	BuildListControl *plstc = (BuildListControl *)GetControlPtr(kidcList);
	plstc->Clear();

	// Initialize list with available unit types.
	// UNDONE: need to link list to dynamic build capability

	Player *pplr = pbldr->GetOwner();
	UnitMask umOwned = (pplr->GetUnitMask() & kumStructures) | pplr->GetUpgrades();
	UnitMask umAllowed = ((BuilderConsts *)pbldr->GetConsts())->umCanBuild & (gfGodMode ? kumAll : pplr->GetAllowedUnits());
	UpgradeMask upgmOwned = pplr->GetUpgradeMask();

	ButtonControl *pbtn;

	m_fOrderValid = !m_bqPrivate.IsFull();

	int j = 0;
	for (int i = 0; i < kutMax; i++) {
		UnitConsts *puntc = gapuntc[i];

		// Does this player have what it takes to build this structure/unit?

		bool fDisabled = true;
		if (gfGodMode || ((umOwned & puntc->umPrerequisites) == puntc->umPrerequisites && 
				(upgmOwned & puntc->upgmPrerequisites) == puntc->upgmPrerequisites))
			fDisabled = false;

		// Yep, is this one of the types this build form allows?

		if (puntc->um & umAllowed) {

			// Yep, add it to the list

			int nStripIcon = puntc->panid->GetStripIndex("icon");
			if (nStripIcon != -1) {				
				plstc->Add(puntc->panid, nStripIcon, 0, (void *)(pword)puntc->ut, (pword)fDisabled);
				j++;
				if (j == m_pbldr->GetLastSelection()) {
					// If disabled, orders are not valid

					if (fDisabled)
						m_fOrderValid = false;

					// Disable Cancel Order button if nothing queued for that unit.

					pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
					pbtn->Show(m_bqPrivate.GetUnitCount(puntc->ut) > 0);

				}
			}
		}
	}
	
	// If nothing added to the list, the order button is invalid

	if (j == 0) {
		m_fOrderValid = false;
		ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
		pbtn->Show(false);
	}

	// Select the last selected unit by default

	// Will be set to the current selection by OnControlSelected via plstc->Select	

	s_iliCurrent = -1;

	plstc->Select(m_pbldr->GetLastSelection());
	plstc->SetQueueInfo(m_pbldr, &m_bqPrivate);

	// Calc what control needs to be visible
    if (plstc->GetSelectedItemIndex() == -1) {
        s_iliCurrent = -1;
        m_fOrderValid = false;
        UpdateOrderButton(false);
    } else {
        UpdateOrderButton(true);
    }
}

void MobileUnitBuildForm::UpdateUnitInfo(ListItem *pli)
{
	MobileUnitConsts *pmuntc = (MobileUnitConsts *)gapuntc[(long)pli->pvData];

	// Update Cost

	LabelControl *plbl = (LabelControl *)GetControlPtr(kidcCost);
	char szT[kcbStructUnitName];
	itoa(pmuntc->GetCost(), szT, 10);
	plbl->SetText(szT);
	PipMeterControl *pmtr = (PipMeterControl *)GetControlPtr(kidcCostMeter);
	Assert(pmuntc->GetCost() <= GetUnitCostMax()); // make sure we're scaling correctly
	pmtr->SetValue(((int)pmuntc->GetCost() * 100) / GetUnitCostMax());

	// Update Name

	plbl = (LabelControl *)GetControlPtr(kidcName);
	strcpy(szT, pmuntc->szLongName);
	HtStrupr(szT);
	plbl->SetText(szT);

	// Update Speed

	WCoord wcOneThirdRange = (gwcMoveDistPerUpdateMax - gwcMoveDistPerUpdateMin) / 3;
	plbl = (LabelControl *)GetControlPtr(kidcMoveRate);
	char *pszT = "MEDIUM";
	if (pmuntc->GetMoveDistPerUpdate() < gwcMoveDistPerUpdateMin + wcOneThirdRange)
		pszT = "SLOW";
	else if (pmuntc->GetMoveDistPerUpdate() >= gwcMoveDistPerUpdateMax - wcOneThirdRange)
		pszT = "FAST";
	plbl->SetText(pszT);
	pmtr = (PipMeterControl *)GetControlPtr(kidcMoveRateMeter);
	Assert(pmuntc->GetMoveDistPerUpdate() <= gwcMoveDistPerUpdateMax);
	pmtr->SetValue((pmuntc->GetMoveDistPerUpdate() * 100) / gwcMoveDistPerUpdateMax);

	// Update Firepower

	DamageMeterControl *pdmtr = (DamageMeterControl *)GetControlPtr(kidcWeaponStrengthMeter);
	pdmtr->SetUnitConsts(pmuntc);

	// Update Armor

	fix fxOneThirdRange = (fix)divfx(subfx(gfxMobileUnitArmorStrengthMax, gfxMobileUnitArmorStrengthMin), itofx(3));
	plbl = (LabelControl *)GetControlPtr(kidcArmorStrength);
	pszT = "MEDIUM";
	if (pmuntc->GetArmorStrength() == 0)
		pszT = "NONE";
	else if (pmuntc->GetArmorStrength() < addfx(gfxMobileUnitArmorStrengthMin, fxOneThirdRange))
		pszT = "LIGHT";
	else if (pmuntc->GetArmorStrength() >= subfx(gfxMobileUnitArmorStrengthMax, fxOneThirdRange))
		pszT = "HEAVY";
	plbl->SetText(pszT);
	pmtr = (PipMeterControl *)GetControlPtr(kidcArmorStrengthMeter);
	Assert(pmuntc->GetArmorStrength() <= gfxMobileUnitArmorStrengthMax);
	pmtr->SetValue(((int)fxtoi(pmuntc->GetArmorStrength()) * 100) / fxtoi(gfxMobileUnitArmorStrengthMax));

	// Update Range

	TCoord tcOneThirdRange = ((gtcFiringRangeMax - gtcFiringRangeMin) * 100) / 3;
	plbl = (LabelControl *)GetControlPtr(kidcWeaponRange);
	pszT = "MEDIUM";
	if (pmuntc->tcFiringRange == 0)
		pszT = "NONE";
	else if (pmuntc->tcFiringRange * 100 <= tcOneThirdRange)
		pszT = "LOW";
	else if (pmuntc->tcFiringRange * 100 >= (gtcFiringRangeMax * 100) - tcOneThirdRange)
		pszT = "HIGH";
	plbl->SetText(pszT);
	pmtr = (PipMeterControl *)GetControlPtr(kidcWeaponRangeMeter);
	Assert(pmuntc->tcFiringRange-1 <= gtcFiringRangeMax);
	pmtr->SetValue(((pmuntc->tcFiringRange - 1) * 100) / gtcFiringRangeMax);

	// Update Description. Substitute the list of prerequisites if the
	// item is disabled.

	plbl = (LabelControl *)GetControlPtr(kidcDescription);
	if (pli->fDisabled) {
		char szT[120];
		GetPrerequisiteString(szT, pmuntc);
		
		char szT2[120];
		sprintf(szT2, "This unit requires: %s.", szT);
		plbl->SetText(szT2);

	} else {
		plbl->SetText(pmuntc->szDescription);
	}

	// Hide the "Order"/"Recruit" button if this structure is disabled

	m_fOrderValid = !m_bqPrivate.IsFull() && !pli->fDisabled;
	UpdateOrderButton(false);
}

void MobileUnitBuildForm::EndForm(int nResult)
{
	ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
	m_pbldr->SetLastSelection(plstc->GetSelectedItemIndex());
	m_pbldr = NULL;
	Form::EndForm(nResult);
}

int MobileUnitBuildForm::s_iliCurrent = -1;
#define kcupdLimit 6

void MobileUnitBuildForm::OnControlSelected(word idc)
{
	switch (idc) {
    case kidcLimitReached:
        break;

	case kidcCancel:
		EndForm(kidcCancel);
		return;
//CRM
	case kidcHelp:
		Help(m_idf == kidfBuildInfantry ? "personnel" : "vehicles", !ggame.IsMultiplayer());
		break;

	case kidcOrder:
		{
			m_fOrderValid = !m_bqPrivate.IsFull();
			UpdateOrderButton(true);

			if (m_fOrderValid && !m_fLimitReached) {
				Message msg;
				memset(&msg, 0, sizeof(msg));
				msg.smidSender = ksmidNull;
				msg.mid = kmidBuildOtherCommand;
				ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
				msg.BuildOtherCommand.ut = UnitTypeFromPVoid(plstc->GetSelectedItemData());
				msg.smidReceiver = m_pbldr->GetId();
				gcmdq.Enqueue(&msg);

				if (m_pbldr->GetOwner() == gpplrLocal) 
					gsndm.PlaySfx(((BuilderConsts *)m_pbldr->GetConsts())->sfxUnitBuild);

				// update our private copy of the queue and see if we filled it

				m_bqPrivate.Enqueue(UnitTypeFromPVoid(plstc->GetSelectedItemData()));

				ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
				pbtn->Show(true);

				// Need to update the order button / show the limit reached label.
				// Since the order was queued we don't really know if the limit has been
				// reached. We'll just update it asap.

				m_cupdLast = gsim.GetUpdateCount() - kcupdLimit + 1;
				m_fOrderValid = !m_bqPrivate.IsFull();
				UpdateOrderButton(false);

				// repaint the number showing the unit count

				plstc->Invalidate();
			}
		}
		break;

	case kidcCancelOrder:
		{
			// decrement the appropriate count

			ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
			if (m_bqPrivate.GetUnitCount(UnitTypeFromPVoid(plstc->GetSelectedItemData())) == 0) {
				Assert(false);  // if my input lags this might be possible...
				return;
			}

			Message msg;
			memset(&msg, 0, sizeof(msg));
			msg.smidSender = ksmidNull;
			msg.mid = kmidAbortBuildOtherCommand;
			msg.AbortBuildOtherCommand.ut= UnitTypeFromPVoid(plstc->GetSelectedItemData());
			msg.smidReceiver = m_pbldr->GetId();
			gcmdq.Enqueue(&msg);

			if (m_pbldr->GetOwner() == gpplrLocal) 
				gsndm.PlaySfx(((BuilderConsts *)m_pbldr->GetConsts())->sfxUnitBuildAbort);

			// decrement the the queue count. private queue for immediate response
			
			UnitType ut = UnitTypeFromPVoid(plstc->GetSelectedItemData());
			m_bqPrivate.RemoveUnit(ut);
			plstc->Invalidate();

			// Queue is not full by definition but gob count may still be at limit

			m_fOrderValid = true;
			m_fLimitReached = false;
			ListItem *pli = plstc->GetSelectedItem();
			if (pli != NULL)
                m_fOrderValid = !pli->fDisabled;
			UpdateOrderButton(false);

			if (m_bqPrivate.GetUnitCount(ut) == 0) {
				ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
				pbtn->Show(false);
			}
		}
		break;

	case kidcOk:
		EndForm();
		return;
	}
}

void MobileUnitBuildForm::OnControlNotify(word idc, int nNotify)
{
	if (idc != kidcList)
		return;

	if (nNotify != knNotifySelectionChange && nNotify != knNotifySelectionTap) {
        return;
    }

    ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
    ListItem *pli = plstc->GetSelectedItem(); 
    if (pli == NULL) {
        s_iliCurrent = -1;
        m_fOrderValid = false;
        UpdateOrderButton(false);
        return;
    }

    UpdateUnitInfo(pli);
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
    pbtn->Show(m_bqPrivate.GetUnitCount(UnitTypeFromPVoid(plstc->GetSelectedItemData())) > 0);

    // Can we order?

    m_fOrderValid = !m_bqPrivate.IsFull() && !pli->fDisabled;
    UpdateOrderButton(false);

    if (m_fOrderValid && !m_fLimitReached) {
        // Tap on a selected item orders it

        int ili = plstc->GetSelectedItemIndex();
        if (s_iliCurrent == ili && !pli->fDisabled) {
            OnControlSelected(kidcOrder);
        } else {
            s_iliCurrent = ili;
        }
    }
}

void MobileUnitBuildForm::UpdateOrderButton(bool fCalcLimit)
{
	Control *pctlLimit = GetControlPtr(kidcLimitReached);
	Control *pctlOrder = GetControlPtr(kidcOrder);

	// Limit only gets calced 

	if (fCalcLimit) {
		m_fLimitReached = !ggobm.IsBelowLimit(knLimitMobileUnit,
                m_pbldr->GetOwner());
    }

	if (m_fLimitReached) {
		pctlLimit->Show(true);
        pctlLimit->SetFlags(pctlLimit->GetFlags() | kfLblHitTest);
		pctlOrder->Show(false);
	} else {
		pctlLimit->Show(false);
		pctlOrder->Show(m_fOrderValid);
	}
}

void MobileUnitBuildForm::DefUpdate(BuilderGob *pbldr, bool fBuildInProgress)
{
	// assumes this is only called if a build is in progress

	if (!(m_wf & kfFrmVisible) || pbldr != m_pbldr)
		return;

	// May have multiple builders that have caused the limit to be reached.
	// Poll to figure this out. This takes into account the units already queued too.
	// Don't do it every update. This gets rechecked at actual order time so it'll be
	// up to date.

	int cupdCurrent = gsim.GetUpdateCount();
	if (abs(cupdCurrent - m_cupdLast) > kcupdLimit) {
		m_cupdLast = cupdCurrent;
		UpdateOrderButton(true);
	}

	// if we're showing during building, we have a progress bar we're 
	// advancing. Make it update.

	if (fBuildInProgress) {
		ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
		plstc->Invalidate();
	}
}

void MobileUnitBuildForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	TBitmap *ptbm = CreateTBitmap("buildformbkgd.png");
	BltHelper(pbm, ptbm, pupd, m_rc.left, m_rc.top);
	delete ptbm;
}

void MobileUnitBuildForm::OnUnitCompleted(BuilderGob *pbldr, UnitType ut)
{
	if (!(m_wf & kfFrmVisible) || pbldr != m_pbldr)
		return;
	
	m_bqPrivate.RemoveUnit(ut);

	// fix up button state 

	ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
	ListItem *pli = plstc->GetSelectedItem();

	m_fOrderValid = pli == NULL || !pli->fDisabled;
	UpdateOrderButton(true);

	if (m_bqPrivate.GetUnitCount(ut) == 0) {
		if (UnitTypeFromPVoid(plstc->GetSelectedItemData()) == ut) {
			ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcCancelOrder);
			pbtn->Show(false);
		}
	}
}

//
// BuildListControl
//

bool BuildListControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!ListControl::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) nfnt cyItem

	int cyItem;
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %*d %d",
			&cyItem);
	if (cArgs != 1)
		return false;

	m_cyItem = PcFromFc(cyItem);
	m_pbldr = NULL;
	m_pbq = NULL;
    SetScrollPosColorIndex(kiclrListBorder);
	return true;
}

bool BuildListControl::Add(AnimationData *panid, int nStrip, int nFrame, void *pvData, bool fDisabled)
{
	ListItem *pli = new ListItem;
	if (pli == NULL)
		return false;

	pli->Anim.panid = panid;
	pli->Anim.nStrip = nStrip;
	pli->Anim.nFrame = nFrame;
	pli->pvData = pvData;
	pli->fDisabled = fDisabled;

	return ListControl::Add(pli);
}

void BuildListControl::GetSubRects(Rect *prcInterior, Rect *prcUpArrow,
        Rect *prcDownArrow, Rect *prcScrollPosition)
{
	Size sizArrow;
	s_ptbmScrollUpUp->GetSize(&sizArrow);

	int l = m_rc.left + gcxyBorder;
	int r = m_rc.right - gcxyBorder;
	int t = m_rc.top + 1;
	int b = m_rc.bottom - 1;

    if (m_wf & kfLstcScrollPosition) {
        if (prcUpArrow != NULL) {
            prcUpArrow->SetEmpty();
        }
        if (prcDownArrow != NULL) {
            prcDownArrow->SetEmpty();
        }
        prcInterior->Set(l, t + sizArrow.cy, r - gcxyBorder * 2,
                b - sizArrow.cy);
        if (prcScrollPosition != NULL) {
            prcScrollPosition->Set(r - gcxyBorder, prcInterior->top, r,
                    prcInterior->bottom - prcInterior->Height() % m_cyItem);
        }
    } else {
        if (prcUpArrow != NULL) {
            prcUpArrow->Set(l, t, r, t + sizArrow.cy);
        }
        if (prcDownArrow != NULL) {
            prcDownArrow->Set(l, b - sizArrow.cy, r, b);
        }
        prcInterior->Set(l, t + sizArrow.cy, r, b - sizArrow.cy);
        if (prcScrollPosition != NULL) {
            prcScrollPosition->SetEmpty();
        }
    }
}

void BuildListControl::OnPaint(DibBitmap *pbm)
{
	Rect rcForm;
	m_pfrm->GetRect(&rcForm);

	Rect rc;
	GetRect(&rc);
	rc.Offset(rcForm.left, rcForm.top);

	DrawBorder(pbm, &rc, 1, GetColor(kiclrListBorder));
	pbm->Fill(rc.left + 1, rc.top + 1, rc.Width() - 2, rc.Height() -2, GetColor(kiclrListBackground));

	ListControl::OnPaint(pbm);
}

void BuildListControl::DrawItem(DibBitmap *pbm, ListItem *pli, int x, int y, int cx, int cy)
{
	HostSoundServiceProc();

	Rect rc;
	if (pli->fSelected) {
		rc.Set(x, y, x + cx, y + cy);
		DrawBorder(pbm, &rc, 2, GetColor(kiclrWhite));
	} else {
		rc.Set(x + 1, y + 1, x + cx - 1, y + cy - 1);
		DrawBorder(pbm, &rc, 1, GetColor(kiclrListBorder));
	}

	AnimationData *panid = pli->Anim.panid;
	int nStrip = pli->Anim.nStrip;
	int nFrame = pli->Anim.nFrame;

	// Center the image within the item area

	Rect rcAnim;
	pli->Anim.panid->GetBounds(nStrip, nFrame, &rcAnim);

	int yOff = (cy - rcAnim.Height()) / 2;
	int xOff = (cx - rcAnim.Width()) / 2;

	panid->DrawFrame(nStrip, nFrame, pbm, x + xOff, y + yOff, gpplrLocal->GetSide());

	if (pli->fDisabled)
		pbm->Shadow(rc.left, rc.top, rc.Width(), rc.Height());

	// if we have a builder pointer, draw building information

	if ((m_pbq != NULL) && (m_pbq->GetUnitCount(UnitTypeFromPVoid(pli->pvData)) > 0)) {

		// draw a test health bar. offset by two pixels - either the width of the selected
		// rect or the width & offset of an unselected rect
		Assert(m_pbldr != NULL);

		if (m_pbldr->UnitBuildInProgress() == UnitTypeFromPVoid(pli->pvData)) {
			rc.top = y + cy - (2 + kcyBuildProgress); 
			rc.left = x + 2;
			rc.bottom = y + cy - 2;
			rc.right = x + cx - 2;
			m_pbldr->DrawBuildProgress(pbm, &rc);
		}

		Font *pfnt = gapfnt[kifntButton];
		char szCount[3];
		sprintf(szCount,"%d", m_pbq->GetUnitCount(UnitTypeFromPVoid(pli->pvData)));
		int cxQ = pfnt->GetTextExtent(szCount);
		pfnt->DrawText(pbm, szCount, x+cx-2-cxQ, y+cy-2-pfnt->GetHeight());
	}
}

// ----------------------------------------------------------------------------------
// BuildQueue - helper class to ensure the public and private queues behave the same
//
// m_achBuildQueue is a signed byte array of unit types. as items are pulled off everything
// is scooted up with a memcopy. That way units can be canceled in a different order than
// they were ordered and gaps can be closed with memcopy while preserving the overall order
// m_achBuildQueue is bytes for size consideratons but UnitTypes are actually larger so be aware
// ----------------------------------------------------------------------------------

// follow the versioning for the build gob
bool BuildQueue::LoadState(Stream *pstm)
{
	m_cchQueueMax = pstm->ReadWord();
	if (m_cchQueueMax > 0)
		pstm->ReadBytesRLE((byte *)m_achBuildQueue, m_cchQueueMax);

	return true;
}

bool BuildQueue::SaveState(Stream *pstm)
{
	pstm->WriteWord(m_cchQueueMax);
	if (m_cchQueueMax > 0)
		pstm->WriteBytesRLE((byte *)m_achBuildQueue, m_cchQueueMax);
	return true;
}

BuildQueue::BuildQueue()
{
	Assert(kutMax < 128);
	m_cchQueueMax = kcBuildQueueMax;
	memset(&m_achBuildQueue, (char)kutNone, sizeof(m_achBuildQueue));
}

void BuildQueue::SetSize(word cutMax)
{
	Assert(cutMax <= kcBuildQueueMax);
	m_cchQueueMax = cutMax;
}

int BuildQueue::GetRemainingCapacity()
{
	int cSlotsOpen = 0;
	for (int i = 0; i < m_cchQueueMax; i++) {
		if (m_achBuildQueue[i] == (char)kutNone)
			cSlotsOpen++;
	}
	return cSlotsOpen;
}

void BuildQueue::Enqueue(UnitType ut)
{
	int i = 0;
	while ((m_achBuildQueue[i] != (char)kutNone) && i < m_cchQueueMax)
		i++;

	// we should have made sure there would be room before we get here, we've already
	// told the user it's on the queue by this point

	Assert(i < m_cchQueueMax);

	m_achBuildQueue[i] = (char)ut;
}

void BuildQueue::Dequeue()
{
	// normally Dequeue would return the thing it's dequeuing, but I don't really care

	memmove(&(m_achBuildQueue[0]), &(m_achBuildQueue[1]), (kcBuildQueueMax - 1));
	m_achBuildQueue[kcBuildQueueMax - 1] = (char) kutNone;
}

bool BuildQueue::RemoveUnit(UnitType ut)
{
	// find something of that unit type and pull it out of the queue
	// search from back to front so we remove a queued item before
	// re-setting what's currently building. 

	int i;
	for(i = m_cchQueueMax - 1; i > -1; i--) {
		if (m_achBuildQueue[i] == (char) ut)
			break;
	}
	Assert(i > -1);

	// it's possible we will encounter things being removed that aren't there because
	// of a lag in state. As long as it happens once in each queue we'll be ok. (IE one queue 
	// will fail canceling and the other will fail removing a completed unit) Since only one
	// queue actually reflects game state it's ok for them to get to the same state different ways

	memmove(&(m_achBuildQueue[i]), &(m_achBuildQueue[i+1]), sizeof(char)*(m_cchQueueMax - i));
	m_achBuildQueue[m_cchQueueMax - 1] = kutNone;

	// return true if we removed the currently building item

	return i == 0;
}

UnitType BuildQueue::Peek()
{
	return (UnitType) m_achBuildQueue[0];
}

bool BuildQueue::IsEmpty()
{
	return m_achBuildQueue[0] == (char) kutNone;
}

bool BuildQueue::IsFull()
{
	return m_achBuildQueue[m_cchQueueMax-1] != (char) kutNone;
}

int BuildQueue::GetUnitCount(UnitType ut)
{
	int cut = 0;
	for (int i = 0; i < m_cchQueueMax; i++) {
		if (ut == kutNone) {
			if (m_achBuildQueue[i] != (char)kutNone)
				cut++;
		} else {
			if (m_achBuildQueue[i] == (char)ut)
				cut++;
		}
	}
	return cut;
}

void BuildQueue::Clear()
{
	memset(&m_achBuildQueue, (char)kutNone, kcBuildQueueMax);
}

BuildQueue &BuildQueue::operator=( BuildQueue &bqRHS )
{
	m_cchQueueMax = bqRHS.m_cchQueueMax;
	memcpy(&m_achBuildQueue, bqRHS.m_achBuildQueue, m_cchQueueMax);

	return *this;
}

} // namespace wi
