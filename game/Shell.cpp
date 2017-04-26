#include "game/ht.h"
#include "game/lobby.h"
#include "game/serviceurls.h"
#include "base/misc.h"

namespace wi {

// A simple form handler for the main menu, for buttons that want to
// be processed without ending the modal form loop.

class MainMenuForm : public ShellForm
{
public:
    virtual void OnControlSelected(word idc) {
        // Catch this here so the form doesn't get re-created (with associated
        // sound effect).
        if (idc == kidcLeaderboard) {
            LoginHandler handler;
            std::string d = base::StringEncoder::QueryEncode(gszDeviceId);
            std::string o(base::StringEncoder::QueryEncode(HostGetPlatformString()));
            const char *url; 
            if (strlen(handler.StatsUsername()) == 0) {
                url = base::Format::ToString("%s?d=%s&o=%s", kszLeaderboardUrl,
                        d.c_str(), o.c_str());
            } else {
                std::string q = base::StringEncoder::QueryEncode(
                        handler.StatsUsername());
                url = base::Format::ToString("%s?p=%s&d=%s&o=%s", kszLeaderboardUrl,
                        q.c_str(), d.c_str(), o.c_str());
            }
            HostInitiateWebView("Hostile Takeover Statistics", url);
            return;
        }
        ShellForm::OnControlSelected(idc);
    }
};

Shell gshl;
Shell::Shell()
{
}

bool Shell::Init()
{
	return true;
}

void Shell::Exit()
{
}

int Shell::PlayGame(PlayMode pm, MissionIdentifier *pmiid, Stream *pstm,
        int nRank)
{
	// UNDONE: free up space-taking Shell resources

	int nGo;

	do {
		nGo = knGoSuccess;

		switch (pm) {
		case kpmNormal:
			nGo = ggame.PlayLevel(pmiid, NULL, nRank);
			break;

		case kpmSavedGame:
			nGo = ggame.PlaySavedGame(pstm);
			break;
		}

		if (nGo == knGoLoadSavedGame) {
			pm = kpmSavedGame;
			pstm = gpstmSavedGame;
 		}
	} while (nGo == knGoLoadSavedGame);

	// UNDONE: reload space-taking Shell resources

	return nGo;
}

void Shell::Launch(bool fLoadReinitializeSave, MissionIdentifier *pmiid)
{
#ifdef STRESS
	static char *s_aszStressLevels[] = {
		"S_01.lvl", "S_02.lvl", "S_03.lvl", "S_04.lvl", "S_05.lvl", "S_06.lvl", "S_07.lvl", 
		"S_08.lvl", "S_09.lvl", "S_10.lvl", "S_11.lvl", "S_12.lvl", "S_13.lvl", "S_14.lvl"
	};

	if (gfStress) {
		while (true) {
			gtStressTimeout = 30 * 60 * 100L;		// 30 minutes
			int ilvl = GetAsyncRandom() % ARRAYSIZE(s_aszStressLevels);
            strncpyz(pmiid->szLvlFilename, s_aszStressLevels[ilvl],
                    sizeof(pmiid->szLvlFilename));
			int nGo = PlayGame(kpmNormal, pmiid, NULL, 0);
			if (nGo == knGoAppStop)
				return;
		}
	}
#endif // def STRESS

#if 0
#if defined(DEBUG) && defined(CE)
	WCHAR wszDeviceName[128];
	SystemParametersInfo(SPI_GETOEMINFO, sizeof(wszDeviceName), &wszDeviceName, 0);
   char tst[500];
   int cch = wcslen(wszDeviceName);
   char *pch = (char *)wszDeviceName;
   char *pdst = tst;
   while (cch > 0) {
	   if (*pch != 0)
		   *pdst++ = *pch++;
		else
			pch++;
	   cch--;
   }
   *pdst = 0;
   HtMessageBox(kfMbClearDib, "Device ID String", tst); 
#endif
#endif

#ifdef BETA_TIMEOUT
	HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "HOSTILE TAKEOVER",
			"Hostile Takeover\n\n"
			"Copyright 2003-2008 Spiffcode, Inc.\n\n"
			"http://www.spiffcode.com\n\n"
			"Test Release. Please keep private until out of beta. Thank you :)");
#endif

	// Handle DRM

	if (!DrmValidate())
		return;

	if (gevm.IsAppStopping())
		return;

	// Are we launching immediately into a saved game?

	Stream *pstm = HostOpenSaveGameStream(knGameReinitializeSave, true);

	if (pstm != NULL) {

		// Stream is closed/deleted upon return
		
		int nGo = PlayGame(kpmSavedGame, NULL, pstm, 0);
		if (nGo == knGoAppStop)
			return;

		//if (nGo == knGoInitFailure)
		// silently fall into a normal game open

	} else if (pmiid != NULL) {
		int nGo = PlayGame(kpmNormal, pmiid, NULL, 0);
		if (nGo == knGoAppStop)
			return;

		if (nGo == knGoInitFailure)
			HtMessageBox(kfMbWhiteBorder, "Load Game", "Error launch level!");
	}

	while (true) {
        if (gevm.IsAppStopping()) {
            return;
        }

        // Show startup form (new single player, new multi player, load saved
        // game, etc)

		ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms,
                kidfStartup, new MainMenuForm());
		Assert(pfrm != NULL);
		if (pfrm == NULL)
			return;

#ifdef V100_MENUS
		pfrm->GetControlPtr(kidcPlayMultiPlayer)->Show(false);
		pfrm->GetControlPtr(kidcPlaySinglePlayer)->Show(false);
		if (gfDemo) {
			pfrm->GetControlPtr(kidcBeginNewGame)->Show(false);   
			pfrm->GetControlPtr(kidcPlayMission)->Show(false);   
		} else {   
			pfrm->GetControlPtr(kidcPlayDemo)->Show(false);   
			Control *pctl = pfrm->GetControlPtr(kidcBuyMe);   
			Rect rc;   
			pctl->GetRect(&rc);   
			pctl->Show(false);   
			pctl = pfrm->GetControlPtr(kidcLoadSavedGame);   
			pctl->SetRect(&rc);   
		} 
#else
		if (gfDemo)
			pfrm->GetControlPtr(kidcLoadSavedGame)->Show(false);
		else
			pfrm->GetControlPtr(kidcBuyMe)->Show(false);
#endif

		int idc;
		pfrm->DoModal(&idc);
		delete pfrm;

		// While the dialog was up the player might have exited the app

		if (gevm.IsAppStopping())
			return;

        // Before beta check

        switch (idc) {
        case kidcPlay:
            if (DoPlay()) {
                return;
            }
            continue;

		case kidcBuyMe:
			DrmValidate();
            continue;

		case kidcSetupGame:
			DoModalGameOptionsForm(false);
            continue;
	
        case kidcForums:
            HostOpenUrl(kszForumUrl);
            continue;

		case kidcHelp:
			Help();
			continue;

		case kidcCredits:
			Help("credits");
			continue;
        }

#ifdef BETA_TIMEOUT
        if (!CheckBetaTimeout()) {
            continue;
        }
#endif

		switch (idc) {
		case kidcPlayDemo:
		case kidcBeginNewGame:
			if (BeginNewGame())
				return;
			break;

		case kidcPlayMission:
		case kidcPlaySinglePlayer:
			if (PlaySinglePlayer(NULL)) {
				return;
            }
			break;

		case kidcPlayMultiPlayer:
            if (PlayMultiplayer(NULL)) {
                return;
            }
			break;

		case kidcLoadSavedGame:
			{
				Stream *pstm = PickLoadGameStream();
				if (pstm == NULL)
					break;

				// Stream is closed/deleted upon return

				int nGo = PlayGame(kpmSavedGame, NULL, pstm, 0);

				if (nGo == knGoAppStop)
					return;

				if (nGo == knGoInitFailure)
					HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "Load Game", "Error loading saved game!");
			}
			break;

        case kidcDownloadMissions:
            DownloadMissionPack();
            break;

		case kidcExitGame:
			return;
		}
	}
}

bool Shell::DoPlay()
{
    while (true) {
        ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms,
                kidfPlay, new ShellForm());
        if (pfrm == NULL) {
            return kidcCancel;
        }
        int idc;
        pfrm->DoModal(&idc);
        delete pfrm;

        if (gevm.IsAppStopping()) {
            return true;
        }

        if (idc == kidcCancel) {
            return false;
        }

#ifdef BETA_TIMEOUT
        if (!CheckBetaTimeout()) {
            continue;
        }
#endif

        if (idc == kidcPlaySinglePlayer) {
			if (PlaySinglePlayer(NULL)) {
                return true;
            }
        }

        if (idc == kidcPlayMultiPlayer) {
            if (PlayMultiplayer(NULL)) {
                return true;
            }
        }
    }
}

void Shell::DownloadMissionPack()
{
    // Show the download form. True is returned if the user wants to
    // play the first mission in the pack identified by packid.

    PackId packid;
    if (!ShowDownloadMissionPackForm(&packid)) {
        return;
    }

    // The downloaded a pack identified by packid, and wants to play it.
    // Find the first mission and determine if it's single or multi
    // player. Then, launch the appropriate Play form with this info.
    // It will show, with the appropriate mission highlighted.

    MissionList *pml = CreateMissionList(&packid, kmltAll);
    if (pml == NULL) {
        return;
    }
    if (pml->GetCount() == 0) {
        delete pml;
        return;
    }
    MissionDescription md;
    pml->GetMissionDescription(0, &md);

    // Show the appropriate form. This will cause the first mission from
    // this pack to be highlighted.
 
    if (pml->IsMultiplayerMissionType(md.mt)) {
        PlayMultiplayer(&packid);
    } else {
        PlaySinglePlayer(&packid);
    }
	
    delete pml;
}

bool Shell::PlayMultiplayer(const PackId *ppackid)
{
    Lobby lobby;
    dword result = lobby.Shell(ppackid);
    return result == knShellResultAppStop;
}

bool Shell::PlaySinglePlayer(const PackId *ppackid)
{
    MissionIdentifier miidFind;
    memset(&miidFind, 0, sizeof(miidFind));
    if (ppackid != NULL) {
        miidFind.packid = *ppackid;
    }

    while (true) {
        // First, create a mission list, which is an enumerator for the
        // missions we want to show in this form.

        MissionList *pml = CreateMissionList(NULL, kmltSinglePlayer);
        if (pml == NULL) {
            return true;
        }
		SelectMissionForm *pfrm = (SelectMissionForm *)gpmfrmm->LoadForm(
                gpiniForms, kidfSelectMissionWide,
                new SelectMissionForm(pml, &miidFind));
		if (pfrm == NULL) {
            delete pml;
			return true;
        }

		int idc;
		pfrm->DoModal(&idc);

        MissionIdentifier miid;
        bool fMissionSelected = false;
        if (idc == kidcOk) {
            if (pfrm->GetSelectedMission(&miid)) {
                miidFind = miid;
                fMissionSelected = true;
            }
        }
            
		delete pfrm;
        delete pml;

		// While the dialog was up the player might have exited the app

		if (gevm.IsAppStopping())
			return true;

		if (!fMissionSelected)
			return false;

		int nGo = PlayGame(kpmNormal, &miid, NULL, 0);

        // The next time the SelectMission form shows, highlight the last
        // mission the user was playing.

        miidFind = ggame.GetLastMissionIdentifier();

		if (nGo == knGoAppStop) {
			return true;
        }

		if (nGo == knGoInitFailure) {
			HtMessageBox(kfMbWhiteBorder, "Error", "Unable to load and initialize the mission.");
			continue;
		}
	}

    return false;
}

#if 0
// Returns true if the app is stopping

bool Shell::PlaySinglePlayer()
{
	while (true) {
		ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms, kidfPlaySolo, new ShellForm());
		if (pfrm == NULL)
			return false;
#if !defined(DEV_BUILD)
		pfrm->GetControlPtr(kidcLoadSavedGame)->Show(false);
		pfrm->GetControlPtr(kidcPlaybackGame)->Show(false);
#endif
		int idc;
		pfrm->DoModal(&idc);
		delete pfrm;

		// While the dialog was up the player might have exited the app

		if (gevm.IsAppStopping())
			return true;

		switch (idc) {
		case kidcCancel:
			return false;

		case kidcBeginNewGame:
			return BeginNewGame();

		case kidcPlayChallengeLevel:
			if (PlayChallengeLevel(false))
				return true;
			break;

		case kidcPlayStoryMission:
			if (PlayChallengeLevel(true))
				return true;
			break;

		case kidcPlaybackGame:
			break;

		case kidcLoadSavedGame:
			{
				Stream *pstm = PickLoadGameStream();
				if (pstm == NULL)
					break;

				// Stream is closed/deleted upon return

				int nGo = PlayGame(kpmSavedGame, pstm);

				if (nGo == knGoAppStop)
					return true;

				if (nGo == knGoInitFailure)
					HtMessageBox(kfMbWhiteBorder | kfMbClearDib, "Load Game", "Error loading saved game!");
			}
			break;
		}
	}

	return false;
}
#endif

bool Shell::PlayChallengeLevel(bool fStory)
{
#if 0
	while (true) {
		// Have the player select a level to play

		PickLevelForm *pfrm = (PickLevelForm *)gpmfrmm->LoadForm(gpiniForms, kidfPickLevel, 
				new PickLevelForm(fStory ? kltStory : kltChallenge));
		Assert(pfrm != NULL);
		if (pfrm == NULL)
			return true;

		if (gfDemo) {
			Control *pctl = pfrm->GetControlPtr(kidcOk);
			pctl->Show(false);
		} else {
#if 0
			// UNDONE:
			pctl = GetControlPtr(kidcPleaseRegister);
			pctl->Show(false);
#endif
		}

		int idc;
		pfrm->DoModal(&idc);

		char szLevel[kcbFilename];
		if (idc != kidcCancel)
			strcpy(szLevel, pfrm->m_szLevel);
		delete pfrm;

		// While the dialog was up the player might have exited the app

		if (gevm.IsAppStopping())
			return true;

		if (idc == kidcCancel)
			return false;

		int nGo = PlayGame(kpmNormal, szLevel);
		if (nGo == knGoAppStop)
			return true;

		if (nGo == knGoInitFailure) {
			HtMessageBox(kfMbWhiteBorder, "Error", "Unable to load and initialize the mission.");
			continue;
		}
	}
#endif
    return false;
}

bool Shell::BeginNewGame()
{
#if 0
	bool fNewGame = true;
	int nRank = 0;
	if ((gnDemoRank != 0) && (!gfDemo)) {

		// would you like to replay missions?  
		// UNDONE: Expand HtMessageBox and use it here instead of all this.

		DialogForm *pfrm = (DialogForm *)gpmfrmm->LoadForm(gpiniForms, kidfContinueGame, new DialogForm());
		if (pfrm != NULL) {
				pfrm->SetBorderColorIndex(kiclrWhite);
				pfrm->SetTitleColor(GetColor(kiclrSide1));
				pfrm->SetBackgroundColorIndex(kiclrShadow2x);
				pfrm->SetClearDibFlag();
				
				// position the form

				Rect rcForm;
				pfrm->GetRect(&rcForm);
				DibBitmap *pbm = pfrm->GetFormMgr()->GetDib();
				Size siz;
				pbm->GetSize(&siz);
				int yNew = (siz.cy - rcForm.Height()) / 2;
				int xNew = (siz.cx - rcForm.Width()) / 2;
				rcForm.Offset(xNew - rcForm.left, yNew - rcForm.top);
				pfrm->SetRect(&rcForm);

				int idc;
				pfrm->DoModal(&idc);
				gpmfrmm->RemoveForm(pfrm);

				if (gevm.IsAppStopping())
					return false;


				fNewGame = (idc == kidcOk);
				delete pfrm;
		}

		nRank = fNewGame ? 0 : gnDemoRank;
		gnDemoRank = 0;
		ggame.SavePreferences();
	}
	int nGo = PlayGame(kpmNormal, (void *)(fNewGame ? "S_00.lvl" : "S_03.lvl"), nRank);
	if (nGo == knGoAppStop)
		return true;

	if (nGo == knGoInitFailure)
		HtMessageBox(kfMbWhiteBorder, "Error", "Unable to load and initialize the mission.");

#endif
	return false;
}

#if 0
//
// PickLevelForm implementation
//

PickLevelForm::PickLevelForm(LevelType lt)
{
	m_lt = lt;
	m_szLevel[0] = 0;
}

int CreateLevelList(char **ppsz)
{
	// Get all the .lvl files

	char szFn[kcbFilename];
	int cFiles = 0;

	Enum enm;
	while (gpakr.EnumFiles(&enm, szFn, sizeof(szFn))) {
		int cch = strlen(szFn);
		if (cch < 4)
			continue;
		if (szFn[cch - 4] == '.' && szFn[cch - 3] == 'l' && szFn[cch - 2] == 'v' && szFn[cch - 1] == 'l') {
			strncpyz((char *)&gpbScratch[cFiles * kcbFilename], szFn, kcbFilename);
			cFiles++;
		}
	}

	// Sort them based on filename!

	for (int i = cFiles - 1; i >= 0; i--) {
		for (int j = 1; j <= i; j++) {
			char *pszBack = (char *)&gpbScratch[(j - 1) * kcbFilename];
			char *pszAhead = (char *)&gpbScratch[j * kcbFilename];

			if (strcmp(pszBack, pszAhead) > 0) {
				char szT[kcbFilename];
				strcpy(szT, pszBack);
				strcpy(pszBack, pszAhead);
				strcpy(pszAhead, szT);
			}
		}
	}

	// Alloc a chunk and copy them in

	char *pszT = new char[cFiles * kcbFilename];
	if (pszT == NULL)
		return 0;
	memcpy(pszT, gpbScratch, cFiles * kcbFilename);
	*ppsz = pszT;
	return cFiles;
}

bool PickLevelForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	if (!ShellForm::Init(pfrmm, pini, idf))
		return false;

	// Create a entry for each level

	char szLevel[kcbFilename];
	char szTitle[100];

	ListControl *plstc = (ListControl *)GetControlPtr(kidcLevelList);

	char *aszLevels = NULL;
	int cLevels = CreateLevelList(&aszLevels);
	for (int nLevel = 0; nLevel < cLevels; nLevel++) {
		strncpyz(szLevel, (char *)&aszLevels[nLevel * kcbFilename], sizeof(szLevel));

		// Only show the requested level types

		LevelType lt;
		if (strnicmp(szLevel, "m_", 2) == 0)
			lt = kltMultiplayer;
		else if (strnicmp(szLevel, "s_", 2) == 0)
			lt = kltStory;
		else
			lt = kltChallenge;

		if (lt != m_lt)
			continue;

		// Pull title from level.

#if 1
//faster
		strcpy(szTitle, "<untitled>");
		IniReader *pini = LoadIniFile(gpakr, szLevel);
		if (pini != NULL) {
			pini->GetPropertyValue("General", "Title", szTitle, sizeof(szTitle));
			delete pini;
		}
		plstc->Add(szTitle, (void *)nLevel);
#else
		Level *plvl = new Level();
		plvl->LoadLevelInfo(szLevel);
		plstc->Add(plvl->GetTitle(), (void *)nLevel);
		delete plvl;
#endif
	}

	delete aszLevels;
	return true;
}

void PickLevelForm::OnControlSelected(word idc)
{
	if (idc == kidcOk) {
		ListControl *plstc = (ListControl *)GetControlPtr(kidcLevelList);
		int nLevel = (int)plstc->GetSelectedItemData();
		char *aszLevels = NULL;
		int cLevels = CreateLevelList(&aszLevels);
		if (nLevel >= 0 && nLevel <= cLevels) {
			strcpy(m_szLevel, (char *)&aszLevels[nLevel * kcbFilename]);
			delete aszLevels;
		} else {
			HtMessageBox(kfMbWhiteBorder, "Error!", "First you must select a level to play.");
			delete aszLevels;
			return;
		}
	} else if (idc == kidcLevelList) {
		return;
	}
	EndForm(idc);
}
#endif

//
// ShellForm implementation
//

ShellForm::ShellForm()
{
	m_fCached = false;
	m_fAnimate = true;
	m_fTimerEnabled = false;
}

ShellForm::~ShellForm()
{
	if (m_fTimerEnabled)
		gtimm.RemoveTimer(this);
}

bool ShellForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	if (!Form::Init(pfrmm, pini, idf))
		return false;

	// Keep invisible until the controls are at the right place

	Show(false);

	// Shell forms draw over the whole screen, so size the form to be full screen.
	// This way it is a full screen opaquing form when it comes up, which means things
	// won't try to draw behind it, slowing the game down

	DibBitmap *pbm = m_pfrmm->GetDib();
	Size siz;
	pbm->GetSize(&siz);

	int xNew = (siz.cx - m_rc.Width()) / 2;
	int yNew = (siz.cy - m_rc.Height()) / 2;

	// Reposition the controls

	for (int n = 0; n < m_cctl; n++) {
		Control *pctl = m_apctl[n];
		Rect rcCtl;
		pctl->GetRect(&rcCtl);
		int xNewCtl = rcCtl.left + xNew;
		int yNewCtl = rcCtl.top + yNew;
		pctl->SetPosition(xNewCtl, yNewCtl);
	}
	Rect rcNew;
	rcNew.Set(0, 0, siz.cx, siz.cy);
	SetRect(&rcNew);

	// Set version string - hack

	if (m_idf == kidfStartup) {
		LabelControl *pctl = (LabelControl *)GetControlPtr(kidcVersion);
		if (pctl != NULL) {
			// If no version string, exe+data are unmarked. Show date/time
			// Use the version string verbatim so we show extra text at the end

			char szT[64];
			if (!ggame.GetFormattedVersionString(gszVersion, szT)) {
				szT[0] = 0;
#ifdef DEV_BUILD
				strcat(szT, "DEV BUILD ");
#endif
				strcat(szT, __DATE__);
				strcat(szT, ", ");
				strcat(szT, __TIME__);
			} else {
				szT[0] = 'v';
				strncpyz(&szT[1], gszVersion, sizeof(szT));
			}
#if __LP64__
			strcat(szT, " (64 bit)");
#endif
			pctl->SetText(szT);
		}
	}

	return true;
}

#define kctRate 2
#define kcFcZip 18
bool ShellForm::DoModal(int *pnResult, bool fAnimate, bool fShowSound)
{
	m_fAnimate = fAnimate;
	if (!m_fAnimate) {
		if (fShowSound)
			gsndm.PlaySfx(ksfxGuiFormShow);
		return Form::DoModal(pnResult, (Sfx)-1, (Sfx)-1);
	}

	// Take over form show sound playing

	Size siz;
	ggame.GetPlayfieldSize(&siz);

	// Reposition all the form's controls of the form so they can be zipped in

	for (int i = 0; i < m_cctl; i++) {
		Control *pctl = m_apctl[i];
		
		Rect rcCtl;
		pctl->GetRect(&rcCtl);

		// Remember where the control is supposed to end up

		m_axDst[i] = (rcCtl.left & ~1);

		// Odd index controls fly in from the left, even from the right

		if (i & 1) {
			pctl->SetPosition(rcCtl.left - siz.cx - (i * PcFromFc(kcFcZip)), rcCtl.top);
		} else {
			pctl->SetPosition(rcCtl.left + siz.cx + (i * PcFromFc(kcFcZip)), rcCtl.top);
		}
	}

	// Now that the controls are positioned right make the form visible

	Show(true);

	if (fShowSound)
		m_wf |= kfFrmShowSound;

	// Start timer

	gtimm.AddTimer(this, kctRate);
	m_fTimerEnabled = true;
	m_fCached = false;
	m_tLast = 0;

	// Some controls might be added on the fly after ShellForm::Init. Since we 
	// won't have their destination stashed away we'll just ignore them.

	m_cctlToZip = m_cctl;

	return Form::DoModal(pnResult, (Sfx)-1, (Sfx)-1);
}

void ShellForm::OnTimer(long tCurrent)
{
	Assert(m_fAnimate);

	int ct = (int)(tCurrent - m_tLast);
	m_tLast = tCurrent;

	// Waiting until all the pieces have been cached before doing absolute
	// timing. This gives smoother movement.

	int pcMove = PcFromFc(kcFcZip);
	if (m_fCached)
		pcMove = ((ct + kctRate / 2) / kctRate) * PcFromFc(kcFcZip);

	bool fZipping = false;

	for (int i = 0; i < m_cctlToZip; i++) {
		Control *pctl = m_apctl[i];

		Rect rcCtl;
		pctl->GetRect(&rcCtl);

		int xDst = m_axDst[i];
		if (rcCtl.left == xDst)
			continue;

		fZipping = true;

		int x;
		if (xDst > rcCtl.left)
			x = _min(rcCtl.left + pcMove, xDst);
		else
			x = _max(rcCtl.left - pcMove, xDst);

		pctl->SetPosition(x & ~1, rcCtl.top);
	}

	gevm.SetRedrawFlags(kfRedrawDirty | kfRedrawBeforeTimer);

	if (!fZipping) {
		gtimm.RemoveTimer(this);
		m_fTimerEnabled = false;
		if (m_wf & kfFrmShowSound)
			gsndm.PlaySfx(ksfxGuiFormShow);
        OnZipDone();
	}
}

void ShellForm::OnPaintBackground(DibBitmap *pbm, UpdateMap *pupd)
{
	// Draw the fancy background bitmap

	Size sizDib;
	pbm->GetSize(&sizDib);
	TBitmap *ptbm = CreateTBitmap("titlescreenbkgd.png");
	Size sizBmp;
	ptbm->GetSize(&sizBmp);
    
    // Draw middle
	Rect rcBmp;
	rcBmp.left = ((sizDib.cx - sizBmp.cx) / 2) & ~1;
	rcBmp.top = (sizDib.cy - sizBmp.cy) / 2;
	rcBmp.right = rcBmp.left + sizBmp.cx;
	rcBmp.bottom = rcBmp.top + sizBmp.cy;
	BltHelper(pbm, ptbm, pupd, rcBmp.left, rcBmp.top);

#if 0
    // Draw left
    if (rcBmp.left > 0) {
        BltHelper(pbm, prbm, pupd, rcBmp.left - sizBmp.cx, rcBmp.top);
    }
    
    // Draw right
    if (rcBmp.right < sizDib.cx) {
        BltHelper(pbm, prbm, pupd, rcBmp.right, rcBmp.top);
    }
#endif
    
	delete ptbm;
    
	// If the screen is wider than the form we clear those areas
	// out first to the form's background color

	Size siz;
	pbm->GetSize(&siz);
	if (rcBmp.top > 0) {
		Rect rc;
		rc.Set(0, 0, siz.cx, rcBmp.top);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}
	if (rcBmp.bottom < siz.cy) {
		Rect rc;
		rc.Set(0, rcBmp.bottom, siz.cx, siz.cy);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}
    
	if (rcBmp.left > 0) {
		Rect rc;
		rc.Set(0, rcBmp.top, rcBmp.left, rcBmp.bottom);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}
	if (rcBmp.right < siz.cx) {
		Rect rc;
		rc.Set(rcBmp.right, rcBmp.top, siz.cx, rcBmp.bottom);
		FillHelper(pbm, pupd, &rc, GetColor(kiclrBlack));
	}

	// Don't start timing for absolute positioned animation until we've
	// loaded the cache at least once. Gives smoother animation

	if (!m_fCached) {
		m_fCached = true;
		m_tLast = gtimm.GetTickCount();
	}
}

//
// RegisterNowForm
//

class RegisterNowForm : public ShellForm
{
public:
	virtual bool OnPenEvent(Event *pevt) secGameOptionsForm;
};

void DoRegisterNowForm()
{
	ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms, kidfRegisterNow, new RegisterNowForm());
	if (pfrm != NULL) {
		pfrm->DoModal();
		delete pfrm;
	}
}

bool RegisterNowForm::OnPenEvent(Event *pevt)
{
	if (pevt->eType == penDownEvent) {
		EndForm(kidcOk);
		return true;
	}
	return false;
}

} // namespace wi
