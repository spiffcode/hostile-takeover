#include "ht.h"

namespace wi {

TBitmap *gaptbmScorches[4];
UnitConsts *gapuntc[kutMax];
int gnUnitCostMin = 32767;
int gnUnitCostMax = -1;
int gnUnitCostMPMin = 32767;
int gnUnitCostMPMax = -1;
fix gfxMobileUnitArmorStrengthMin = itofx(511);
fix gfxMobileUnitArmorStrengthMax = itofx(-1);
fix gfxStructureArmorStrengthMin = itofx(511);
fix gfxStructureArmorStrengthMax = itofx(-1);
fix gfxInfantryDamageMin = itofx(511);
fix gfxInfantryDamageMax = itofx(-1);
fix gfxVehicleDamageMin = itofx(511);
fix gfxVehicleDamageMax = itofx(-1);
fix gfxStructureDamageMin = itofx(511);
fix gfxStructureDamageMax = itofx(-1);
TCoord gtcFiringRangeMin = 127;
TCoord gtcFiringRangeMax = -1;
WCoord gwcMoveDistPerUpdateMin = 32767;
WCoord gwcMoveDistPerUpdateMax = -1;
int gnPowerSupplyMin = 32767;
int gnPowerSupplyMax = -1;
int gnPowerDemandMin = 32767;
int gnPowerDemandMax = -1;

UnitConsts *GetUnitConsts(GobType gt)
{
	for (int i = 0; i < kutMax; i++) {
		if (gapuntc[i]->gt == gt)
			return gapuntc[i];
	}

	return NULL;
}

void GetPrerequisiteString(char *psz, UnitConsts *puntc)
{
	*psz = 0;
	int i;
	for (i = 0; i < kutMax; i++) {
		if (puntc->umPrerequisites & (1UL << i)) {
			if (*psz != 0)
				strcat(psz, ", ");
			strcat(psz, gapuntc[i]->szLongName);
		}
	}

	for (i = 0; i < kupgtMax; i++) {
		if (puntc->upgmPrerequisites & (1UL << i)) {
			if (*psz != 0)
				strcat(psz, ", ");
			strcat(psz, gaupg[i].szName);
			strcat(psz, " upgrade");
		}
	}
}

//===========================================================================
// UnitGob implementation

bool UnitGob::InitClass(UnitConsts *puntc, IniReader *pini)
{
	gapuntc[puntc->ut] = puntc;
	puntc->um = 1L << puntc->ut;

	char szTemplate[10];
	itoa(puntc->gt, szTemplate, 10);

	puntc->nInfantryDamage = 0;
	puntc->nVehicleDamage = 0;
	puntc->nStructureDamage = 0;
	puntc->nInfantryDamageMP = 0;
	puntc->nVehicleDamageMP = 0;
	puntc->nStructureDamageMP = 0;
	puntc->ctFiringRate = 0;
	puntc->tcFiringRange = 0;

	// Required properties

	if (pini->GetPropertyValue(szTemplate, "Cost", "%d", &puntc->nCost) != 1)
		return false;
	gnUnitCostMin = _min(gnUnitCostMin, puntc->nCost);
	gnUnitCostMax = _max(gnUnitCostMax, puntc->nCost);

 	int csecTimeToBuild;
 	if (pini->GetPropertyValue(szTemplate, "TimeToBuild", "%d", &csecTimeToBuild) != 1)
		return false;
 	puntc->cupdTimeToBuild = UpdFromSec(csecTimeToBuild);
	if (pini->GetPropertyValue(szTemplate, "TimeToBuildMP", "%d", &csecTimeToBuild) != 1)
		puntc->cupdTimeToBuildMP = puntc->cupdTimeToBuild;
	else
		puntc->cupdTimeToBuildMP = UpdFromSec(csecTimeToBuild);
	int pcT;
	if (pini->GetPropertyValue(szTemplate, "UIBoundsLeft", "%d", &pcT) != 1)
		return false;
	puntc->wrcUIBounds.left = pcT * kwcTile16th; // HACK: so we can keep existing GobTemplate UIBounds
	if (pini->GetPropertyValue(szTemplate, "UIBoundsTop", "%d", &pcT) != 1)
		return false;
	puntc->wrcUIBounds.top = pcT * kwcTile16th; // HACK: so we can keep existing GobTemplate UIBounds
	if (pini->GetPropertyValue(szTemplate, "UIBoundsRight", "%d", &pcT) != 1)
		return false;
	puntc->wrcUIBounds.right = pcT * kwcTile16th; // HACK: so we can keep existing GobTemplate UIBounds
	if (pini->GetPropertyValue(szTemplate, "UIBoundsBottom", "%d", &pcT) != 1)
		return false;
	puntc->wrcUIBounds.bottom = pcT * kwcTile16th; // HACK: so we can keep existing GobTemplate UIBounds
	if (pini->GetPropertyValue(szTemplate, "SortOffset", "%d", &puntc->wdySortOffset) != 1)
		return false;
	puntc->wdySortOffset *= 16;		// HACK: so we can keep existing GobTemplate sort offsets
	if (pini->GetPropertyValue(szTemplate, "Animation", "%s", &puntc->szAniName) != 1)
		return false;
	if (pini->GetPropertyValue(szTemplate, "Name", "%s", puntc->szName) != 1) {
		Assert("Struct/Unit must have 'Name' property");
		return false;
	}
	Assert(strlen(puntc->szName) < kcbStructUnitName);

	// Optional properties

	int cmsFiringRate; 
	if (pini->GetPropertyValue(szTemplate, "FiringRate", "%d", &cmsFiringRate) == 1)
		puntc->ctFiringRate = cmsFiringRate / 10;

	fix fxDamagePerSec;
	if (pini->GetPropertyValue(szTemplate, "InfantryDamage", "%d", &puntc->nInfantryDamage)) {
		if (puntc->nInfantryDamage != 0) {
			fxDamagePerSec = (fix)mulfx(itofx32(puntc->nInfantryDamage), divfx(itofx(1000), itofx(cmsFiringRate)));
			gfxInfantryDamageMin = _min(gfxInfantryDamageMin, fxDamagePerSec);
			gfxInfantryDamageMax = _max(gfxInfantryDamageMax, fxDamagePerSec);
		}
	}
	if (pini->GetPropertyValue(szTemplate, "InfantryDamageMP", "%d", &puntc->nInfantryDamageMP) != 1)
		puntc->nInfantryDamageMP = puntc->nInfantryDamage;
	if (pini->GetPropertyValue(szTemplate, "VehicleDamage", "%d", &puntc->nVehicleDamage)) {
		if (puntc->nVehicleDamage != 0) {
			fxDamagePerSec = (fix)mulfx(itofx32(puntc->nVehicleDamage), divfx(itofx(1000), itofx(cmsFiringRate)));
			gfxVehicleDamageMin = _min(gfxVehicleDamageMin, fxDamagePerSec);
			gfxVehicleDamageMax = _max(gfxVehicleDamageMax, fxDamagePerSec);
		}
	}
	if (pini->GetPropertyValue(szTemplate, "VehicleDamageMP", "%d", &puntc->nVehicleDamageMP) != 1)
		puntc->nVehicleDamageMP = puntc->nVehicleDamage;
	if (pini->GetPropertyValue(szTemplate, "StructureDamage", "%d", &puntc->nStructureDamage)) {
		if (puntc->nStructureDamage != 0) {
			fxDamagePerSec = (fix)mulfx(itofx32(puntc->nStructureDamage), divfx(itofx(1000), itofx(cmsFiringRate)));
			gfxStructureDamageMin = _min(gfxStructureDamageMin, fxDamagePerSec);
			gfxStructureDamageMax = _max(gfxStructureDamageMax, fxDamagePerSec);
		}
	}
	if (pini->GetPropertyValue(szTemplate, "StructureDamageMP", "%d", &puntc->nStructureDamageMP) != 1)
		puntc->nStructureDamageMP = puntc->nStructureDamage;

	pini->GetPropertyValue(szTemplate, "FiringRange", "%d", &puntc->tcFiringRange);
	gtcFiringRangeMin = _min((int)gtcFiringRangeMin, puntc->tcFiringRange);
	gtcFiringRangeMax = _max((int)gtcFiringRangeMax, puntc->tcFiringRange);

	char szT[300];
#ifdef DEBUG
	szT[sizeof(szT) - 1] = 0; // guard
#endif
	if (pini->GetPropertyValue(szTemplate, "Description", szT, sizeof(szT))) {

		// We dynamically allocate szDescription to save .bss space

		Assert(szT[sizeof(szT) - 1] == 0);
		puntc->szDescription = new char[strlen(szT) + 1];
		if (puntc->szDescription != NULL)
			strcpy(puntc->szDescription, szT);
	}

	if (!pini->GetPropertyValue(szTemplate, "LongName", puntc->szLongName, sizeof(puntc->szLongName)))
		strcpy(puntc->szLongName, puntc->szName);

	if (pini->GetPropertyValue(szTemplate, "CostMP", "%d", &puntc->nCostMP) != 1)
		puntc->nCostMP = puntc->nCost;
	gnUnitCostMPMin = _min(gnUnitCostMPMin, puntc->nCostMP);
	gnUnitCostMPMax = _max(gnUnitCostMPMax, puntc->nCostMP);

	puntc->panid = LoadAnimationData(puntc->szAniName);
	Assert(puntc->panid != NULL);
	if (puntc->panid == NULL)
		return false;

    InitFingerUIBounds(puntc);

	// If they aren't already loaded, load the scorch marks

	if (gaptbmScorches[0] == NULL) {
		gaptbmScorches[0] = LoadTBitmap("scorch_8x8.tbm");
		gaptbmScorches[1] = LoadTBitmap("scorch_16x16.tbm");
		gaptbmScorches[2] = LoadTBitmap("scorch_32x16.tbm");
		gaptbmScorches[3] = LoadTBitmap("scorch_48x48.tbm");
	}

	return true;
}

void UnitGob::InitFingerUIBounds(UnitConsts *puntc)
{
    // Finger UI Bounds inflates the UI bounds for finger hit testing

    WRect wrc = puntc->wrcUIBounds;

    // Make sure height is kwcTile * 4, then adjust width to keep aspect
    // ratio the same

    int cy = wrc.Height();
    int cx = wrc.Width();
    if (cy < kwcTile * 4) {
        cy = kwcTile * 4;
        cx = cx * cy * 64 / wrc.Height() / 64;
    }
    wrc.Inflate((cx - wrc.Width()) / 2, (cy - wrc.Height()) / 2);
    puntc->wrcUIBoundsFinger = wrc;
}

void UnitGob::ExitClass(UnitConsts *puntc)
{
	// If they aren't already deleted, delete the scorch marks

	if (gaptbmScorches[0] != NULL) {
		for (int i = 0; i < sizeof(gaptbmScorches) / sizeof(TBitmap *); i++) {
			delete gaptbmScorches[i];
			gaptbmScorches[i] = NULL;
		}
	}

	delete puntc->panid;
	puntc->panid = NULL;

	delete puntc->pfrmMenu;
	puntc->pfrmMenu = NULL;

	delete puntc->szDescription;
	puntc->szDescription = NULL;
}

UnitGob::UnitGob(UnitConsts *puntc)
{
	m_ff |= kfGobUnit | kfGobStateMachine | kfGobLayerDepthSorted;
	m_puntc = puntc;
	m_wfUnit = 0;
	for (int n = 0; n < ARRAYSIZE(m_agidEnemyNearby); n++)
		m_agidEnemyNearby[n] = kgidNull;

	// Just to be clear this Unit hasn't notified anyone about being hit for awhile

	m_cupdLastHitNotify = -1000000;
    m_panispr = NULL;
}

UnitGob::~UnitGob()
{
    delete m_panispr;
    m_panispr = NULL;
}

bool UnitGob::Init(IniReader *pini, FindProp *pfind, const char *pszName)
{
	int wf = 0;
	int side, tx, ty;
	int nHealth;
	int cArgs = pini->GetPropertyValue(pfind, "%*d ,%d ,%d ,%d ,%d ,%d", &side, &tx, &ty, &wf, &nHealth);
	if (cArgs < 5) {
		Assert("UnitGob requires at least 5 valid initialization parameters");
		return false;
	}

	fix fxHealth = (fix)((m_puntc->GetArmorStrength() * (long)nHealth) / 100L);
	return Init(WcFromTc(tx), WcFromTc(ty), gplrm.GetPlayer(side), fxHealth, wf, pszName);
}

bool UnitGob::Init(WCoord wx, WCoord wy, Player *pplr, fix fxHealth, dword ff, const char *pszName)
{
	// UNDONE: stash Name away somewhere

	m_pplr = pplr;
	m_wx = WcTrunc(wx);
	m_wy = WcTrunc(wy);

	m_ff |= ff;

	m_ani.Init(m_puntc->panid);
	StartAnimation(&m_ani, 0, 0, kfAniLoop | kfAniIgnoreFirstAdvance);

	// Initial health is derived from nArmorStrength

	if (fxHealth == 0)
		m_fxHealth = m_puntc->GetArmorStrength();
	else
		m_fxHealth = fxHealth;

	// Reveal this part of the map

	if (m_pplr == gpplrLocal) {
		WCoord wxView, wyView;
		gsim.GetViewPos(&wxView, &wyView);
		FogMap *pfogm = gsim.GetLevel()->GetFogMap();
		WPoint wpt;
		GetCenter(&wpt);
		RevealPattern *prvlp = (RevealPattern *)(m_puntc->wf & kfUntcLargeDefog ? grvlpLarge : grvlp);
		pfogm->Reveal(TcFromWc(wpt.wx), TcFromWc(wpt.wy), prvlp, gpupdSim, wxView, wyView);
	}

	// Add the fresh Gob to the GobMgr.

	ggobm.AddGob(this);

	// Redraw this part of minimap

	TRect trc;
	GetTileRect(&trc);
	gpmm->RedrawTRect(&trc);

	// Initialize the state machine by sending the first Initialize msg

	gsmm.SendMsg(kmidReservedEnter, m_gid, m_gid);

	return true;
}

void UnitGob::Delete()
{
	// Get trect for minimap redrawing

	TRect trc;
	GetTileRect(&trc);

	// Remove from gid map

	ggobm.RemoveGob(this);
	delete this;

	// Redraw this part of minimap after the gob is gone from gobmgr.

	gpmm->RedrawTRect(&trc);
}

#define knVerUnitGobState 4
bool UnitGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerUnitGobState)
		return false;
	m_fxHealth = (fix)pstm->ReadWord();
	m_wfUnit = pstm->ReadWord();
	pstm->Read(m_agidEnemyNearby, sizeof(m_agidEnemyNearby));
	m_ani.Init(m_puntc->panid);
	m_ani.LoadState(pstm);
	m_cupdLastHitNotify = (long)pstm->ReadDword();
	m_cDamageCountdown = pstm->ReadWord();

	return Gob::LoadState(pstm);
}

bool UnitGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerUnitGobState);
	pstm->WriteWord(m_fxHealth);
	pstm->WriteWord(m_wfUnit);
	pstm->Write(m_agidEnemyNearby, sizeof(m_agidEnemyNearby));
	m_ani.SaveState(pstm);
	pstm->WriteDword(m_cupdLastHitNotify);
	pstm->WriteWord(m_cDamageCountdown);

	return Gob::SaveState(pstm);
}

GobType UnitGob::GetType()
{
	return m_puntc->gt;
}

bool UnitGob::IsAlly(Side side)
{
	if (m_pplr->GetAllies() & GetSideMask(side))
		return true;

	return false;
}

void UnitGob::Activate() 
{
	m_ff |= kfGobActive;
	m_pplr->IncUnitCount(m_puntc->ut);

    // Best place to do this is here, after the unit has been
    // fully built, it is counted as having been built.
    m_pplr->IncUnitBuiltCount(m_puntc->ut);
}

void UnitGob::Deactivate() 
{
	Assert(m_ff & kfGobActive);

	// make sure this unit's menu is not left up

	UnitConsts *puntc = GetUnitConsts(GetType());
	Assert(puntc->pfrmMenu != NULL);
	if (puntc->pfrmMenu->GetOwner() == (UnitGob *)this)
		puntc->pfrmMenu->EndForm(kidcCancel);

	m_ff &= ~(kfGobActive | kfGobDrawFlashed);
	Select(false);
	m_pplr->DecUnitCount(m_puntc->ut);
	ClearDamageIndicator();

    // Delete the highlight sprite

    delete m_panispr;
    m_panispr = NULL;
}

bool UnitGob::IsIdle()
{
	return true;
}

bool UnitGob::IsValidTarget(Gob *pgobTarget)
{
	return (pgobTarget->GetFlags() & (kfGobUnit | kfGobActive)) == (kfGobUnit | kfGobActive)
			&& !IsAlly(pgobTarget->GetSide());
}

void UnitGob::SetTarget(Gid gid, WCoord wxTarget, WCoord wyTarget, WCoord wxCenter, WCoord wyCenter, TCoord tcRange, WCoord wcMoveDistPerUpdate)
{
}

int UnitGob::GetDamageTo(UnitGob *puntTarget)
{
	UnitMask umTarget = puntTarget->GetConsts()->um;
	int nDamage = 0;

	if (umTarget & kumInfantry) {
		nDamage = m_puntc->GetInfantryDamage();
	} else if (umTarget & kumVehicles) {
		nDamage = m_puntc->GetVehicleDamage();
	} else if (umTarget & kumStructures) {
		nDamage = m_puntc->GetStructureDamage();
	}

	if (m_pplr->GetHandicap() & kfHcapIncreasedFirePower)
		nDamage = ((nDamage * (100 + knIncreasedFirePowerPercent)) + 50) / 100; // +50 for rounding

	return nDamage;
}

void UnitGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
    // When a unit is displayed as a corpse we want other units draw on top of
    // it UNDONE: this affects all unit types and structures that don't
    // override ::Draw. It may be better to  do this is a more localized way
    // (e.g., specific units spawn a corpse/SurfaceDecalGob upon death)

	if ((nLayer == knLayerDepthSorted && m_st != kstDying) || 
			(nLayer == knLayerSurfaceDecal && m_st == kstDying)) {

#ifdef DRAW_OCCUPIED_TILE_INDICATOR
		{
			WRect wrcT;
			GetTilePaddedWRect(&wrcT);
			Rect rcT;
			rcT.FromWorldRect(&wrcT);
			rcT.Offset(-xViewOrigin, -yViewOrigin);
			DrawBorder(pbm, &rcT, 1, GetColor(kiclrWhite));
		}
#endif
		Side side = m_pplr->GetSide();
		if (m_ff & kfGobDrawFlashed)
			side = (Side)-1;
		else if (m_ff & kfGobBeingBuilt)
			side = ksideNeutral;
	
        int xT = PcFromUwc(m_wx) - xViewOrigin;
        int yT = PcFromUwc(m_wy) - yViewOrigin;
 		m_ani.Draw(pbm, xT, yT, side);

	} else if (nLayer == knLayerSelection) {
		if (m_ff & kfGobSelected) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
			DrawSelectionIndicator(pbm, &rcT, m_fxHealth, m_puntc->GetArmorStrength());

		// If this Gob was hit recently show its health

		} else if (m_wfUnit & kfUnitDrawHealthIndicator) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx),
                    -yViewOrigin + PcFromWc(m_wy));
			DrawHealthIndicator(pbm, &rcT, m_fxHealth,
                    m_puntc->GetArmorStrength());
		}

        // If this gob is hilighted, draw the hilight indicator
        if (m_wfUnit & kfUnitHilighted) {
            if (m_panispr != NULL) {
                int xT = PcFromUwc(m_wx) - xViewOrigin;
                int yT = PcFromUwc(m_wy) - yViewOrigin;
                m_panispr->SetPosition(xT, yT);
                m_panispr->SetScale((float)(kwcTile * 4) / (float)m_puntc->wrcUIBounds.Height());
                m_panispr->CaptureFrame(this);
            }
        }
	}
}

dword UnitGob::GetSortKey()
{
	return MakeSortKey(m_wy + m_puntc->wdySortOffset, m_gid);
}

// See Gob::GetClippingBounds for a description of what the clipping
// bounds is and how it is used.
// NOTE: the values returned are in pixel coordinates

void UnitGob::GetClippingBounds(Rect *prc)
{
	// Union of animation and selection

	m_ani.GetBounds(prc);

    if (m_ff & kfGobSelected) {
		if (gwfPerfOptions & kfPerfSelectionBrackets) {
			Rect rcSel;
			rcSel.FromWorldRect(&m_puntc->wrcUIBounds);
			rcSel.top -= kcyHealthBar;
			if (m_puntc->wf & kfUntcHasFullnessIndicator)
				rcSel.bottom += kcyFullnessBar;
			prc->Union(&rcSel);
		} else {
			Rect rcSel;
			rcSel.FromWorldRect(&m_puntc->wrcUIBounds);
			prc->top = rcSel.top - kcyHealthBar;
			prc->left = _min(prc->left, rcSel.left);
			prc->right = _max(prc->right, rcSel.right);
			if (m_puntc->wf & kfUntcHasFullnessIndicator)
				prc->bottom = _max(prc->bottom, rcSel.bottom + kcyFullnessBar);
		}
	} else if (m_ff & (kfGobBeingUpgraded | kfGobBeingBuilt) || (m_wfUnit & kfUnitDrawHealthIndicator)) {

		// Health is drawn above gob when being upgraded or built 
		// or if it has been recently damaged

		Rect rcSel;
		rcSel.FromWorldRect(&m_puntc->wrcUIBounds);
		prc->top = rcSel.top - kcyHealthBar;
		prc->left = _min(prc->left, rcSel.left);
		prc->right = _max(prc->right, rcSel.right);
	}

	prc->Offset(PcFromUwc(m_wx), PcFromUwc(m_wy));
}

// See Gob::GetUIBounds for a description of what the UI bounds
// is and how it is used.
// NOTE: the values returned are in world coordinates

void UnitGob::GetUIBounds(WRect *pwrc)
{
	// Fudging to make it look/feel good.
	// These fudge-factors are read from GobTemplates.ini

	pwrc->left = m_wx + m_puntc->wrcUIBounds.left;
	pwrc->top = m_wy + m_puntc->wrcUIBounds.top;
	pwrc->right = m_wx + m_puntc->wrcUIBounds.right;
	pwrc->bottom = m_wy + m_puntc->wrcUIBounds.bottom;
}

// UNDONE: OPT: not necessarily the fastest way to return the center

void UnitGob::GetCenter(WPoint *pwpt)
{
	pwpt->wx = m_wx + m_puntc->wrcUIBounds.left + (m_puntc->wrcUIBounds.right - m_puntc->wrcUIBounds.left) / 2;
	pwpt->wy = m_wy + m_puntc->wrcUIBounds.top + (m_puntc->wrcUIBounds.bottom - m_puntc->wrcUIBounds.top) / 2;
}

void UnitGob::GetAttackPoint(WPoint *pwpt)
{
	GetCenter(pwpt);
}

dword UnitGob::GetAnimationHash()
{
    int nFrame = m_ani.GetFrame();
    int nStrip = m_ani.GetStrip();
    return ((nFrame << 16) | nStrip) ^ (dword)(unsigned long)this ^ (GetId() << 16);
}

void UnitGob::GetAnimationBounds(Rect *prc, bool fBase)
{
    m_ani.GetBounds(prc);
}

void UnitGob::DrawAnimation(DibBitmap *pbm, int x, int y)
{
    m_ani.Draw(pbm, x, y, m_pplr->GetSide());
}

void UnitGob::Select(bool fSelect)
{
	// Set bits

	if (fSelect) {
		if (m_ff & kfGobSelected) {
			return;
        }
		m_ff |= kfGobSelected | kfGobLayerSelection;
	} else {
		if (!(m_ff & kfGobSelected)) {
			return;
        }
        m_ff &= ~kfGobSelected;
        if ((m_wfUnit & kfUnitHilighted) == 0) {
            m_ff &= ~kfGobLayerSelection;
        }
	}
	MarkRedraw();

	// Redraw this part of minimap

	TRect trc;
	GetTileRect(&trc);
	gpmm->RedrawTRect(&trc);
}

void UnitGob::Hilight(bool fHilight)
{
    if (fHilight) {
        if (m_wfUnit & kfUnitHilighted) {
            return;
        }
        m_wfUnit |= kfUnitHilighted;
        m_ff |= kfGobLayerSelection;
        m_panispr = CreateHilightSprite();

        // Only need to do this so the sprite captures the current frame
        MarkRedraw();
    } else {
        if (!(m_wfUnit & kfUnitHilighted)) {
            return;
        }
        m_wfUnit &= ~kfUnitHilighted;
        if ((m_ff & kfGobSelected) == 0) {
            m_ff &= ~kfGobLayerSelection;
        }
        delete m_panispr;
        m_panispr = NULL;
    }
}

AnimSprite *UnitGob::CreateHilightSprite()
{
    AnimSprite *panispr = gpsprm->CreateAnimSprite();
    if (panispr != NULL) {
        panispr->SetPalette(gsim.GetLevel()->GetPalette());
    }
    return panispr;
}

void UnitGob::SetHealth(fix fxHealth)
{
	if (m_wfUnit & kfUnitInvulnerable)
		return;

	if (m_fxHealth != fxHealth) {
		m_fxHealth = fxHealth;
		if ((m_ff & (kfGobBeingBuilt | kfGobSelected)) || (m_wfUnit & kfUnitDrawHealthIndicator))
			MarkRedraw();
	}
}

void UnitGob::DefUpdate()
{
	// Handle flashing

	if (m_ff & kfGobFlashing) {
		m_ff &= ~kfGobFlashing;
		m_ff |= kfGobDrawFlashed | kfGobRedraw;
		m_unvl.MinSkip();
		gevm.SetRedrawFlags(kfRedrawBeforeTimer);
	} else if (m_ff & kfGobDrawFlashed) {
		m_ff &= ~kfGobDrawFlashed;
		m_ff |= kfGobRedraw;
		m_unvl.MinSkip();
	}

	// Handle damage countdown

	if (m_cDamageCountdown != 0 && !(m_ff & kfGobBeingBuilt)) {
		m_cDamageCountdown -= m_unvl.GetUpdateCount();
		if (m_cDamageCountdown <= 0) {
			m_cDamageCountdown = 0;

			// Time to redraw w/ no damage indicator

			ClearDamageIndicator();
		} else {
			m_unvl.MinSkip(m_cDamageCountdown);
		}
	}
}

void UnitGob::PopupMenu()
{
	if (m_puntc->pfrmMenu == NULL)
		return;

	// UNDONE: show a special being-built menu (Abort only)
	if (m_ff & kfGobBeingBuilt)
		return;

	// Show the menu and get the user's selection

	Assert(!(m_puntc->pfrmMenu->GetFlags() & kfFrmDoModal));

	m_puntc->pfrmMenu->SetOwner(this);
	gpmfrmm->AddForm(m_puntc->pfrmMenu);
	int idc;
	m_puntc->pfrmMenu->DoModal(&idc);
	gpmfrmm->RemoveForm(m_puntc->pfrmMenu);

	// Act on the user's selection

	OnMenuItemSelected(idc);
}

bool UnitGob::LoadMenu(UnitConsts *puntc, IniReader *pini, char *pszTemplate, int idfDefault)
{
	int idf;
	if (pini->GetPropertyValue(pszTemplate, "Menu", "%d", &idf) != 1)
		idf = idfDefault;

	puntc->pfrmMenu = new UnitMenu();
	if (puntc->pfrmMenu == NULL)
		return false;

	if (!puntc->pfrmMenu->Init(gpmfrmm, gpiniForms, idf))
		return false;

	// UNDONE: override title label

	gpmfrmm->RemoveForm(puntc->pfrmMenu);
	return true;
}

TCoord UnitGob::CalcRange(TCoord tx, TCoord ty, Gob *pgob)
{
	TPoint tpt;
	pgob->GetTilePosition(&tpt);
	TCoord tcx = abs(tx - tpt.tx);
	TCoord tcy = abs(ty - tpt.ty);

	if (tcx >= 10 || tcy >= 10)
		// Lame rectangular range calculator good enough for stuff that
		// is almost certainly out of interesting range anyway and it
		// allows us to keep the size of the Euclidean distance table down.

		return tcx > tcy ? tcx : tcy;
	else
		// Nice accurate euclidean distance calculator

		return gmpDistFromDxy[tcx][tcy];
}

void UnitGob::RememberEnemyNearby(Gid gidEnemy)
{
	// If this gob is already in our list, nothing to do

	int n;
	for (n = 0; n < ARRAYSIZE(m_agidEnemyNearby); n++) {
		if (m_agidEnemyNearby[n] == gidEnemy)
			return;
	}

	// We remember a limited # of gobs, so replace the first one
	// we find that is further away than the new enemy. Sometimes
	// the gid ids can be recycled into gobs on the same side, so check for that.

	UnitGob *puntEnemy = (UnitGob *)ggobm.GetGob(gidEnemy);
	if (puntEnemy == NULL)
		return;
	TCoord txThis = TcFromWc(m_wx);
	TCoord tyThis = TcFromWc(m_wy);
	Side sideThis = GetSide();

	TCoord tcEnemy = CalcRange(txThis, tyThis, puntEnemy);
	for (n = 0; n < ARRAYSIZE(m_agidEnemyNearby); n++) {
		Gob *puntEnemyNearby = ggobm.GetGob(m_agidEnemyNearby[n]);
		if (puntEnemyNearby == NULL || puntEnemyNearby->GetSide() == sideThis) {
			m_agidEnemyNearby[n] = gidEnemy;
			m_ff &= ~kfGobNoEnemiesNearby;
			return;
		}
		TCoord tc = CalcRange(txThis, tyThis, puntEnemyNearby);
		if (tc > tcEnemy) {
			m_agidEnemyNearby[n] = gidEnemy;
			m_ff &= ~kfGobNoEnemiesNearby;
			return;
		}
	}

    // NOTE: might want to set a "rescan" bit if an enemy doesn't fit in the
    // list, the issue being we don't want that bit set most of the time.
}

UnitGob *UnitGob::FindEnemyNearby(TCoord tcRange)
{
	if (m_ff & kfGobNoEnemiesNearby)
		return NULL;

	TCoord txThis = TcFromWc(m_wx);
	TCoord tyThis = TcFromWc(m_wy);
	Side sideThis = GetSide();

	int cEmpty = 0;
	TCoord tcClosest = tcRange;
	UnitGob *puntEnemyClosest = NULL;

	for (int n = 0; n < ARRAYSIZE(m_agidEnemyNearby); n++) {
		Gid gidEnemyNearby = m_agidEnemyNearby[n];
		if (gidEnemyNearby == kgidNull) {
			cEmpty++;
			continue;
		}
		UnitGob *puntEnemyNearby = (UnitGob *)ggobm.GetGob(gidEnemyNearby);
		if (puntEnemyNearby == NULL || puntEnemyNearby->GetSide() == sideThis) {
			m_agidEnemyNearby[n] = kgidNull;
			cEmpty++;
			continue;
		}
		TCoord tc = CalcRange(txThis, tyThis, puntEnemyNearby);
		if (tc <= tcClosest) {
			tcClosest = tc;
			puntEnemyClosest = puntEnemyNearby;
		}
	}

	if (puntEnemyClosest != NULL)
		return puntEnemyClosest;

	if (cEmpty == ARRAYSIZE(m_agidEnemyNearby)) {
#if defined(DEBUG) && defined(WIN)
//		Assert(ggobm.FindEnemyWithinRange(this, tcRange) == NULL, "Lost track of a nearby enemy!");
#endif
		m_ff |= kfGobNoEnemiesNearby;
	}

	return NULL;
}

// NOTE: Max sight distance assumption

#define ktcSightRangeMax 5

void UnitGob::NotifyEnemyNearby()
{
	// OPT: Could scan leading edges only (when moving), as long as
	// the edge is 2 tiles thick. Would need to break out cases where the whole
	// rect needs to be scanned.

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);

	TCoord txLeft = tx - ktcSightRangeMax;
	if (txLeft < 0)
		txLeft = 0;
	TCoord txRight = tx + ktcSightRangeMax + 1;
	if (txRight > ctx)
		txRight = ctx;
	TCoord tyTop = ty - ktcSightRangeMax;
	if (tyTop < 0)
		tyTop = 0;
	TCoord tyBottom = ty + ktcSightRangeMax + 1;
	if (tyBottom > cty)
		tyBottom = cty;

	// Let's check for 'discovery' while we're at it

	SideMask sidmAlreadyDiscovered = m_pplr->GetDiscoveredSides();

	for (TCoord tyT = tyTop; tyT < tyBottom; tyT++) {
		for (TCoord txT = txLeft; txT < txRight; txT++) {
			// Any enemies here that want to know about mobile units?

			Side side = m_pplr->GetSide();
			bool fPreped = false;
			for (Gid gid = ggobm.GetFirstGid(txT, tyT); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {
				UnitGob *punt = (UnitGob *)ggobm.GetGob(gid);
				if (punt == NULL)
					continue;
				dword ffUnit = punt->GetFlags();
				if (!(ffUnit & kfGobUnit))
					continue;

				// Has side A already discovered side B?

				SideMask sidm = GetSideMask(punt->GetSide());
				if (!(sidmAlreadyDiscovered & sidm)) {

					// No

					sidmAlreadyDiscovered |= sidm;
					m_pplr->SetDiscoveredSides(sidmAlreadyDiscovered);
					TPoint tpt;
					punt->GetTilePosition(&tpt);
					m_pplr->SetDiscoverPoint(&tpt);

					// Then side B has just discovered side A too.

					Player *pplrB = punt->m_pplr;
					pplrB->SetDiscoveredSides(pplrB->GetDiscoveredSides() | GetSideMask(side));
					pplrB->SetDiscoverPoint(&tpt);
				}

				// Any enemies in this tile that want to know about mobile units?

				if (punt->IsAlly(side))
					continue;

				// Notify this enemy

				Gid gidFound = punt->GetId();
				Message msg;
				if (!fPreped) {
					fPreped = true;
					memset(&msg, 0, sizeof(msg));
					msg.mid = kmidEnemyNearby;
					msg.smidSender = m_gid;
				}

				// Notify this enemy of the current gob if it wants to know

				if (gapuntc[punt->GetUnitType()]->wf & kfUntcNotifyEnemyNearby) {
					msg.EnemyNearby.gidEnemy = m_gid;
					msg.smidReceiver = gidFound;
					gsmm.SendMsg(&msg);
				}

				// Notify the current gob of this enemy, only if this enemy is a mobile unit
				// (so that the current gob doesn't get notified of towers).

				if (ffUnit & kfGobMobileUnit) {
					if (m_puntc->wf & kfUntcNotifyEnemyNearby) {
						msg.EnemyNearby.gidEnemy = gidFound;
						msg.smidReceiver = m_gid;
						gsmm.SendMsg(&msg);
					}
				}
			}
		}
	}
}

void UnitGob::RecalcEnemyNearby(bool fClearOnly)
{
	// NOTE: Could set a lazy rescan bit instead of forcing a rescan.
	// Only if perf is an issue.

	if (fClearOnly) {
		// Wipe the enemies list clean and recalc

		m_ff &= ~kfGobNoEnemiesNearby;
		for (int n = 0; n < ARRAYSIZE(m_agidEnemyNearby); n++)
			m_agidEnemyNearby[n] = kgidNull;
		return;
	}

	// Calc enemy nearby

	NotifyEnemyNearby();
}

// TUNE:

const TCoord ktcNotifyAlliesOfHitRange = 4;

void UnitGob::NotifyNearbyAlliesOfHit(Gid gidAssailant)
{
	// Only report getting nailed once every other update

	long cupd = gsim.GetUpdateCount();
	if (cupd - m_cupdLastHitNotify < 2)
		return;
	m_cupdLastHitNotify = cupd;

	TCoord ctx, cty;
	ggobm.GetMapSize(&ctx, &cty);
	TCoord tx = TcFromWc(m_wx);
	TCoord ty = TcFromWc(m_wy);

	TCoord txLeft = tx - ktcNotifyAlliesOfHitRange;
	if (txLeft < 0)
		txLeft = 0;
	TCoord txRight = tx + ktcNotifyAlliesOfHitRange + 1;
	if (txRight > ctx)
		txRight = ctx;
	TCoord tyTop = ty - ktcNotifyAlliesOfHitRange;
	if (tyTop < 0)
		tyTop = 0;
	TCoord tyBottom = ty + ktcNotifyAlliesOfHitRange + 1;
	if (tyBottom > cty)
		tyBottom = cty;

	Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.mid = kmidNearbyAllyHit;
	msg.smidSender = m_gid;
	msg.Hit.gidAssailant = gidAssailant;

	for (TCoord tyT = tyTop; tyT < tyBottom; tyT++) {
		for (TCoord txT = txLeft; txT < txRight; txT++) {
			// Any MobileGob allies around?

			Side side = m_pplr->GetSide();
			bool fPreped = false;
			for (Gid gid = ggobm.GetFirstGid(txT, tyT); gid != kgidNull; gid = ggobm.GetNextGid(gid)) {

				// Don't notify self!

				if (gid == m_gid)
					continue;

				UnitGob *punt = (UnitGob *)ggobm.GetGob(gid);
				if (punt == NULL)
					continue;
				dword ffUnit = punt->GetFlags();
				if (!(ffUnit & kfGobMobileUnit))
					continue;
				if (!punt->IsAlly(side))
					continue;

				// OPT: don't notify MobileUnits that are already attacking a target?

				// Notify this ally

				msg.smidReceiver = punt->GetId();
				gsmm.SendMsg(&msg);
			}
		}
	}
}

void UnitGob::ShowDamageIndicator()
{
	// Only show enemy units' health

	if (m_pplr == gpplrLocal)
		return;

	// Only if turned on

	if (!(gwfPerfOptions & kfPerfEnemyDamageIndicator))
		return;

	// Enable health indicator drawing

	m_wfUnit |= kfUnitDrawHealthIndicator;
	m_ff |= kfGobLayerSelection;
	MarkRedraw();

	m_cDamageCountdown = UpdFromMsec(kcmsDamageIndicatorTimeout);
	m_unvl.MinSkip(m_cDamageCountdown);
}

void UnitGob::ClearDamageIndicator()
{
	if ((m_ff & kfGobSelected) == 0)
		m_ff &= ~kfGobLayerSelection;
	m_wfUnit &= ~kfUnitDrawHealthIndicator;
	MarkRedraw();
}

//===========================================================================
// UnitMenu implementation

UnitGob *UnitMenu::GetOwner() 
{
	return m_punt;
}

bool UnitMenu::DoModal(int *pnResult, Sfx sfxShow, Sfx sfxHide)
{
	gtimm.AddTimer(this, 25);
	bool f = Form::DoModal(pnResult, sfxShow, sfxHide);
	gtimm.RemoveTimer(this);
	return f;
}

void UnitMenu::OnTimer(long tCurrent)
{
	// Initialize on a timer so that options show / hide while the form
	// is visible

	m_punt->InitMenu(this);
}

const int kcxButtonSpace = 0;
const int kcyTitleMargin = 3;
const int kcxTitleMargin = 5;

void UnitMenu::SetOwner(UnitGob *punt, bool fPerUnitInit)
{
	m_punt = punt;

	// Let the owning Gob initialize the menu state (hide/show/disable buttons)

    if (fPerUnitInit) {
        m_punt->InitMenu(this);
    }

	char *pszTitle = m_punt->GetConsts()->szName;

	// Calc the width/height of the 'button bar' (all visible buttons)

	int cButtons = 0;
	int cxButton = 0, cyButton = 0;
	int i = 0;
	for (i = 0; i < m_cctl; i++) {
		Control *pctl = m_apctl[i];
		word id = pctl->GetId();
		if (id < kidcRelocButtonMin || id >= kidcRelocButtonMax)
			continue;

		// While we're at it force each button to draw using the 'side1' colors
		// This is because some of them use side1's dark blues.

		word wf = pctl->GetFlags();
		pctl->SetFlags(wf | kfCtlUseSide1Colors);

		if (!(wf & kfCtlVisible))
			continue;

		if (cButtons == 0) {
			Rect rcT;
			pctl->GetRect(&rcT);
			cxButton = rcT.Width();
			cyButton = rcT.Height();
		}
		cButtons++;
	}
	int cxButtons = ((cxButton + kcxButtonSpace) * cButtons) - kcxButtonSpace;

	// Calc the width/height of the title bar

	int cxTitleText = gapfnt[kifntTitle]->GetTextExtent(pszTitle);
	int cxTitleWhole = cxTitleText + kcxTitleMargin * 2;
	int cyFont = gapfnt[kifntTitle]->GetHeight();
	int cyTitle = cyFont + kcyTitleMargin * 2;

	// Get Gob's UIBounds in screen coordinates which becomes the interior
	// rect around which the title and button bar are formatted.
	// This somewhat elaborate means of producing the screen-coord rcUIBounds
	// is designed to perfectly match the coordinate conversion pipeline
	// used to draw the selection rect. This way the tile/button bar are
	// locked right with the selection rect it regardless of resolution.

	WCoord wxViewOrigin, wyViewOrigin;
	gsim.GetViewPos(&wxViewOrigin, &wyViewOrigin);
	int xViewOrigin = PcFromUwc(wxViewOrigin) & 0xfffe;
	int yViewOrigin = PcFromUwc(wyViewOrigin) & 0xfffe;

	Rect rcUIBounds;
	rcUIBounds.FromWorldRect(&m_punt->GetConsts()->wrcUIBounds);
	WPoint wpt;
	m_punt->GetPosition(&wpt);
    int xPos = PcFromWc(wpt.wx) - xViewOrigin;
    int yPos = PcFromWc(wpt.wy) - yViewOrigin;
	rcUIBounds.Offset(xPos, yPos);

	// Set form size/position
    // Put title on top of hilight sprite, if there is one.

	Rect rcForm;
    AnimSprite *panispr = punt->GetAnimSprite();
    if (panispr != NULL) {
        Rect rcSpriteBounds;
        panispr->GetBounds(&rcSpriteBounds);
        rcSpriteBounds.Offset(xPos, yPos);
        rcForm.top = rcSpriteBounds.top - cyTitle - kcyHealthBar;
    } else {
        rcForm.top = rcUIBounds.top - cyTitle - kcyHealthBar;
    }
	rcForm.bottom = rcUIBounds.bottom + cyButton;
	int cxForm = _max(rcUIBounds.Width(), cxTitleWhole);
	if (cxForm < cxButtons)
		cxForm = cxButtons;
	rcForm.left = rcUIBounds.left + (rcUIBounds.Width() / 2) - (cxForm / 2);
	rcForm.right = rcForm.left + cxForm;
	int cyForm = rcForm.Height();

	// Keep the form on screen

	Size sizPlayfield;
	ggame.GetPlayfieldSize(&sizPlayfield);
	int dx = 0, dy = 0;
	if (rcForm.left < 0)
		dx = -rcForm.left;
	else if (rcForm.right > sizPlayfield.cx)
		dx = sizPlayfield.cx - rcForm.right;
	if (rcForm.top < 0)
		dy = -rcForm.top;
	else if (rcForm.bottom > sizPlayfield.cy)
		dy = sizPlayfield.cy - rcForm.bottom;
	rcForm.Offset(dx, dy);
	SetRect(&rcForm);

	// Set title size/position. dx/dy (amount of form that would have
	// been off-screen) are cleverly used to keep the buttons as close
	// to the partially off-screen unit as possible.

	LabelControl *plblTitle = (LabelControl *)GetControlPtr(kidcTitle);
	Rect rcTitle;
	rcTitle.left = (cxForm - cxTitleText) / 2 - dx;
	if (rcTitle.left < 0)
		rcTitle.left = 0;
	else if (rcTitle.left + cxTitleText > cxForm)
		rcTitle.left += cxForm - (rcTitle.left + cxTitleText);
	rcTitle.top = kcyTitleMargin - dy;
	if (rcTitle.top < kcyTitleMargin)
		rcTitle.top = kcyTitleMargin;
	else if (rcTitle.top - kcyTitleMargin + cyTitle > cyForm - cyButton)
		rcTitle.top += (cyForm - cyButton) - (rcTitle.top - kcyTitleMargin + cyTitle);
	rcTitle.right = rcTitle.left + cxTitleText;
	rcTitle.bottom = rcTitle.top + cyFont;
	plblTitle->SetRect(&rcTitle);

	// Set the titlebar text

	plblTitle->SetText(pszTitle);

	// Set the button positions. dx/dy are used as above for the title.

	Rect rcButton;
	rcButton.left = (cxForm - cxButtons) / 2 - dx;
	if (rcButton.left < 0)
		rcButton.left = 0;
	else if (rcButton.left + cxButtons > cxForm)
		rcButton.left += cxForm - (rcButton.left + cxButtons);
	rcButton.top = cyTitle + kcyHealthBar + rcUIBounds.Height() - dy;
	if (rcButton.top < cyTitle)
		rcButton.top = cyTitle;
	else if (rcButton.top + cyButton > cyForm)
		rcButton.top = cyForm - cyButton;
	rcButton.bottom = rcButton.top + cyButton;

	// Stash away for painting later

	m_rcButtons = rcButton;
	m_rcButtons.right = m_rcButtons.left + cxButtons;
	m_rcButtons.Offset(m_rc.left, m_rc.top);

	// Position the buttons

	for (i = 0; i < m_cctl; i++) {
		Control *pctl = m_apctl[i];
		word id = pctl->GetId();
		if (id < kidcRelocButtonMin || id >= kidcRelocButtonMax)
			continue;

		if (!(pctl->GetFlags() & kfCtlVisible))
			continue;

		rcButton.right = rcButton.left + cxButton;
		pctl->SetRect(&rcButton);
		rcButton.left += cxButton + kcxButtonSpace;
	}

	// HACK: if there is only one button it's a Transform or Deliver button
	// and we don't want to draw a border around it.

	if (cButtons == 1)
		m_rcButtons.SetEmpty();

	// menus are AutoTakedown

	m_wf |= kfFrmAutoTakedown | kfFrmTranslucent;
}

void UnitMenu::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	// Draw title background

	LabelControl *plblTitle = (LabelControl *)GetControlPtr(kidcTitle);
	Rect rc;
	plblTitle->GetRect(&rc);
	rc.Offset(m_rc.left, m_rc.top);
	rc.Inflate(kcxTitleMargin - 1, kcyTitleMargin - 1);	// leave 1 pixel space for black border
	ShadowHelper(pbm, pupd, &rc);

	// Draw border around title

	rc.Inflate(1, 1);
	DrawBorder(pbm, &rc, 1, GetColor(kiclrBlack), pupd);

	// Draw border around buttons

	if (!m_rcButtons.IsEmpty())
		DrawBorder(pbm, &m_rcButtons, 1, GetColor(kiclrBlack), pupd);

	// Draw border around whole thing for testing
//	DrawBorder(pbm, &m_rc, 1, GetColor(kiclrRed), pupd);
}

void UnitMenu::OnPaint(DibBitmap *pbm)
{
	Form::OnPaint(pbm);
}

// Treat everything except a tap on one of the button controls as a 'miss'
// and auto-takedown.

bool UnitMenu::OnHitTest(Event *pevt)
{
	if (!(m_wf & kfFrmVisible))
		return false;

	for (int n = m_cctl - 1; n >= 0; n--) {
		// Is it on this control?

		Control *pctl = m_apctl[n];
		if (pctl->OnHitTest(pevt) < 0)
			continue;

		// Yes, keep it

		return true;
	}

	// we're AutoTakedown and hit not on one of the controls

//	Assert(pevt->eType != penHoldEvent && pevt->eType != penUpEvent);
	if (pevt->eType == penDownEvent)  {

		// Take down and let the event pass through

		EndForm(kidcCancel);
		return false;
	}

	// As long as this form stays up it should swallow any other
	// events to avoid surprising the forms/objects underneath.

	return true;
}

#ifdef MP_DEBUG_SHAREDMEM
void UnitGob::MPValidate()
{
//  Warehouse sets animation state during draw
//	m_ani.MPValidate((Animation *)(((byte *)MPGetGobPtr(m_gid)) + OFFSETOF(UnitGob, m_ani)));
	MPValidateGobMember(UnitGob, m_agidEnemyNearby);
	MPValidateGobMember(UnitGob, m_fxHealth);
	MPValidateGobMember(UnitGob, m_cupdLastHitNotify);
//  gpplrLocal specific
//	MPValidateGobMember(UnitGob, m_cDamageCountdown);
//	MPValidateGobMember(UnitGob, m_wfUnit);
	MPValidateGobMember(UnitGob, m_unvl);
}
#endif

} // namespace wi
