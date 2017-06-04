#include "game/ht.h"
#include "game/chatter.h"
#include "mpshared/netmessage.h"

namespace wi {

extern bool gfSuspendUpdates;
extern bool gfSingleStep;

//
// InputUIForm implementation
//

InputUIForm::InputUIForm(Chatter *chatter)
{
	m_fTimerAdded = false;
    m_pchatter = chatter;
}

InputUIForm::~InputUIForm()
{
	if (m_fTimerAdded)
		gtimm.RemoveTimer(this);
}

bool InputUIForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	if (!Form::Init(pfrmm, pini, idf))
		return false;

	// Set the form rect

	DibBitmap *pbm = m_pfrmm->GetDib();
	Size sizDib;
	pbm->GetSize(&sizDib);
	int cyCommandBar = sizDib.cy;
	ModeInfo mode;
	gpdisp->GetMode(&mode);
	int cyInputForm = cyCommandBar + mode.cyGraffiti;
	Rect rcT;
	rcT.Set(0, 0, sizDib.cx, cyInputForm);
	SetRect(&rcT);

	// Get pointers to and dimensions of all the controls involved in the layout

	Control *pctlMenuButton = GetControlPtr(kidcMenuButton);
	Assert(pctlMenuButton != NULL);
	Rect rcMenuButton;
	pctlMenuButton->GetRect(&rcMenuButton);

	Control *pctlCredits = GetControlPtr(kidcCreditsLabel);
	Assert(pctlCredits != NULL);
	Rect rcCredits;
	pctlCredits->GetRect(&rcCredits);

	Control *pctlPower = GetControlPtr(kidcPower);
	Assert(pctlPower != NULL);
	Rect rcPower;
	pctlPower->GetRect(&rcPower);

	// Calc space between controls

	int xLeftMiniMap = m_rc.right - MiniMapControl::CalcWidth();
	int cxSpace = (xLeftMiniMap - (rcMenuButton.Width() + rcCredits.Width() + rcPower.Width())) / 3;
	if (cxSpace < 0)
		cxSpace = 0;

	// Position controls (ycoord of 1 to not overlap the separating line, except for menu button)

	pctlMenuButton->SetPosition(0, 0);
	pctlCredits->SetPosition(rcMenuButton.right + cxSpace, 1);
	pctlPower->SetPosition(rcMenuButton.right + cxSpace + rcCredits.Width() + cxSpace, 1);
	
	// Handle silkscreen area

	static int s_aidcLayout[] = {
			kidcGraffitiScroll, kidcAppsSilkButton, kidcMenuSilkButton,
			kidcCalcSilkButton, kidcFindSilkButton
	};
	static int s_aircGraffiti[] = {
			kircSilkGraffiti, kircSilkApps, kircSilkMenu,
			kircSilkCalc, kircSilkFind
	};

	if (mode.cyGraffiti == 0) {
		// No silkscreen graffiti area, so hide the controls

		for (int n = 0; n < ARRAYSIZE(s_aidcLayout); n++)
			GetControlPtr(s_aidcLayout[n])->Show(false);
	} else {
		// Note the returned graffiti rects are in native coordinates
		// from the top of the screen, whereas the input form starts on-screen,
		// so the rects need to be adjusted
		
		int yOffset = mode.cy - cyCommandBar;
		for (int n = 0; n < ARRAYSIZE(s_aidcLayout); n++) {
			Rect rcT;
			HostGetSilkRect(s_aircGraffiti[n], &rcT);
			rcT.Offset(0, -yOffset);
			GetControlPtr(s_aidcLayout[n])->SetRect(&rcT);
		}
	}

	gtimm.AddTimer(this, kctMapScrollRate);
	m_fTimerAdded = true;

	return true;
}

void InputUIForm::Update()
{
	// Update the player credits display

	CreditsControl *pctlCredits = (CreditsControl *)GetControlPtr(kidcCreditsLabel);
	pctlCredits->Update(gpplrLocal->GetCredits(), gpplrLocal->GetNeedCreditsCount());

	// Update the power indicator

	PowerControl *pctlPower = (PowerControl *)GetControlPtr(kidcPower);
	pctlPower->Update(gpplrLocal->GetPowerDemand(), gpplrLocal->GetPowerSupply());
}

void InputUIForm::OnTimer(long tCurrent)
{
	if (gpmfrmm->GetFocus() == this) {
		dword dwKeys = HostGetCurrentKeyState(keyBitDpadLeft | keyBitDpadRight | keyBitPageUp | keyBitPageDown | keyBitHard1 | keyBitHard2 | keyBitHard3 | keyBitHard4 | keyBitRockerUp | keyBitRockerDown | keyBitRockerLeft | keyBitRockerRight);
		if (dwKeys != 0) {
			WCoord wxView, wyView;
			gsim.GetViewPos(&wxView, &wyView);
			WCoord wcStep = kwcTile;

			if (dwKeys & (keyBitPageUp | keyBitRockerUp))
				wyView -= wcStep;
			if (dwKeys & (keyBitPageDown | keyBitRockerDown))
				wyView += wcStep;
			if (dwKeys & (keyBitDpadLeft | keyBitRockerLeft | keyBitHard1 | keyBitHard2))
				wxView -= wcStep;
			if (dwKeys & (keyBitDpadRight | keyBitRockerRight | keyBitHard3 | keyBitHard4))
				wxView += wcStep;

			gsim.SetViewPos(wxView, wyView);
		}
	}


    ggame.GetSimUIForm()->GetPenHandler()->CheckScroll();

}

void InputUIForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	// Fill background

	Rect rcT;
	gpmm->GetRect(&rcT);
	int xLeftMiniMap = m_rc.right - rcT.Width();
	rcT.Set(0, 0, xLeftMiniMap, m_rc.bottom);
	FillHelper(pbm, pupd, &rcT, GetColor(kiclrBlack));

	// Draw separating line

	rcT.bottom = rcT.top + 1;

	int iclr = gfMultiplayer ? gaiclrSide[gpplrLocal->GetSide()] : kiclr0CyanSide;
	FillHelper(pbm, pupd, &rcT, GetColor(iclr));
}

bool InputUIForm::EventProc(Event *pevt)
{
	if (pevt->eType == keyDownEvent) {
		switch (pevt->chr) {
		case vchrMenu:
        case vchrBack:
        case 'm':
			OnControlSelected(kidcMenuButton);
			break;

#ifdef DEBUG
		case ' ':
			if (gfSuspendUpdates)
				gfSingleStep = true;
			break;

		case 'z':
			{
				static int s_cMark;
				Trace("");
				Trace("+++ MARK %d +++", s_cMark++);
				Trace("");
			}
			break;

		case 'r':
			{
				// Restart mission

				Event evt;
				memset(&evt, 0, sizeof(evt));
				evt.eType = gameOverEvent;
				evt.dw = knGoRetryLevel;
				gevm.PostEvent(&evt);
			}
			break;
#endif
		}
	} else {
		return Form::EventProc(pevt);
	}

	return false;
}

void InputUIForm::OnControlSelected(word idc)
{
	switch (idc) {
	case kidcMenuButton:
		InGameMenu();
		break;

#if 0
	case kidcMapButton:
		// To be replaced with control at some point
		ggame.GetSimUIForm()->ToggleMiniMap();
		break;
#endif

	case kidcAppsSilkButton:
		{
			Event evt;
			evt.eType = appStopEvent;
			gevm.PostEvent(&evt);
		}
		break;

	case kidcMenuSilkButton:
		OnControlSelected(kidcMenuButton);
		break;

#if 0
	case kidcCalcSilkButton:
		OnControlSelected(kidcMapButton);
		break;
#endif

#if defined(DEV_BUILD)
	case kidcFindSilkButton:
	case kidcPower:
		TestOptions();
		break;
#endif

	case kidcCreditsLabel:
		if (!gfMultiplayer)
			gsim.Pause(true);
		gpplrLocal->ShowObjectives(ksoObjectives);
		if (!gfMultiplayer)
			gsim.Pause(false);
		break;
	}
}

bool InputUIForm::OnControlHeld(word idc)
{
	return true;

#if 0
	// Check for preset buttons as a group

	int iPreset = idc - kidcPreset1Button;
	if (iPreset < 0 || iPreset > 7)
		return false;

	if (m_aagidPreset[iPreset] != NULL) {
		delete m_aagidPreset[iPreset];
		m_aagidPreset[iPreset] = NULL;
	}

	m_awptViewPreset[iPreset].wx = kwxInvalid;

	// Collect all selected Gobs

	int cgob = 0;
	Gid *pgid = (Gid *)gpbScratch;
	for (Gob *pgobT = ggobm.GetFirstGob(); pgobT != NULL; pgobT = ggobm.GetNextGob(pgobT)) {
		if (pgobT->GetFlags() & kfGobSelected) {
			*pgid++ = pgobT->GetId();
			cgob++;
		}
	}

	if (cgob == 0) {
		gsim.GetViewPos(&m_awptViewPreset[iPreset].wx, &m_awptViewPreset[iPreset].wy);

		// UNDONE: Play 'preset set' sound
		Control *pctl = GetControlPtr(kidcPreset1Button + iPreset);
		pctl->SetFlags(pctl->GetFlags() | kfCtlSet);
	} else {
		Gid *agid = new Gid[cgob];
		if (agid != NULL) {
			memcpy(agid, gpbScratch, cgob * sizeof(Gid));
			m_aagidPreset[iPreset] = agid;
			m_acgidPreset[iPreset] = cgob;

			// UNDONE: Play 'preset set' sound
			Control *pctl = GetControlPtr(kidcPreset1Button + iPreset);
			pctl->SetFlags(pctl->GetFlags() | kfCtlSet);
		}
	}

	return false;
#endif
}

void InputUIForm::InGameMenu()
{
	// Invoke modal InGameMenu form

	ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms, kidfInGameMenu, new ShellForm());

#if defined(IPHONE) || defined(__IPHONEOS__) || defined(__ANDROID__)
    pfrm->GetControlPtr(kidcExitGame)->Show(false);
#endif

	if (gfMultiplayer) {
		pfrm->GetControlPtr(kidcSaveGame)->Show(false);
		pfrm->GetControlPtr(kidcLoadGame)->Show(false);
		pfrm->GetControlPtr(kidcRestartMission)->Show(false);
		pfrm->GetControlPtr(kidcHelp)->Show(false);

		// Hack

		ButtonControl *pbtn = (ButtonControl *)pfrm->GetControlPtr(kidcAbortMission);
		pbtn->SetText((gpplrLocal->GetFlags() & kfPlrObserver) ? (char *)"STOP OBSERVING" : (char *)"RESIGN");

		word aidcFormat[] = { kidcObjectives, kidcSaveGame, kidcLoadGame, kidcRestartMission, kidcAbortMission, kidcOptions, kidcChat, kidcExitGame, kidcHelp };
		FormatButtons(pfrm, aidcFormat, ARRAYSIZE(aidcFormat), kidcObjectives, kidcOptions);

        // Hiding this here ensures kidcChat is spaced below the rest of the
        // buttons.
        pfrm->GetControlPtr(kidcOptions)->Show(false);
	} else {
        pfrm->GetControlPtr(kidcChat)->Show(false);
    }

	if (!gfMultiplayer)
		gsim.Pause(true);
	int idc;
	pfrm->DoModal(&idc);
	delete pfrm;

	// While the dialog was up the player might have exited the app

	if (!gevm.IsAppStopping()) {
		// UNDONE: display mode change

		switch (idc) {
		case kidcSaveGame:
			ggame.SaveGame();
			break;

		case kidcLoadGame:
			{
				Stream *pstm = PickLoadGameStream();
				if (pstm == NULL)
					break;

				Event evt;
				memset(&evt, 0, sizeof(evt));
				evt.eType = gameOverEvent;
				evt.dw = knGoLoadSavedGame;
				gpstmSavedGame = pstm;
				gevm.PostEvent(&evt);
			}
			break;

		case kidcAbortMission:
		case kidcRestartMission:
			{
				if (gfMultiplayer) {
					if (!ggame.AskResignGame())
						break;
				}

				if ((gpplrLocal->GetFlags() & kfPlrObserver) == 0) {
                    // HACK: only ksoWinSummary has the fields necessary to
                    // support the fAborting flag
					gpplrLocal->ShowObjectives(
                            gfMultiplayer ? ksoLoseSummary : ksoWinSummary,
                            false, gfMultiplayer ? false : true);
                }

				if (!gfMultiplayer
                        || (gpplrLocal->GetFlags() & kfPlrObserver) != 0
                        || !ggame.AskObserveGame()) {
					Event evt;
					memset(&evt, 0, sizeof(evt));
					evt.eType = gameOverEvent;
					evt.dw = (idc == kidcAbortMission)
                            ? knGoAbortLevel : knGoRetryLevel;
					gevm.PostEvent(&evt);
				}
			}
			break;

		case kidcOptions:
			DoModalGameOptionsForm(true);
			break;

		case kidcHelp:
			Help();
			break;

		case kidcExitGame:
			{
				if (gfMultiplayer) {
					if (!ggame.AskResignGame())
						break;
				}

				Event evt;
				memset(&evt, 0, sizeof(evt));
				evt.eType = gameOverEvent;
				evt.dw = knGoAppStop;
				gevm.PostEvent(&evt);

				ggame.SaveReinitializeGame();
			}
			break;

		case kidcObjectives:
			gpplrLocal->ShowObjectives(ksoObjectives);
			break;

        case kidcChat:
            if (m_pchatter != NULL) {
                m_pchatter->ShowChat();
            }
            break;
		}
	}

	if (!gfMultiplayer)
		gsim.Pause(false);
}

void InputUIForm::TestOptions()
{
	// Invoke modal test options form

	Form *pfrm = gpmfrmm->LoadForm(gpiniForms, kidfTestOptions, new TestOptionsForm());
	if (!gfMultiplayer)
		gsim.Pause(true);
	pfrm->DoModal();
	if (!gfMultiplayer)
		gsim.Pause(false);
	delete pfrm;
}

//
// Graffiti scroll control
//

GraffitiScrollControl::GraffitiScrollControl()
{
	m_nScale = 0;
	m_xDragStart = 0;
	m_yDragStart = 0;
	m_wxViewStart = 0;
	m_wyViewStart = 0;
	m_fFrame = false;
}

bool GraffitiScrollControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	// idc (x y cx cy) nScale

	char sz[32];
	int cArgs = pini->GetPropertyValue(pfind, "%*d (%*d %*d %*d %*d) %d %s", &m_nScale, sz);
	if (cArgs == 1) {
		m_fFrame = false;
		return true;
	}
	if (cArgs == 2 && strcmp(sz, "frame") == 0) {
		m_fFrame = true;
		return true;
	}
	return true;
}

void GraffitiScrollControl::OnPenEvent(Event *pevt)
{
	switch (pevt->eType) {
	case penDownEvent:
		m_xDragStart = pevt->x;
		m_yDragStart = pevt->y;
		gsim.GetViewPos(&m_wxViewStart, &m_wyViewStart);
		break;

	case penMoveEvent:
#if 0
		// BUGBUG: overflowed WcFromPc on Mission 3 ((pevt->y - m_yDragStart) * m_nScale == -1548)
		gsim.SetViewPos(m_wxViewStart - WcFromPc((pevt->x - m_xDragStart) * m_nScale), 
				m_wyViewStart - WcFromPc((pevt->y - m_yDragStart) * m_nScale));
#else
		{
			int pcMac = PcFromWc(kwcMax - 1);
			int dxT = (pevt->x - m_xDragStart) * m_nScale;
			if (dxT < -pcMac)
				dxT = -pcMac;
			if (dxT > pcMac)
				dxT = pcMac;
			int dyT = (pevt->y - m_yDragStart) * m_nScale;
			if (dyT < -pcMac)
				dyT = -pcMac;
			if (dyT > pcMac)
				dyT = pcMac;
			gsim.SetViewPos(m_wxViewStart + WcFromPc(dxT), m_wyViewStart + WcFromPc(dyT));
		}
#endif
		break;
	}
}

void GraffitiScrollControl::OnPaint(DibBitmap *pbm)
{
	if (!m_fFrame)
		return;

	Rect rcForm;
	m_pfrm->GetRect(&rcForm);
	Rect rcT = m_rc;
	rcT.Offset(rcForm.left, rcForm.top);

	Color clr = GetColor(kiclrWhite);
	pbm->Fill(rcT.left + 1, rcT.top, rcT.Width() - 2, 1, clr);
	pbm->Fill(rcT.left, rcT.top + 1, 1, rcT.Height() - 2, clr);
	pbm->Fill(rcT.right - 1, rcT.top + 1, 1, rcT.Height() - 2, clr);
	pbm->Fill(rcT.left + 1, rcT.bottom - 1, rcT.Width() - 2, 1, clr);
}

bool GraffitiScrollControl::IsPainting()
{
	return m_fFrame;
}

//
// Credits control
//

bool CreditsControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	m_nCredits = 0;
	Font *pfnt = gapfnt[kifntHud];
	m_rc.right = m_rc.left + pfnt->GetTextExtent("ABC"); // the credits background image
	m_rc.bottom = m_rc.top + pfnt->GetHeight();
	m_fDrawCreditSymbol = true;
	return true;
}

void CreditsControl::OnPaint(DibBitmap *pbm)
{
	// Draw background

	int cx = m_rc.Width();
	Font *pfnt = gapfnt[kifntHud];
	if (m_fDrawCreditSymbol) 
		pfnt->DrawText(pbm, "ABC", m_rc.left, m_rc.top, cx, -1);
	else 
		pfnt->DrawText(pbm, "LBC", m_rc.left, m_rc.top, cx, -1);

	// Draw credits

	char szT[20];
	sprintf(szT, "%ld", m_nCredits > 99999 ? 99999 : m_nCredits);
	pfnt->DrawText(pbm, szT, m_rc.left + pfnt->GetTextExtent("AB") - pfnt->GetTextExtent(szT), m_rc.top, cx, -1);
}

void CreditsControl::Update(long nCredits, word cCreditNeeders)
{
	if ((nCredits != m_nCredits)  || (cCreditNeeders != m_cCreditNeeders)) {
		m_nCredits = nCredits;
		m_cCreditNeeders = cCreditNeeders;

		if (cCreditNeeders == 0) {
			m_fDrawCreditSymbol = true;
		}
		Invalidate();
	}

	if (cCreditNeeders > 0) {
		long cUpdates = gsim.GetUpdateCount();
		long cupdFlash = cUpdates % kcupdSymbolFlashRate;

		// Low Credits will flash OFF for odd intervals
			
		if (cupdFlash == 0) {
			if (gwfPerfOptions & kfPerfSymbolFlashing) {
				m_fDrawCreditSymbol = !((short)(cUpdates / kcupdSymbolFlashRate) & 1);
				Invalidate();
			} else {
				if (!m_fDrawCreditSymbol)
					Invalidate();
				m_fDrawCreditSymbol = true;
			}
		}
	}
}

//
// Power control
//

bool PowerControl::Init(Form *pfrm, IniReader *pini, FindProp *pfind)
{
	// Base initialization

	if (!Control::Init(pfrm, pini, pfind))
		return false;

	m_fShowPowerSymbol = true;
	m_nDemand = m_nSupply = 0;
	Font *pfnt = gapfnt[kifntHud];
	m_rc.right = m_rc.left + pfnt->GetTextExtent("DEF"); // the power background image
	m_rc.bottom = m_rc.top + pfnt->GetHeight();

	// Figure out the maximum amount of power required by an individual structure type
	// so we can draw the 'yellow zone' at OnPaint time.

	m_nDemandMax = 0;
	UnitMask um = 1;
	for (int i = 0; i < 32; i++, um <<= 1) {
		if (um & kumStructures) {
			StructConsts *pstruc = (StructConsts *)gapuntc[i];
			if (pstruc->nPowerDemand > m_nDemandMax)
				m_nDemandMax = pstruc->nPowerDemand;
		}
	}

	return true;
}

// meter is crafted to divide evenly by 10

const int kcPowerMeterPips = 10;

void PowerControl::OnPaint(DibBitmap *pbm)
{
	// Draw background

	int cx = m_rc.Width();
	Font *pfnt = gapfnt[kifntHud];
	pfnt->DrawText(pbm, "DEF", m_rc.left, m_rc.top, cx, -1);

	// Draw power symbol

	if (m_fShowPowerSymbol)
		pfnt->DrawText(pbm, "G", m_rc.left, m_rc.top, cx, -1);

	// Draw power meter

	int cxLeftSide = pfnt->GetTextExtent("D");
	int x = m_rc.left + cxLeftSide;
	int cxMeter = pfnt->GetTextExtent("E");
	int cxPip = pfnt->GetTextExtent("I");
    const char *pszTick = "I";

	if (m_nSupply < m_nDemand) {
        cxPip = pfnt->GetTextExtent("K");
        pszTick = "K";
	} else {
		// Dip into the yellow if building a structure might cause demand 
		// to become less than supply.
		if (m_nSupply - m_nDemand <= m_nDemandMax) {
			cxPip = pfnt->GetTextExtent("J");
            pszTick = "J";
        }
	}

	// UNDONE: handle > 400 units of power
	int nSupplyPerPip = ((StructConsts *)gapuntc[kutReactor])->nPowerSupply;
	int cSupplyPips = m_nSupply / nSupplyPerPip;
	if (cSupplyPips > kcPowerMeterPips)
		cSupplyPips = kcPowerMeterPips;

	for (int i = 0; i < cSupplyPips; i++, x += cxPip) {
        char szTick[2];
        strncpyz(szTick, pszTick, 2);

        // HACK: Power ticks need to overlap by two pixels
        pfnt->DrawText(pbm, szTick, x - i*2, m_rc.top, 1);
    }

	// Draw demand indicator

	int cxDemand = pfnt->GetTextExtent("H");
	int xDemand = (m_nDemand * cxMeter) / (nSupplyPerPip * kcPowerMeterPips);
	pfnt->DrawText(pbm, "H", m_rc.left + cxLeftSide + xDemand - (cxDemand / 2), m_rc.top, cxDemand, -1);
}

void PowerControl::Update(int nDemand, int nSupply)
{
	if (nDemand != m_nDemand || nSupply != m_nSupply) {
		m_nDemand = nDemand;
		m_nSupply = nSupply;
		m_fPowerLow = (m_nDemand > m_nSupply);
		if (!m_fPowerLow)
			m_fShowPowerSymbol = true;
		Invalidate();
	}

	if (m_fPowerLow) {
		long cUpdates = gsim.GetUpdateCount();
		long cupdFlash = cUpdates % kcupdSymbolFlashRate;

			// Low Power will flash ON for odd intervals
			
		if (cupdFlash == 0) {
			if (gwfPerfOptions & kfPerfSymbolFlashing) {
				m_fShowPowerSymbol = ((short)(cUpdates / kcupdSymbolFlashRate) & 1);
				Invalidate();
			} else {
				if (!m_fShowPowerSymbol)
					Invalidate();
				m_fShowPowerSymbol = true;
			}
		}
	}
}

} // namespace wi
