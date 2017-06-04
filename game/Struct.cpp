#include "ht.h"
#include "wistrings.h"

namespace wi {

static AnimationData *s_panidStructureExplosion = NULL;
static AnimationData *s_panidBigSmoke = NULL;
static DialogForm *s_pfrmConfirmation = NULL;
TBitmap *StructGob::s_ptbmRepairing = NULL;
TBitmap *StructGob::s_ptbmNeedsPower = NULL;
TBitmap *StructGob::s_ptbmNeedCredits = NULL;

//===========================================================================
// StructGob implementation

bool StructGob::InitClass(StructConsts *pstruc, IniReader *pini)
{
	if (!UnitGob::InitClass(pstruc, pini))
		return false;

	char szTemplate[10];
	itoa(pstruc->gt, szTemplate, 10);

	// Required properties

	if (pini->GetPropertyValue(szTemplate, "TileDimensions", "%d,%d", 
			&pstruc->ctx, &pstruc->cty) != 2)
		return false;

	int nArmorStrength;
	if (pini->GetPropertyValue(szTemplate, "ArmorStrength", "%d", &nArmorStrength) != 1)
		return false;
	Assert(nArmorStrength < 1 << 10);
	pstruc->fxArmorStrength = itofx(nArmorStrength);

	// Don't include Headquarters in min/max since they aren't player-buildable

	if (pstruc->gt != kgtHeadquarters) {
		gfxStructureArmorStrengthMin = _min(gfxStructureArmorStrengthMin, pstruc->fxArmorStrength);
		gfxStructureArmorStrengthMax = _max(gfxStructureArmorStrengthMax, pstruc->fxArmorStrength);
	}

	// Optional properties

	if (pini->GetPropertyValue(szTemplate, "ArmorStrengthMP", "%d", &nArmorStrength) != 1) {
		pstruc->fxArmorStrengthMP = pstruc->fxArmorStrength;
	} else {
		Assert(nArmorStrength < 1 << 10);
		pstruc->fxArmorStrengthMP = itofx(nArmorStrength);
	}

	if (pini->GetPropertyValue(szTemplate, "ReserveDimensions", "%d,%d", 
			&pstruc->ctxReserve, &pstruc->ctyReserve) != 2) {
		pstruc->ctxReserve = pstruc->ctx;
		pstruc->ctyReserve = pstruc->cty;
	}

	if (pini->GetPropertyValue(szTemplate, "PowerSupply", "%d", &pstruc->nPowerSupply) != 1)
		pstruc->nPowerSupply = 0;
	gnPowerSupplyMin = _min(gnPowerSupplyMin, pstruc->nPowerSupply);
	gnPowerSupplyMax = _max(gnPowerSupplyMax, pstruc->nPowerSupply);

	if (pini->GetPropertyValue(szTemplate, "PowerDemand", "%d", &pstruc->nPowerDemand) != 1)
		pstruc->nPowerDemand = 0;
	gnPowerDemandMin = _min(gnPowerDemandMin, pstruc->nPowerDemand);
	gnPowerDemandMax = _max(gnPowerDemandMax, pstruc->nPowerDemand);

	if (pini->GetPropertyValue(szTemplate, "UpgradeCost", "%d", &pstruc->nUpgradeCost) != 1)
		pstruc->nUpgradeCost = 0;

	// Preload the structure's menu form

	if (!LoadMenu(pstruc, pini, szTemplate, kidfStructMenu))
		return false;

	// Preload the confirmation "Are you sure?" dialog

	if (!LoadConfirmation()) {
		return false;
	}

	// Preload the smoke animation data

	if (s_panidBigSmoke == NULL) {
		s_panidBigSmoke = LoadAnimationData("smoke.anir");
		if (s_panidBigSmoke == NULL)
			return false;
	}
	
	// Preload the structure explosion animation data

	if (s_panidStructureExplosion == NULL) {
		s_panidStructureExplosion = LoadAnimationData("sexplosion.anir");
		if (s_panidStructureExplosion == NULL)
			return false;
	}

	// Preload the repair symbol bitmap

	if (s_ptbmRepairing == NULL) {
		s_ptbmRepairing = CreateTBitmap("repairing_symbol.png");
		if (s_ptbmRepairing == NULL) {
			Assert("Failed to load repairing_symbol.png");
			return false;
		}
	}
	
	// Preload the needs power symbol bitmap

	if (s_ptbmNeedsPower == NULL) {
		s_ptbmNeedsPower = CreateTBitmap("needs_power_symbol.png");
		if (s_ptbmNeedsPower == NULL) {
			Assert("Failed to load needs_power_symbol.png");
			return false;
		}
	}

	// Preload the need credits symbol bitmap

	if (s_ptbmNeedCredits == NULL) {
		s_ptbmNeedCredits = CreateTBitmap("need_credits_symbol.png");
		if (s_ptbmNeedCredits == NULL) {
			Assert("Failed to load need_credits_symbol.png");
			return false;
		}
	}

	// All structures defog a larger area

	pstruc->wf |= kfUntcLargeDefog;
	
	return true;
}

void StructGob::ExitClass(StructConsts *pstruc)
{
	delete s_ptbmNeedCredits;
	s_ptbmNeedCredits = NULL;
	delete s_ptbmNeedsPower;
	s_ptbmNeedsPower = NULL;
	delete s_ptbmRepairing;
	s_ptbmRepairing = NULL;
	delete s_panidStructureExplosion;
	s_panidStructureExplosion = NULL;
	delete s_panidBigSmoke;
	s_panidBigSmoke = NULL;
	delete s_pfrmConfirmation;
	s_pfrmConfirmation = NULL;

	UnitGob::ExitClass(pstruc);
}

StructGob::StructGob(StructConsts *pstruc) : UnitGob(pstruc)
{
	m_ff |= kfGobStructure;
	m_tLastSmoke = 0;
	m_nSeqLastVisible = 0;
}

StructGob::~StructGob()
{
}

bool StructGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	if (!UnitGob::Init(wx, wy, pplr, fxHealth, ff, pszName))
		return false;

	// Mark this structure's position as occupied on the terrain map

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);
	Assert(tx >= 0 && ty >= 0 && tx + m_pstruc->ctx <= ptrmap->GetWidth() && ty + m_pstruc->cty <= ptrmap->GetHeight(),
			"%s out of map bounds", m_pstruc->szName);
	ptrmap->SetFlags(tx, ty, m_pstruc->ctx, m_pstruc->cty, kbfStructure);
	Assert(tx + m_pstruc->ctxReserve <= ptrmap->GetWidth() && ty + m_pstruc->ctyReserve <= ptrmap->GetHeight(),
			"%s reserve out of map bounds", m_pstruc->szName);
	ptrmap->SetFlags(tx, ty, m_pstruc->ctxReserve, m_pstruc->ctyReserve, kbfReserved);

	// Shadow this structure's occupation in the gid map

	ggobm.ShadowGob(this, TcFromWc(m_wx), TcFromWc(m_wy), m_pstruc->ctx, m_pstruc->cty);

	// Redraw this part of the minimap

	TRect trc;
	GetTileRect(&trc);
	gpmm->RedrawTRect(&trc);

	if (ff & kfGobBeingBuilt) {
		SetState(kstBeingBuilt);
	} else {
		Activate();
		SetState(kstIdle);
	}

	return true;
}

void StructGob::Delete()
{
	// Mark this structure's position as unoccupied on the terrain map

	gsim.GetLevel()->GetTerrainMap()->ClearFlags(TcFromWc(m_wx), TcFromWc(m_wy),
		m_pstruc->ctx, m_pstruc->cty, kbfStructure);
	gsim.GetLevel()->GetTerrainMap()->ClearFlags(TcFromWc(m_wx), TcFromWc(m_wy),
		m_pstruc->ctxReserve, m_pstruc->ctyReserve, kbfReserved);

	// Unshadow this structure's occupation in the gid map

	ggobm.UnshadowGob(this, TcFromWc(m_wx), TcFromWc(m_wy), m_pstruc->ctx, m_pstruc->cty);

	// Remove from Gid map and force minimap redraw

	UnitGob::Delete();
}

#define knVerStructGobState 3
bool StructGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerStructGobState)
		return false;
	dword ctLastSmoke = pstm->ReadDword();
	m_tLastSmoke = gsim.GetTickCount() - ctLastSmoke;

	if (UnitGob::LoadState(pstm)) {
		ggobm.ShadowGob(this, TcFromWc(m_wx), TcFromWc(m_wy), m_pstruc->ctx, m_pstruc->cty);
		if (m_wfUnit & kfUnitNeedCredits)
			WaitingForCredits(true, true);
		return true;
	}
	return false;
}

bool StructGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerStructGobState);
	pstm->WriteDword((dword)(gsim.GetTickCount() - m_tLastSmoke));
	return UnitGob::SaveState(pstm);
}

// are you sure dialog used to confirm structure recycling

bool StructGob::LoadConfirmation()
{
	if (s_pfrmConfirmation != NULL)
		return true;

	s_pfrmConfirmation = new DialogForm();
	if (s_pfrmConfirmation == NULL)
		return false;

	if (!s_pfrmConfirmation->Init(gpmfrmm, gpiniForms, kidfAreYouSure))
		return false;

	s_pfrmConfirmation->SetFlags(s_pfrmConfirmation->GetFlags() | kfFrmAutoTakedown);

	gpmfrmm->RemoveForm(s_pfrmConfirmation);
	return true;
}

bool StructGob::PopupConfirmation(char *pszTitle)
{
	Assert(s_pfrmConfirmation != NULL);

	// set the title color here so it will be correct for the gob we're
	// popping off of

	s_pfrmConfirmation->SetTitleColor(GetSideColor(GetSide()));

	// set the position to be near the gob like the menu is

	Rect rc;
	s_pfrmConfirmation->GetRect(&rc);
	int cx = rc.Width();
	int cy = rc.Height();
	Size sizPlayfield;
	ggame.GetPlayfieldSize(&sizPlayfield);

	WPoint wpt;
	GetCenter(&wpt);
	WCoord wxViewOrigin, wyViewOrigin;
	gsim.GetViewPos(&wxViewOrigin, &wyViewOrigin);

	int xDialog = PcFromWc(wpt.wx - wxViewOrigin) - (cx / 2);
	int yDialog = PcFromWc(wpt.wy - wyViewOrigin) - (cy / 2);
	if (xDialog < 0)
		xDialog = 0;
	else if (xDialog >= sizPlayfield.cx - cx)
		xDialog = sizPlayfield.cx - cx;
	if (yDialog < 0)
		yDialog = 0;
	else if (yDialog >= sizPlayfield.cy - cy)
		yDialog = sizPlayfield.cy - cy;
	rc.Offset(xDialog - rc.left, yDialog - rc.top);
	s_pfrmConfirmation->SetRect(&rc);

	// set the title

	LabelControl *pctl = (LabelControl *)s_pfrmConfirmation->GetControlPtr(kidcTitle);
	pctl->SetText(pszTitle);

	// Show the dialog and get the user's selection

	s_pfrmConfirmation->SetUserDataPtr((void*) this);
	gpmfrmm->AddForm(s_pfrmConfirmation);
	int idc;
	s_pfrmConfirmation->DoModal(&idc);
	gpmfrmm->RemoveForm(s_pfrmConfirmation);
	return idc == kidcOk;
}

void StructGob::TakedownConfirmation()
{
	Assert(s_pfrmConfirmation != NULL);
	if ((StructGob *)s_pfrmConfirmation->GetUserDataPtr() == this)
		s_pfrmConfirmation->EndForm(kidcCancel);
}

void StructGob::Activate()
{
	// Now this structure can influence global power

	m_pplr->AddPowerSupplyAndDemand(m_pstruc->nPowerSupply, m_pstruc->nPowerDemand);

	// Add to area lists

	AreaMask amNew = ggobm.CalcAreaMask(TcFromWc(m_wx), TcFromWc(m_wy), m_pstruc->ctx, m_pstruc->cty);
	ggobm.MoveGobBetweenAreas(m_gid, 0, amNew);

	// Call base

	UnitGob::Activate();
}

void StructGob::Deactivate()
{
	WaitingForCredits(false);
	m_wfUnit &= ~(kfUnitRepairing | kfUnitDrawRepairingSymbol | kfUnitDrawNeedsPowerSymbol);
	EnableOrDisableSymbolLayer();

	// Remove this structure's global power influence

	m_pplr->AddPowerSupplyAndDemand(-m_pstruc->nPowerSupply, -m_pstruc->nPowerDemand);

	Assert(s_pfrmConfirmation != NULL);
	if ((StructGob *)s_pfrmConfirmation->GetUserDataPtr() == this)
		s_pfrmConfirmation->EndForm(kidcCancel);

	// Remove from area lists

	AreaMask amOld = ggobm.CalcAreaMask(TcFromWc(m_wx), TcFromWc(m_wy), m_pstruc->ctx, m_pstruc->cty);
	ggobm.MoveGobBetweenAreas(m_gid, amOld, 0);

	// Call base

	UnitGob::Deactivate();
}

void StructGob::GetTileRect(TRect *ptrc) 
{
	ptrc->left = TcFromWc(m_wx);
	ptrc->top = TcFromWc(m_wy);
	ptrc->right = ptrc->left + m_pstruc->ctx;
	ptrc->bottom = ptrc->top + m_pstruc->cty;
}

void StructGob::GetTilePaddedWRect(WRect *pwrc) 
{
	pwrc->left = WcTrunc(m_wx);
	pwrc->top = WcTrunc(m_wy);
	pwrc->right = pwrc->left + WcFromTc(m_pstruc->ctx);
	pwrc->bottom = pwrc->top + WcFromTc(m_pstruc->cty);
}

bool StructGob::IsAccessible(TCoord tx, TCoord ty)
{
	// tx, ty is in the structure "somewhere". See if it is accessible (a tile somewhere around it that is free
	// of terrain and free of structures

	TerrainMap *ptrmap = gsim.GetLevel()->GetTerrainMap();
	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	for (Direction dir = 0; dir < 8; dir++) {
		TCoord txT = tx + g_mpDirToDx[dir];
		TCoord tyT = ty + g_mpDirToDy[dir];
		if (txT < 0 || txT >= ctx || tyT < 0 || tyT >= cty)
			continue;
		if (!ptrmap->IsBlocked(txT, tyT, kbfStructure))
			return true;
	}
	return false;
}

void StructGob::GetAttackPoint(WPoint *pwpt)
{
	// Need to get a terrain accessible point on this structure that can be attacked.
	// This tile needs to be free of blocking terrain and blocking structures.
	// If there is none at least get the closest to the enemy.

	// Don't use GetCenter() as that uses the UI rect. We want the structure rect which gaurantees
	// a coordinate whose tile is occupied by this structure

	WPoint wptExtent;
	wptExtent.wx = WcFromTc(TcFromWc(m_wx) + m_pstruc->ctx);
	wptExtent.wy = WcFromTc(TcFromWc(m_wy) + m_pstruc->cty);

	pwpt->wx = WcTrunc(m_wx + (wptExtent.wx - m_wx) / 2) + kwcTileHalf;
	pwpt->wy = WcTrunc(m_wy + (wptExtent.wy - m_wy) / 2) + kwcTileHalf;

	TCoord txTest = TcFromWc(pwpt->wx);
	TCoord tyTest = TcFromWc(pwpt->wy);
	if (IsAccessible(txTest, tyTest))
		return;

	// Otherwise find a spot that is accessible

	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);
	for (TCoord tyT = ty; tyT < ty + m_pstruc->cty; tyT++) {
		for (TCoord txT = tx; txT < tx + m_pstruc->ctx; txT++) {
			if (txT != txTest && tyT != tyTest) {
				if (IsAccessible(txT, tyT)) {
					pwpt->wx = WcFromTc(txT) + kwcTileHalf;
					pwpt->wy = WcFromTc(tyT) + kwcTileHalf;
					return;
				}
			}
		}
	}

	// Couldn't find anything that is "accessible" unfortunately. Try to find something close by
	// that is accessible

	FindNearestFreeTile(TcFromWc(pwpt->wx), TcFromWc(pwpt->wy), pwpt, kbfStructure);
}

void StructGob::DrawSymbol(TBitmap *ptbm, DibBitmap *pbm, int xViewOrigin, int yViewOrigin)
{
	int x = -xViewOrigin + PcFromWc(m_wx);
	int y = -yViewOrigin + PcFromWc(m_wy);

	Rect rcT;
	rcT.FromWorldRect(&m_puntc->wrcUIBounds);
	Size sizT;
	ptbm->GetSize(&sizT);
	ptbm->BltTo(pbm, x + (rcT.Width() - sizT.cx) / 2, y + (rcT.Height() - sizT.cy) / 2);
}

void StructGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	switch (nLayer) {
	case knLayerMiniMap:
		{
			Color clr;
			if (m_ff & kfGobSelected)
				clr = GetColor(kiclrWhite);
			else 
				clr = GetSideColor(m_pplr->GetSide());

			int cxy = gsim.GetMiniMapScale();
			pbm->Fill(xViewOrigin + MmcFromWc(m_wx), yViewOrigin + MmcFromWc(m_wy), m_pstruc->ctx * cxy, m_pstruc->cty * cxy,
					clr);
		}
		return;

	case knLayerSymbols:

		// only one symbol shows at a time. Low Power can alternate with repairing
		// or with need credits, but repairing and need credits are mutually exclusive

		UnitGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);
		if (m_wfUnit & kfUnitDrawNeedsPowerSymbol) {
			DrawSymbol(s_ptbmNeedsPower, pbm, xViewOrigin, yViewOrigin);
		} else {
			if (m_wfUnit & kfUnitDrawRepairingSymbol) {
				DrawSymbol(s_ptbmRepairing, pbm, xViewOrigin, yViewOrigin);
			} else if (m_wfUnit & kfUnitDrawNeedCreditsSymbol) {
				DrawSymbol(s_ptbmNeedCredits, pbm, xViewOrigin, yViewOrigin);
			}
		}
		break;

	case knLayerSelection:
		if ((m_ff & (kfGobSelected | kfGobBeingBuilt)) == kfGobBeingBuilt) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
			DrawHealthIndicator(pbm, &rcT, m_fxHealth, m_puntc->GetArmorStrength());
			return;
		}

		// fall through

	default:
		UnitGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);
		break;
	}
}

void StructGob::DefUpdate()
{
	// If we're repairing we'll skip no updates, even if blocked on credits. If we're blocked
	// on low power then we'll skip till we get to the next symbol flash. However, most of the time 
	// we will be none of these, so GetUpdateCount and do the math for cupdUntilFlash inside the tests
	// for symbol situations rather than on every single update.

	int cUpdates;
	int cupdUntilFlash;

	if ((m_puntc->wf & kfUntcNotifyPowerLowHigh) && m_pplr->IsPowerLow() && (m_ff & kfGobActive)) {
		cUpdates = gsim.GetUpdateCount();
		cupdUntilFlash = kcupdSymbolFlashRate - (cUpdates % kcupdSymbolFlashRate);

		if (cupdUntilFlash == kcupdSymbolFlashRate) {

			// Low Power will flash ON for odd intervals
			
			if ((short)(cUpdates / kcupdSymbolFlashRate) & 1)
				m_wfUnit |= kfUnitDrawNeedsPowerSymbol;
			else
				m_wfUnit &= ~kfUnitDrawNeedsPowerSymbol;

			EnableOrDisableSymbolLayer();
			if (gwfPerfOptions & kfPerfSymbolFlashing)
				MarkRedraw();
		}
		m_unvl.MinSkip(cupdUntilFlash - 1);
	}

	// If repairing and sufficient credits are available

	if (m_wfUnit & kfUnitRepairing) {
		cUpdates = gsim.GetUpdateCount();
		cupdUntilFlash = kcupdSymbolFlashRate - (cUpdates % kcupdSymbolFlashRate);

		int nCreditsHave = m_pplr->GetCredits();

		if (cupdUntilFlash == kcupdSymbolFlashRate) {
			if (nCreditsHave > 0) { 

				// Repair will flash OFF for odd intervals
				
				if ((short)(cUpdates / kcupdSymbolFlashRate) & 1)
					m_wfUnit &= ~kfUnitDrawRepairingSymbol;
				else
					m_wfUnit |= kfUnitDrawRepairingSymbol;

			} else { 
				m_wfUnit &= ~kfUnitDrawRepairingSymbol;
			}
			EnableOrDisableSymbolLayer();
			if (gwfPerfOptions & kfPerfSymbolFlashing)
				MarkRedraw();
		}

		fix fxArmorStrength = m_pstruc->GetArmorStrength();
		if (m_fxHealth < fxArmorStrength) {

			// n health unit costs m credits to repair

			int nCreditsNeeded = knCreditsPerRepairUpdate;

			// Take whatever is left (repair on a discount!)

			if (nCreditsHave != 0 && nCreditsHave < nCreditsNeeded)
				nCreditsNeeded = nCreditsHave;

			if (nCreditsHave >= nCreditsNeeded) {

				fix fxHealth = addfx(m_fxHealth, kfxHealthUnitsRepairedPerUpdate);
				m_pplr->SetCredits(nCreditsHave - nCreditsNeeded, true, knConsumerRepair);

				// make sure we're not in need credits mode

				WaitingForCredits(false);

				// At full strength? If so, stop repairing

				if (fxHealth >= fxArmorStrength) {
					fxHealth = fxArmorStrength;
					m_wfUnit &= ~(kfUnitRepairing | kfUnitDrawRepairingSymbol);
					Assert(!(m_wfUnit & kfUnitNeedCredits), "finished a repair without credits?"); 
					EnableOrDisableSymbolLayer();
					MarkRedraw();
				}
				SetHealth(fxHealth);
			} else {
				WaitingForCredits(true);
			}
		} else {
			m_wfUnit &= ~(kfUnitRepairing | kfUnitDrawRepairingSymbol);
			Assert(!(m_wfUnit & kfUnitNeedCredits), "finished a repair without credits?"); 
			EnableOrDisableSymbolLayer();
			MarkRedraw();
		}

        // when repairing we need every update to do our repair increments or
        // to check for a fresh cash infusion (there is no credit change
        // notification and because building can block w/o credits at zero it's
        // too complex for now)

		m_unvl.MinSkip();
	}

	if (m_wfUnit & kfUnitNeedCredits) {
		cUpdates = gsim.GetUpdateCount();
		cupdUntilFlash = kcupdSymbolFlashRate - (cUpdates % kcupdSymbolFlashRate);
		
		if (cupdUntilFlash == kcupdSymbolFlashRate) {

			// NeedCredits will flash OFF for odd intervals
			
			if ((short)(cUpdates / kcupdSymbolFlashRate) & 1)
				m_wfUnit &= ~kfUnitDrawNeedCreditsSymbol;
			else
				m_wfUnit |= kfUnitDrawNeedCreditsSymbol;

			EnableOrDisableSymbolLayer();
			if (gwfPerfOptions & kfPerfSymbolFlashing)
				MarkRedraw();
		}

		// again, we have no credit infusion update, and so far all things that 
		// can block on credits get every update while credit consuming

		m_unvl.MinSkip();
	}

	UnitGob::DefUpdate();
}

void StructGob::EnableOrDisableSymbolLayer()
{
	dword ffOld = m_ff;

	if (!IsAlly(gpplrLocal->GetSide()) && ((gpplrLocal->GetHandicap() & kfHcapShowEnemyResourceStatus) == 0))
		m_wfUnit &= ~(kfUnitDrawNeedsPowerSymbol | kfUnitDrawNeedCreditsSymbol);

	if (m_wfUnit & (kfUnitDrawRepairingSymbol | kfUnitDrawNeedsPowerSymbol | kfUnitDrawNeedCreditsSymbol)) {
		m_ff |= kfGobLayerSymbols;
	} else {
		m_ff &= ~kfGobLayerSymbols;
	}

	// Changed?

	if ((ffOld ^ m_ff) & kfGobLayerSymbols) {
		if (gwfPerfOptions & kfPerfSymbolFlashing)
			MarkRedraw();
	}
}

bool StructGob::NeedsRepair()
{
	return m_fxHealth < m_pstruc->GetArmorStrength();
}

bool StructGob::IsTakeoverable(Player *pplr)
{
	// Can only takeover if the player has limit space

	if (!ggobm.IsBelowLimit(knLimitStruct, pplr)) {
		if (pplr == gpplrLocal)
			ShowAlert(kidsBuildingLimitReached);
		return false;
	}

	if (m_ff & kfGobActive)
		return true;
	return false;
}

void StructGob::Takeover(Player *pplr)
{
	// Removes any player-specific influence this Structure has

	Deactivate();

	// Change owners

	Assert(ggobm.IsBelowLimit(knLimitStruct, pplr));
	ggobm.TrackGobCounts(this, false);
	m_pplr->IncStructuresLost();
	SetOwner(pplr);
	m_pplr->IncEnemyStructuresKilled();
	ggobm.TrackGobCounts(this, true);

	// Applies any player-specific influence this Structure has

	Activate();

	// Reveal fog around this newly owned structure

	if (pplr == gpplrLocal) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);
		FogMap *pfogm = gsim.GetLevel()->GetFogMap();
		WPoint wpt;
		GetCenter(&wpt);
		RevealPattern *prvlp = (RevealPattern *)(m_pstruc->wf & kfUntcLargeDefog ? grvlpLarge : grvlp);
		pfogm->Reveal(TcFromWc(wpt.wx), TcFromWc(wpt.wy), prvlp, gpupdSim, wxView, wyView);
	}

	// force an update so it gets in sync on lowpower/credits etc

	m_unvl.MinSkip();

	// Redraw this part of the minimap

	TRect trc;
	GetTileRect(&trc);
	gpmm->RedrawTRect(&trc);
}

// pplr is used for the replicator do do a custom override

void StructGob::WaitingForCredits(bool fNeed, bool fOverride, Player *pplr)
{
	// override is for LoadState

	if ((fNeed == ((m_wfUnit & kfUnitNeedCredits) == kfUnitNeedCredits)) && !fOverride)
		return;

	int cDelta;
	if (fNeed) {
		m_wfUnit |= kfUnitNeedCredits;
		cDelta = 1;
	} else {
		m_wfUnit &= ~( kfUnitDrawNeedCreditsSymbol | kfUnitNeedCredits );
		EnableOrDisableSymbolLayer();
		if (gwfPerfOptions & kfPerfSymbolFlashing)
			MarkRedraw();
		cDelta = -1;
	}

	// track how many buildings need credits in the player so the local player can reflect
	// that need in the UI

	if (pplr == NULL)
		pplr = m_pplr;
	pplr->ModifyNeedCreditsCount(cDelta);

	return;
}

void StructGob::OnPowerLowHigh()
{
	if (!GetOwner()->IsPowerLow()) {
		m_wfUnit &= ~kfUnitDrawNeedsPowerSymbol;
		EnableOrDisableSymbolLayer();
		MarkRedraw();
	} else {

		// force an update to start symbol flashing

		m_unvl.MinSkip();
	}
}

void StructGob::Repair(bool fOn)
{
	if (fOn) {
		// If already at max health, don't waste any more time

		if (m_fxHealth == m_pstruc->GetArmorStrength())
			return;

		// toggle on repair state. Icon will be shown in update

		m_wfUnit |= kfUnitRepairing;

		if (m_pplr == gpplrLocal) {
			if (m_pplr->GetCredits() > 0)
				gsndm.PlaySfx(m_pstruc->sfxRepair);
		}

	} else {
		m_wfUnit &= ~(kfUnitRepairing | kfUnitDrawRepairingSymbol | kfUnitDrawNeedCreditsSymbol);
		WaitingForCredits(false);
		if (m_pplr == gpplrLocal)
			gsndm.PlaySfx(m_pstruc->sfxAbortRepair);
	}

	m_unvl.MinSkip();
}

bool StructGob::IsValidTarget(Gob *pgobTarget)
{
	// most structures cannot attack. Towers can so they override this.

	return false;	
}

void StructGob::InitMenu(Form *pfrm)
{
	ButtonControl *pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcRepair);
	pbtn->Enable(NeedsRepair());
	pbtn->Show((m_wfUnit & kfUnitRepairing) == 0);

	pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcAbortRepair);
	pbtn->Show((m_wfUnit & kfUnitRepairing) != 0);
}

void StructGob::OnMenuItemSelected(int idc)
{
	switch (idc) {

	// RepairCommand acts as a toggle

	case kidcRepair:
	case kidcAbortRepair:
		gcmdq.Enqueue(kmidRepairCommand, m_gid);
		break;

	case kidcSelfDestruct:
		if (PopupConfirmation("SELL BUILDING"))
			gcmdq.Enqueue(kmidSelfDestructCommand, m_gid);
		break;
	}
}

void StructGob::SelfDestruct(bool fRecycleValue) 
{
	// Refund half the value of the structure if recycling

	if (fRecycleValue) {
		m_pplr->SetCredits(m_pplr->GetCredits() + (m_pstruc->GetCost() / 2), true);

		// Only do this for 'sold' structures. Structures lost while being built don't count as 'lost'

		m_pplr->IncStructuresLost();
	}
	SetState(kstDying);
}

void StructGob::SetHealth(fix fxHealth)
{
	UnitGob::SetHealth(fxHealth);

	if (m_ff & kfGobBeingBuilt)
		return;

	if (m_fxHealth * 2 < m_pstruc->GetArmorStrength()) {
		if (m_ani.GetStrip() != 1)
			StartAnimation(&m_ani, 1, 0, m_ani.GetFlags());
	} else {
		if (m_ani.GetStrip() != 0)
			StartAnimation(&m_ani, 0, 0, m_ani.GetFlags());
	}
}

ScorchGob *StructGob::GetScorchGob()
{
	if (ggobm.IsBelowLimit(knLimitScorch))
		return new ScorchGob;

	// Find the oldest scorch and delete it

	int nSequenceOldest = 0x7fff;
	ScorchGob *pgobScorchOldest = NULL;
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->GetType() != kgtScorch)
			continue;
		ScorchGob *pgobScorch = (ScorchGob *)pgobT;
		int nSequence = pgobScorch->GetSequence();
		if (nSequence < nSequenceOldest) {
			nSequenceOldest = nSequence;
			pgobScorchOldest = pgobScorch;
		}
	}

	Assert(pgobScorchOldest != NULL);
	if (pgobScorchOldest != NULL) {
		ggobm.RemoveGob(pgobScorchOldest);
		delete pgobScorchOldest;
	}

	return new ScorchGob;
}

int StructGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	OnMsg(kmidPowerLowHigh)
		OnPowerLowHigh();

	OnMsg(kmidHit)
		int fxHealthPrev = m_fxHealth;

		// apply damage

		fix fxDamage = itofx(pmsg->Hit.nDamage);
		if (m_pplr->GetHandicap() & kfHcapIncreasedArmor)
			fxDamage = (fix)mulfx(fxDamage, (itofx(knDecreasedDamagePercent) / 100));
		SetHealth(subfx(m_fxHealth, fxDamage));
		if (m_fxHealth <= 0) {
			Player *pplr = gplrm.GetPlayer(pmsg->Hit.sideAssailant);
			pplr->IncEnemyStructuresKilled();
			m_pplr->IncStructuresLost();

			SetState(kstDying);

		} else {
			ShowDamageIndicator();

			// If health is dipping into the severely damaged zone, fire off
			// a couple smoke animations.

			if (fxHealthPrev * 2 >= m_pstruc->GetArmorStrength() && m_fxHealth * 2 < m_pstruc->GetArmorStrength()) {

				// Play damaged sound

				gsndm.PlaySfx(m_pstruc->sfxDamaged);

				if (gsim.GetTickCount() - m_tLastSmoke > 200) {
					int cSmokes = (GetRandom() % 3) + 2;
					if (m_pstruc->ctx == 1 || m_pstruc->cty == 1)
						cSmokes /= 2;
					for (int i = 0; i < cSmokes; i++)
						gsmm.SendDelayedMsg(kmidSpawnSmoke, GetRandom() & 63, m_gid, m_gid);

					m_tLastSmoke = gsim.GetTickCount();
				}
			}
		}

		// Remember that a structure has been attacked

		m_pplr->SetFlags(m_pplr->GetFlags() | kfPlrStructureAttacked);

		// Notify nearby allies that we've been hit! CONSIDER: pass in an expanded range?

		NotifyNearbyAlliesOfHit(pmsg->Hit.gidAssailant);

	OnMsg(kmidSpawnSmoke)
		if (gwfPerfOptions & kfPerfSmoke) {
			if (ggobm.IsBelowLimit(knLimitSupport)) {
				SmokeGob *pgob = new SmokeGob((GetRandom() & 1));
				Assert(pgob != NULL, "out of memory!");
				if (pgob != NULL) {
					WCoord wcx = WcFromTc(m_pstruc->ctx);
					WCoord wcy = WcFromTc(m_pstruc->cty) - kwcTileHalf;
					WCoord wdx = GetRandom() % wcx;
					WCoord wdy = (GetRandom() % wcy) + WcFromTile16ths(12);
					WCoord wxSmoke = m_wx + wdx;
					WCoord wySmoke = m_wy + wdy;

					Size wsizMap;
					gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&wsizMap);
					wsizMap.cx = WcFromTc(wsizMap.cx);
					wsizMap.cy = WcFromTc(wsizMap.cy);

					if (wxSmoke < 0)
						wxSmoke = 0;
					if (wxSmoke >= wsizMap.cx)
						wxSmoke = wsizMap.cx - 1;
					if (wySmoke >= wsizMap.cy)
						wySmoke = wsizMap.cy - 1;

					if (!pgob->Init(wxSmoke, wySmoke, m_gid))
						delete pgob;
				}
			}
		}

	OnMsg(kmidSelfDestructCommand)

		// assume we only get this message when user has
		// initiated a recycle. Game can call the method.
		SelfDestruct();

	OnMsg(kmidRepairCommand)

		// Toggle repairing

		Repair(!(m_wfUnit & kfUnitRepairing));

	OnMsg(kmidDelete)
		Assert("Shouldn't receive kmidDelete when not in kstDying state");

	//-----------------------------------------------------------------------

	State(kstBeingBuilt)
		OnEnter
			m_ff |= kfGobLayerSelection;

		OnExit
			m_ff &= ~kfGobLayerSelection;

		OnMsg(kmidBuildComplete)
			Activate();
			SetState(kstIdle);

		OnUpdate
			// Don't advance animation

 			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstIdle)
		OnEnter
			// Play idle animation, chosen based on the health of the Structure

			int nStrip;
			if (m_fxHealth == 0)
				nStrip = 2; // destroyed
			else if (m_fxHealth * 2 < m_pstruc->GetArmorStrength())
				nStrip = 1; // damaged
			else
				nStrip = 0; // healthy
			StartAnimation(&m_ani, nStrip, 0, kfAniIgnoreFirstAdvance | kfAniLoop);

		OnUpdate
			AdvanceAnimation(&m_ani);
			DefUpdate();

	//-----------------------------------------------------------------------

	State(kstDying)
		OnEnter
			// if you're destroyed during building it's possible to get here unactivated

			if (m_ff & kfGobActive)
				Deactivate();
			m_ff ^= kfGobLayerDepthSorted | kfGobLayerSurfaceDecal;

			gsndm.PlaySfx(m_pstruc->sfxDestroyed);
			StartAnimation(&m_ani, 2, 0, kfAniIgnoreFirstAdvance);	// UNDONE: hardcoded

			// dont remove ourselves from the "occupied" map until it's all over
			// so troops can't run across this freshly destroyed area while it's hot (they have cheap boots)

			gsmm.SendDelayedMsg(kmidDelete, 800, m_gid, m_gid);

			// Show a huge explosion!

			WCoord wcx = WcFromTc(m_pstruc->ctx);
			WCoord wcy = WcFromTc(m_pstruc->cty) + kwcTileHalf;
			Gob *pgobExpl = CreateAnimGob(m_wx + (wcx / 2), m_wy + (wcy / 2), kfAnmDeleteWhenDone | kfAnmSmokeFireLayer, NULL, 
					s_panidStructureExplosion);
			if (pgobExpl != NULL)
				pgobExpl->SetOwner(m_pplr);

			// Show some smoke (in the near future)

			int cSmokes = (GetRandom() % 3) + 1;
			int i;
			for (i = 0; i < cSmokes; i++)
				gsmm.SendDelayedMsg(kmidSpawnSmoke, GetRandom() & 63, m_gid, m_gid);

			// Lay down a random smattering of scorch marks

			if (gwfPerfOptions & kfPerfScorchMarks) { 
				int cScorches = (GetRandom() & 3) + 1;
				WCoord wcxScorchable = WcFromTc(m_pstruc->ctx);
				WCoord wcyScorchable = WcFromTc(m_pstruc->cty);

				Size wsizMap;
				gsim.GetLevel()->GetTileMap()->GetTCoordMapSize(&wsizMap);
				wsizMap.cx = WcFromTc(wsizMap.cx);
				wsizMap.cy = WcFromTc(wsizMap.cy);

				for (i = 0; i < cScorches; i++) {

					// Pick a random scorch type to lay down but make sure it is
					// one that is smaller than the structure leaving it.

					Size sizScorch;
					WCoord wcxScorch, wcyScorch;
					int nScorch;
					do {
						nScorch = GetRandom() % (sizeof(gaptbmScorches) / sizeof(TBitmap *));
						gaptbmScorches[nScorch]->GetSize(&sizScorch);
						wcxScorch = WcFromUpc(sizScorch.cx);
						wcyScorch = WcFromUpc(sizScorch.cy);
					} while (wcxScorch > wcxScorchable || wcyScorch > wcyScorchable);

					WCoord wxoff = GetRandom() % (wcxScorchable - wcxScorch + 1);
					WCoord wyoff = GetRandom() % (wcyScorchable - wcyScorch + 1);

					ScorchGob *pgobScorch = GetScorchGob();
					if (pgobScorch != NULL) {
						// Make sure the scorch stays within the map bounds

						WCoord wxScorch = m_wx + wxoff + (wcxScorch / 2);
						WCoord wyScorch = m_wy + wyoff + wcyScorch;
						if (wxScorch < 0)
							wxScorch = 0;
						if (wxScorch >= wsizMap.cx)
							wxScorch = wsizMap.cx - 1;
						if (wyScorch >= wsizMap.cy)
							wyScorch = wsizMap.cy - 1;

						pgobScorch->Init(wxScorch, wyScorch, nScorch);
					}
				}
			}

		OnUpdate
			AdvanceAnimation(&m_ani);

		OnMsg(kmidDelete)
			Delete();
			return knDeleted;

		// Do this so the kmidSpawnSmoke message will bubble up to the 'global' handler above

		OnMsg(kmidSpawnSmoke)
			return knNotHandled;

		// Eat all other messages (e.g., kmidHit, kmidNearbyAllyHit)

		DiscardMsgs

	//-----------------------------------------------------------------------

EndStateMachine
}

//
// SmokeGob implementation
//

SmokeGob::SmokeGob(int cLoops)
{
	m_cLoops = cLoops;
}

bool SmokeGob::Init(WCoord wx, WCoord wy, StateMachineId smidNotify)
{
	return AnimGob::Init(wx, wy, kfAnmLoop | kfAnmSmokeFireLayer, NULL, s_panidBigSmoke, 0, smidNotify, NULL);
}

#define knVerSmokeGobState 1
bool SmokeGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerSmokeGobState)
		return false;
	m_cLoops = pstm->ReadWord();
	m_ani.Init(s_panidBigSmoke);
	m_ani.LoadState(pstm);
	m_smidNotify = pstm->ReadWord();
	m_wfAnm = kfAnmLoop | kfAnmSmokeFireLayer;
	return AnimGob::LoadState(pstm);
}

bool SmokeGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerSmokeGobState);
	pstm->WriteWord(m_cLoops);
	m_ani.SaveState(pstm);
	pstm->WriteWord(m_smidNotify);
	return AnimGob::SaveState(pstm);
}

bool SmokeGob::IsSavable()
{
	// Because AnimGob is not savable, we need to override

	return true;
}

GobType SmokeGob::GetType()
{
	return kgtSmoke;
}

// After the initial set (smoke growing) completes, draw the
// cycle a few times then destroy self.

bool SmokeGob::OnStripDone()
{
	if (m_ani.GetStrip() == 0) {
		SetAnimationStrip(&m_ani, 1);
	} else {
		m_cLoops--;
		if (m_cLoops < 0)
			return true;
	}
	return false;
}

} // namespace wi
