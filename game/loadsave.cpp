#include "ht.h"

namespace wi {

Stream *PickLoadGameStream()
{
	LoadGameForm *pfrm = (LoadGameForm *)gpmfrmm->LoadForm(gpiniForms, kidfLoadGame, new LoadGameForm());
	if (pfrm == NULL)
		return NULL;
	pfrm->SelectLast(true);

	Stream *pstm = NULL;
	while (true) {
		int nGame = -1;
		if (!pfrm->DoModal(&nGame)) {
			delete pfrm;
			return NULL;
		}

		pstm = HostOpenSaveGameStream(nGame);
		if (pstm != NULL)
			break;
		pfrm->Show(true);
		HtMessageBox(kfMbWhiteBorder, "Load Saved Game", "Please select a valid saved game to load first!");
	}

	delete pfrm;
	return pstm;
}

bool PickSaveGameStream(Stream **ppstm)
{
	LoadGameForm *pfrm = (LoadGameForm *)gpmfrmm->LoadForm(gpiniForms, kidfSaveGame, new LoadGameForm());
	if (pfrm == NULL)
		return false;
	pfrm->SelectLast(false);
	int nGame = -1;
	if (!pfrm->DoModal(&nGame)) {
		delete pfrm;
		return false;
	}
	delete pfrm;
	*ppstm = HostNewSaveGameStream(nGame, (char *)gsim.GetLevel()->GetTitle());
	return true;
}

void DeleteStaleSaveGames()
{
	for (int nGame = knGameReinitializeSave; nGame < 20;) {
		Stream *pstm = HostOpenSaveGameStream(nGame, false);
		if (pstm != NULL) {
			char szVersion[32];
			szVersion[0] = 0;
			pstm->ReadString(szVersion, sizeof(szVersion));
			byte bVer = pstm->ReadByte();
			byte bPlatform = 0;
			if (bVer >= 6)
				bPlatform = pstm->ReadByte();
			pstm->Close();
			delete pstm;

			if (!CheckSaveGameVersion(szVersion, bPlatform))
				HostDeleteSaveGame(NULL, nGame);
		}

		// Ugly

		if (nGame == knGameReinitializeSave) {
			nGame = 0;
		} else {
			nGame++;
		}
	}

	// delete any crashed save games

	HostDeleteSaveGame(kszTempName, 0);
}

bool CheckSaveGameVersion(char *pszVersion, byte bPlatform)
{
#ifndef DEV_BUILD
#ifdef PIL
#ifdef PNO
	if (bPlatform != 1)
		return false;
#else
	if (bPlatform != 0)
		return false;
#endif
#endif
#endif

	// If not upwardly compatible with ship version and major version, delete the save game.
	// This is so v1.0a allows reading save game files from v1.0, etc.

	int nShipVersionSaveGame;
	int nMajorVersionSaveGame;
	char chMinorVersionSaveGame;
	ggame.ParseVersion(pszVersion, &nShipVersionSaveGame, &nMajorVersionSaveGame, &chMinorVersionSaveGame);
	return ggame.IsVersionCompatibleWithExe(nShipVersionSaveGame, nMajorVersionSaveGame, chMinorVersionSaveGame, true);
}

//
// LoadGameFrom
//

LoadGameForm::LoadGameForm()
{
	m_nGameLast = -1;
}

bool LoadGameForm::Init(FormMgr *pfrmm, IniReader *pini, word idf)
{
	if (!ShellForm::Init(pfrmm, pini, idf))
		return false;

	ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
	Rect rc;
	plstc->GetSubRects(&rc);
	int cEntries = rc.Height() / (gapfnt[kifntDefault]->GetHeight() + gcxyBorder);
	Date dateLast;
	dateLast.nYear = 0;
	int nHours24Last = -1;
	int nMinutesLast = -1;
	int nSecondsLast = -1;
	int nGameLast = -1;
	int nGame;
	for (nGame = 0; nGame < cEntries; nGame++) {
		int nHours24;
		int nMinutes;
		int nSeconds;
		char szLevel[64];
		char szT[64];
		Date date;
		if (HostGetSaveGameName(nGame, szLevel, sizeof(szLevel), &date, &nHours24, &nMinutes, &nSeconds)) {
			sprintf(szT, "%02d:%02d %s", nHours24, nMinutes, szLevel);

			// Remember last saved game

			bool fGreater = false;
			switch (CompareDates(&dateLast, &date)) {
			case 1:
				fGreater = false;
				break;

			case -1:
				fGreater = true;
				break;

			case 0:
				if (nHours24 > nHours24Last)
					fGreater = true;
				if (nHours24 == nHours24Last && nMinutes > nMinutesLast)
					fGreater = true;
				if (nHours24 == nHours24Last && nMinutes == nMinutesLast && nSeconds >= nSecondsLast)
					fGreater = true;
				break;
			}
			if (fGreater) {
				nGameLast = nGame;
				dateLast = date;
				nHours24Last = nHours24;
				nMinutesLast = nMinutes;
				nSecondsLast = nSeconds;
			}
		} else {
			strcpy(szT, szLevel);
		}
		plstc->Add(szT, (void *)(nGame + 1));
	}
	m_nGameLast = nGameLast;

	return true;
}

void LoadGameForm::SelectLast(bool fLast)
{
	ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
	int nGameSelect;
	if (fLast) {
		nGameSelect = m_nGameLast;
		if (nGameSelect == -1)
			nGameSelect = 0;
	} else {
		nGameSelect = m_nGameLast + 1;
		if (nGameSelect == plstc->GetCount())
			nGameSelect = 0;
	}
	plstc->Select(nGameSelect);
}

void LoadGameForm::OnControlSelected(word idc)
{
	switch (idc) {
	case kidcOk:
		{
			ListControl *plstc = (ListControl *)GetControlPtr(kidcGameList);
			int nGame = ((int)plstc->GetSelectedItemData()) - 1;
			EndForm(nGame);
		}
		return;

	case kidcCancel:
		ShellForm::OnControlSelected(idc);
		break;
	}
}

} // namespace wi