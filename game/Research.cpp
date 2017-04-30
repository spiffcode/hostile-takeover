#include "ht.h"
#include "wistrings.h"

namespace wi {

const int kcUpdateCost = 3; //TUNE: - credits of upgrade per update

class UpgradeForm : public Form
{
public:
	ResearchGob *GetOwner() {
		return m_pgobOwner;
	}
	void SetOwner(ResearchGob *pgobOwner) secStructures;
	void UpdateUpgradeInfo(ListItem *pli) secStructures;

	// Form overrides

	void EndForm(int nResult = kidcCancel) secStructures;
	virtual void OnControlSelected(word idc) secStructures;
	virtual void OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd) secStructures;

private:
	ResearchGob *m_pgobOwner;
};

// UNDONE: migrate from wfUpgrade (kfUpgrade*)
// UNDONE: v2 upgrade infrastructure
// UNDONE: implement increase mining speed upgrade
// UNDONE: new action for setting upgrades and allowed upgrades
// UNDONE: initialize this from upgrades.ini (mirror gobtemplates.ini structure)
// UNDONE: tapping '?' icon hangs

// Single player upgrades

Upgrade gaupg[kupgtMax] = {
	{
		kupgmAdvancedHRC,
		"Advanced HRC", 
		"hrc1", 
		0, kumHumanResourceCenter,
		750, 4500, 
		"Upgrade the HRC to gain access to more advanced infantry.", 
		"This upgrade requires a HRC." 
	},
	{ 
		kupgmAdvancedVTS,
		"Advanced VTS", 
		"vts1", 
		0, kumVehicleTransportStation,
		1150, 4500, 
		"Upgrade the VTS to gain access to more advanced vehicles.", 
		"This upgrade requires a VTS." 
	},
};

// Multiplayer upgrades

Upgrade gaupgMP[kupgtMax] = {
	{
		kupgmAdvancedHRC,
		"Advanced HRC", 
		"hrc1", 
		0, kumHumanResourceCenter,
		1500, 4500, 
		"Upgrade the HRC to gain access to more advanced infantry.", 
		"This upgrade requires a HRC." 
	},
	{ 
		kupgmAdvancedVTS,
		"Advanced VTS", 
		"vts1", 
		0, kumVehicleTransportStation,
		3000, 4500, 
		"Upgrade the VTS to gain access to more advanced vehicles.", 
		"This upgrade requires a VTS." 
	},
#if 0
	{ 
		kupgmIncreasedBullpupSpeed,
		"Increased Bullpup Speed", 
		"bullpup1",
		0, kumProcessor,
		3000, 4500,
		"Increase Bullpup movement and mining speed by 10%. All existing and future Bullpups will "
		"be retrofited with high efficiency triple-buffered fuel cores.",
		"This upgrade requires a Processor."
	}
#endif
};

//
// ResearchGob implementation
//

static StructConsts gConsts;
static UpgradeForm *s_pfrmUpgrade = NULL;
static AnimationData *s_panidUpgradeIcons = NULL;

//
// Gob methods
//

bool ResearchGob::InitClass(IniReader *pini)
{
	gConsts.gt = kgtResearchCenter;
	gConsts.ut = kutResearchCenter;
	//TUNE:
	gConsts.umPrerequisites = kumReactor;

	// Sound effects

	gConsts.sfxAbortRepair = ksfxResearchCenterAbortRepair;
	gConsts.sfxRepair = ksfxResearchCenterRepair;
	gConsts.sfxDamaged = ksfxResearchCenterDamaged;
	gConsts.sfxSelect = ksfxResearchCenterSelect;
	gConsts.sfxDestroyed = ksfxResearchCenterDestroyed;
	gConsts.sfxImpact = ksfxNothing;
	gConsts.sfxAbortUpgrade = ksfxResearchCenterAbortRepair;
	gConsts.sfxUpgrade = ksfxResearchCenterRepair;

	// Wants power notification

	gConsts.wf |= kfUntcNotifyPowerLowHigh;

	// Preload the Upgrade form

	s_pfrmUpgrade = new UpgradeForm();
	if (s_pfrmUpgrade == NULL) {
		Assert("fatal error");
		return false;
	}

	if (!s_pfrmUpgrade->Init(gpmfrmm, gpiniForms, kidfUpgrade)) {
		Assert("unable to init Upgrade form");
		return false;
	}
	gpmfrmm->RemoveForm(s_pfrmUpgrade);

	// Initialize animation that contains the upgrade icons

	s_panidUpgradeIcons = LoadAnimationData("upgrades.anir");
	Assert(s_panidUpgradeIcons != NULL, "Upgrade icons failed to load");
	if (s_panidUpgradeIcons == NULL)
		return false;

	return StructGob::InitClass(&gConsts, pini);
}

void ResearchGob::ExitClass()
{
	delete s_panidUpgradeIcons;
	s_panidUpgradeIcons = NULL;
	delete s_pfrmUpgrade;
	s_pfrmUpgrade = NULL;
	StructGob::ExitClass(&gConsts);
}

ResearchGob::ResearchGob() : StructGob(&gConsts)
{
	m_utTarget = kutNone;
	m_nCreditsSpentOnUpgrade = 0;
}

#define knVerResearchGobState 3
bool ResearchGob::LoadState(Stream *pstm)
{
	byte nVer = pstm->ReadByte();
	if (nVer != knVerResearchGobState)
		return false;
	m_utTarget = (UnitType)(char)pstm->ReadByte();
	
	//temp fix me - 255 check is because CW compiler isn't sign extending
	// m_utTarget in the up-cast from char to UnitType (which is short).
	// Even forcing (UnitType)(short)(signed char) results in 255!

	if (m_utTarget == 255)
		m_utTarget = kutNone;

	if (m_utTarget != kutNone) {
		m_nCreditsSpentOnUpgrade = (int)pstm->ReadWord();
	}
	return StructGob::LoadState(pstm);
}

bool ResearchGob::SaveState(Stream *pstm)
{
	pstm->WriteByte(knVerResearchGobState);
	pstm->WriteByte((byte)m_utTarget);
	if (m_utTarget != kutNone) {
		pstm->WriteWord(m_nCreditsSpentOnUpgrade);
	}
	return StructGob::SaveState(pstm);
}

void ResearchGob::InitMenu(Form *pfrm)
{
	bool fUpgradeInProgress = (m_pplr->GetUpgrades() & (kfUpgradeHrcInProgress | kfUpgradeVtsInProgress)) > 0;

	Control *pctl = pfrm->GetControlPtr(kidcResearch);
	pctl->Show(!fUpgradeInProgress);
	
	pctl = pfrm->GetControlPtr(kidcAbortUpgrade);
	pctl->Show(fUpgradeInProgress);

	StructGob::InitMenu(pfrm);
}

void ResearchGob::OnMenuItemSelected(int idc)
{
	switch (idc) {
	case kidcResearch:
		{
			s_pfrmUpgrade->SetOwner(this);
			gpmfrmm->AddForm(s_pfrmUpgrade);
			int idc;
			s_pfrmUpgrade->DoModal(&idc);
			gpmfrmm->RemoveForm(s_pfrmUpgrade);

			if (idc == kidcCancel)
				break;

			// this must be enqueued because it begins the consumption of credits

			Message msg;
			memset(&msg, 0, sizeof(msg));
			msg.mid = kmidUpgradeCommand;
			msg.smidReceiver = m_gid;
			msg.smidSender = ksmidNull;
			msg.UpgradeCommand.wfUpgrade = idc == kupgmAdvancedHRC ? kfUpgradeHrc : kfUpgradeVts;
			gcmdq.Enqueue(&msg);
			if (m_pplr == gpplrLocal)
				gsndm.PlaySfx(gConsts.sfxUpgrade);
		}
		break;

	case kidcAbortUpgrade:

		// terminate the consumption of credits and restore
		// we can only do one upgrade at a time so don't need to say which

		gcmdq.Enqueue(kmidAbortUpgradeCommand, m_gid);
		if (m_pplr == gpplrLocal)
			gsndm.PlaySfx(gConsts.sfxAbortUpgrade);
		break;

	default:
		StructGob::OnMenuItemSelected(idc);
		break;
	}
}

//
// StateMachine methods
//

#if defined(DEBUG_HELPERS)
char *ResearchGob::GetName()
{
	return "ResearchCenter";
}
#endif

int ResearchGob::ProcessStateMachineMessage(State st, Message *pmsg)
{
BeginStateMachine
	//-----------------------------------------------------------------------

	State(kstIdle)
		OnMsg(kmidUpgradeCommand)

			// the upgrade basically applies to the player - they now build/have
			// fancier versions of the same old structures in question.
			// it's up to the structures themselves to check for the upgrade
			// or its progress and behave appropriately

			if (pmsg->UpgradeCommand.wfUpgrade & kfUpgradeVts) {
				if (!(m_pplr->GetUpgrades() & (kfUpgradeVts | kfUpgradeVtsInProgress))) {
					m_pplr->SetUpgrades(m_pplr->GetUpgrades() | kfUpgradeVtsInProgress);
					Assert(m_utTarget == kutNone);
					m_utTarget = kutVehicleTransportStation;

				}
			}

			if (pmsg->UpgradeCommand.wfUpgrade & kfUpgradeHrc) {
				if (!(m_pplr->GetUpgrades() & (kfUpgradeHrc | kfUpgradeHrcInProgress))) {
					m_pplr->SetUpgrades(m_pplr->GetUpgrades() | kfUpgradeHrcInProgress);
					Assert(m_utTarget == kutNone);
					m_utTarget = kutHumanResourceCenter;
				}
			}

			// Cause the current state to wake up and start costing the upgrade

			m_unvl.MinSkip();

		OnMsg(kmidAbortUpgradeCommand)
			AbortUpgrade();

		OnMsg(kmidSelfDestructCommand)
			AbortUpgrade();
			SelfDestruct();

		OnUpdate
			// Don't animate if power is too low
			// This relies on kmidPowerLowHigh to wake up if power situation changes

			if (!m_pplr->IsPowerLow())
				AdvanceAnimation(&m_ani);

			if (m_utTarget != kutNone) {
				UpgradeUpdate();
				MarkRedraw();
				m_unvl.MinSkip();
			}
			DefUpdate();

#if 0
EndStateMachineInherit(StructGob)
#else
            return knHandled;
        }
    } else {
        return (int)StructGob::ProcessStateMachineMessage(st, pmsg); 
    }
    return (int)StructGob::ProcessStateMachineMessage(st, pmsg);
#endif
}

void ResearchGob::UpgradeUpdate()
{			
	int nCost = ((StructConsts *)gapuntc[m_utTarget])->nUpgradeCost;

	// God can upgrade things instantly and for free

	if (gfGodMode && !ggame.IsMultiplayer()) {
		m_nCreditsSpentOnUpgrade = nCost;
		WaitingForCredits(false);
	} else {

		// calc update as either a default amount or what's left

		int nCostPerUpdate = nCost - m_nCreditsSpentOnUpgrade;
		if (nCostPerUpdate > kcUpdateCost)
 			nCostPerUpdate = kcUpdateCost;	//TUNE:

		// update if the player has enough credits to cover it

 		int nCredits = m_pplr->GetCredits() - nCostPerUpdate;
 		if (nCredits >= 0) {
 			m_pplr->SetCredits(nCredits, true);
			m_nCreditsSpentOnUpgrade += nCostPerUpdate;
			Assert(m_nCreditsSpentOnUpgrade <= nCost);
		}
		WaitingForCredits(nCredits < 0);
	}
	if (m_nCreditsSpentOnUpgrade == nCost) {

		// fix up the player flags
		// play the appropriate noises
		// reset our state

		word wFlagOn, wFlagOff;
		if (m_utTarget == kutHumanResourceCenter) {
			wFlagOn = kfUpgradeHrc;
			wFlagOff = kfUpgradeHrcInProgress;
		} else {
			wFlagOn = kfUpgradeVts;
			wFlagOff = kfUpgradeVtsInProgress;
		}
		word wUpgrades = m_pplr->GetUpgrades();
		wUpgrades &= ~wFlagOff;
		m_pplr->SetUpgrades(wUpgrades | wFlagOn);

		if (m_pplr == gpplrLocal) {
			gsndm.PlaySfx(m_utTarget == kutHumanResourceCenter ? ksfxGameNewRecruitOptions : ksfxGameNewVehicleOptions);
			ShowAlert(m_utTarget == kutHumanResourceCenter ? kidsNewRecruitOptions : kidsNewVehicleOptions);
		}
		
		m_utTarget = kutNone;
		m_nCreditsSpentOnUpgrade = 0;
		WaitingForCredits(false);
	}
}

void ResearchGob::Draw(DibBitmap *pbm, int xViewOrigin, int yViewOrigin, int nLayer)
{
	StructGob::Draw(pbm, xViewOrigin, yViewOrigin, nLayer);

	if (nLayer == knLayerDepthSorted) {
		if (m_utTarget != kutNone) {
			Rect rcT;
			rcT.FromWorldRect(&m_puntc->wrcUIBounds);
			rcT.Offset(-xViewOrigin + PcFromWc(m_wx), -yViewOrigin + PcFromWc(m_wy));
			DrawBuildProgressIndicator(pbm, &rcT, m_nCreditsSpentOnUpgrade, ((StructConsts *)gapuntc[m_utTarget])->nUpgradeCost); // passing into fix params?
		}
	}
}

void ResearchGob::AbortUpgrade()
{
	// abandon upgrade and restore spent credits
	
	if (m_utTarget != kutNone) {
		m_pplr->SetCredits(m_pplr->GetCredits() + m_nCreditsSpentOnUpgrade, false);
		m_pplr->ModifyTotalCreditsConsumed(-m_nCreditsSpentOnUpgrade);
		m_pplr->SetUpgrades(m_pplr->GetUpgrades() & ~(kfUpgradeVtsInProgress | kfUpgradeHrcInProgress));
		m_utTarget = kutNone;
		m_nCreditsSpentOnUpgrade = 0;
		WaitingForCredits(false);
		m_unvl.MinSkip();
	}
}

void ResearchGob::Deactivate()
{
	// for both takeover and destruction the upgrade should be abandoned w/o restoring credits

	if (m_utTarget != kutNone)
	{
		m_pplr->SetUpgrades(m_pplr->GetUpgrades() & ~(kfUpgradeVtsInProgress | kfUpgradeHrcInProgress));
		m_utTarget = kutNone;
		m_nCreditsSpentOnUpgrade = 0;
	}

	StructGob::Deactivate(); 
}

//===========================================================================
// UpgradeForm implementation

void UpgradeForm::SetOwner(ResearchGob *pgobOwner)
{
	Size sizDib;
	m_pfrmm->GetDib()->GetSize(&sizDib);
	Rect rc;
	rc.left = ((sizDib.cx - m_rc.Width()) / 2) & ~1;
	rc.top = 0; // (sizDib.cy - m_rc.Height()) / 2;
	rc.right = rc.left + m_rc.Width();
	rc.bottom = rc.top + m_rc.Height();
	SetRect(&rc);

	m_wf |= kfFrmAutoTakedown;

	m_pgobOwner = pgobOwner;

	BuildListControl *plstc = (BuildListControl *)GetControlPtr(kidcList);
	plstc->Clear();

	// Initialize list with available upgrade types.
	// UNDONE: need to link list to dynamic build capability

	Player *pplr = pgobOwner->GetOwner();
	UpgradeMask upgmOwned = pplr->GetUpgradeMask();
	UpgradeMask upgmAllowed = pplr->GetAllowedUpgrades();

	int cUpgrades = 0;
	Upgrade *pupg = gfMultiplayer ? gaupgMP : gaupg;
	for (int i = 0; i < kupgtMax; i++, pupg++) {

		// Has the player already acquired this upgrade? If so, don't add it to the list

		if (pplr->GetUpgradeMask() & pupg->upgm)
			continue;

		// Does this player have what it takes to perform this upgrade?

		bool fDisabled = true;
		if (gfGodMode || ((upgmOwned & pupg->upgmPrerequisites) == pupg->upgmPrerequisites &&
				(pplr->GetUnitMask() & pupg->umPrerequisites) == pupg->umPrerequisites))
			fDisabled = false;

		// Is this one of the upgrades this player is allowed?

		if (gfGodMode || (pupg->upgm & upgmAllowed)) {

			// Yep, add it to the list

			cUpgrades++;
			int nStripIcon = s_panidUpgradeIcons->GetStripIndex(pupg->szIconName);
			if (nStripIcon != -1) {
				plstc->Add(s_panidUpgradeIcons, nStripIcon, 0, (void *)pupg, fDisabled);
			}
		}
	}

	if (cUpgrades == 0) {
		// Hide the "Research" button if no upgrades are available

		ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
		pbtn->Show(false);
		LabelControl *plbl = (LabelControl *)GetControlPtr(kidcCost);
		plbl->SetText("");
		PipMeterControl *pmtr = (PipMeterControl *)GetControlPtr(kidcCostMeter);
		pmtr->SetValue(0);
		plbl = (LabelControl *)GetControlPtr(kidcName);
		plbl->SetText("NO UPGRADES AVAILABLE");
		plbl = (LabelControl *)GetControlPtr(kidcDescription);
		plbl->SetText("");
		plbl = (LabelControl *)GetControlPtr(kidcPrerequisites);
		plbl->SetText("");
		plbl = (LabelControl *)GetControlPtr(kidcCostLabel);
		plbl->Show(false);
		plbl = (LabelControl *)GetControlPtr(kidcPrerequisitesLabel);
		plbl->Show(false);
	}
	
	// Select the first upgrade by default

	plstc->Select(0);
	plstc->SetQueueInfo(NULL, NULL);

}

void UpgradeForm::UpdateUpgradeInfo(ListItem *pli)
{
	Upgrade *pupg = (Upgrade *)pli->pvData;

	// Update Cost

	LabelControl *plbl = (LabelControl *)GetControlPtr(kidcCost);
	char szT[20];
	itoa(pupg->nCost, szT, 10);
	plbl->SetText(szT);
	PipMeterControl *pmtr = (PipMeterControl *)GetControlPtr(kidcCostMeter);
	pmtr->SetValue((pupg->nCost * 100) / 3000);
	plbl = (LabelControl *)GetControlPtr(kidcCostLabel);
	plbl->Show(true);

	// Update Name

	plbl = (LabelControl *)GetControlPtr(kidcName);
	plbl->SetText(pupg->szName);

	// Update Description

	plbl = (LabelControl *)GetControlPtr(kidcDescription);
	plbl->SetText(pupg->szDescription);

	// Update prerequisites

	plbl = (LabelControl *)GetControlPtr(kidcPrerequisites);
	plbl->SetText(pupg->szPrerequisites);
	plbl = (LabelControl *)GetControlPtr(kidcPrerequisitesLabel);
	plbl->Show(true);

	// Hide the "Research" button if this upgrade is disabled

	ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
	pbtn->Show(!pli->fDisabled);
}

void UpgradeForm::EndForm(int nResult)
{
	m_pgobOwner = NULL;
	Form::EndForm(nResult);
}

void UpgradeForm::OnControlSelected(word idc)
{
	switch (idc) {
	case kidcCancel:
		EndForm(kidcCancel);
		return;

	case kidcHelp:
		// UNDONE: upgrade help text
		Help("upgrades", !ggame.IsMultiplayer());
		break;

	case kidcList:
		{
			ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
			ListItem *pli = plstc->GetSelectedItem();
			if (pli != NULL)
				UpdateUpgradeInfo(pli);
		}
		break;

	case kidcOk:
		ListControl *plstc = (ListControl *)GetControlPtr(kidcList);
		EndForm(((Upgrade *)plstc->GetSelectedItemData())->upgm);
		return;
	}
}

void UpgradeForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	TBitmap *ptbm = CreateTBitmap("buildformbkgd.png");
	BltHelper(pbm, ptbm, pupd, m_rc.left, m_rc.top);
	delete ptbm;
}

} // namespace wi
