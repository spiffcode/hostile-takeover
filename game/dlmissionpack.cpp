#include "game/ht.h"
#include "game/httpindexloader.h"
#include "game/httppackinfomanager.h"
#include "game/httppackmanager.h"
#include "base/format.h"
#include "base/thread.h"
#include "yajl/wrapper/jsontypes.h"
#include <ctype.h>

namespace wi {

#define CTX_INDEX ((void *)0)
#define CTX_INFO ((void *)1)

#define PACKID_CUSTOMURL 0xFEFEFEFE

class DownloadMissionPackForm : public ShellForm, ProgressCallback {
public:
    DownloadMissionPackForm(bool fMultiplayer) : fMultiplayer_(fMultiplayer),
            idxl_(gphttp, gppackm), fWantsPlay_(false) {}
    ~DownloadMissionPackForm();

    bool WantsPlay(PackId *ppackid) {
        if (fWantsPlay_) {
            *ppackid = packidPlay_;
            return true;
        }
        return false;
    }

	// Form overrides
    virtual bool Init(FormMgr *pfrmm, IniReader *pini, word idf);
    virtual void OnControlSelected(word idc);
    virtual void OnControlNotify(word idc, int nNotify);
	virtual bool OnPenEvent(Event *pevt);
    virtual bool OnFilterEvent(Event *pevt);

private:
    void HandleCustomURL();
    void Sort(word id);
    void PositionColumns();
    void HideShow();
    void ShowStatus(void *ctx, const char *psz);
    void ShowIndex(const PackId *ppackidSelect = NULL);
    bool ShowInfo(bool fRequest);
    const char *GetItemString(const IndexEntry *pentry);
    const char *GetString(const json::JsonMap *map, const char *key,
            const char *missing);
    void UpdateLabelRects(LabelControl *plbl);

    // ProgressCallback
    virtual void OnBegin(void *ctx, int cbLength);
    virtual void OnProgress(void *ctx, int cbTotal, int cbLength);
    virtual void OnFinished(void *ctx);
    virtual void OnError(void *ctx, const char *pszError);

    HttpIndexLoader idxl_;
    int x0_, x1_, x2_, x3_;
    word idSortIndicator_;
    HttpIndexLoader::SortType sort_;
    Rect rcStatus_;
    Rect rcTitle_;
    Rect rcNumPlayers_;
    Rect rcNumMissions_;
    bool fWantsPlay_;
    PackId packidPlay_;
    bool fMultiplayer_;
};

bool ShowDownloadMissionPackForm(PackId *ppackid) {
    bool fMultiplayer = false;
#ifdef MULTIPLAYER
    // First, choose single, or multiplayer. This UI added due to feedback.
    ShellForm *pfrmT = (ShellForm *)gpmfrmm->LoadForm(gpiniForms,
            kidfAddOnSingleMulti, new ShellForm());
    if (pfrmT == NULL) {
        return false;
    }
    int idc;
    pfrmT->DoModal(&idc);
    delete pfrmT;
    if (idc == kidcCancel || gevm.IsAppStopping()) {
        return false;
    }
    if (idc == kidcPlayMultiPlayer) {
        fMultiplayer = true;
    }
#endif

    // Bring up the download form
    DownloadMissionPackForm *pfrm;
    pfrm = (DownloadMissionPackForm *)gpmfrmm->LoadForm(gpiniForms,
            kidfDownloadMissionPackWide, new DownloadMissionPackForm(
            fMultiplayer));
    if (pfrm != NULL) {
        pfrm->DoModal();
        if (pfrm->WantsPlay(ppackid)) {
            delete pfrm;
            return true;
        }
        delete pfrm;
    }
    return false;
}

DownloadMissionPackForm::~DownloadMissionPackForm() {
    gppim->Reset();
}

bool DownloadMissionPackForm::Init(FormMgr *pfrmm, IniReader *pini, word idf) {
	if (!ShellForm::Init(pfrmm, pini, idf)) {
		return false;
    }

    // Start downloading the mission pack index

    if (!idxl_.Start(CTX_INDEX, this)) {
        return false;
    }

    // No operation is available until a mission is selected

    GetControlPtr(kidcOk)->Show(false);
    GetControlPtr(kidcDiscuss)->Show(false);

    // Show status

    ShowStatus(CTX_INDEX, "Contacting Server...");

    // Position Column labels
    
    PositionColumns();

    // Set info label to use ellipsis

    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMissionPackInfo);
    plbl->SetFlags(plbl->GetFlags() | kfLblEllipsis);

    // No sorting yet

    idSortIndicator_ = 0;
    sort_ = HttpIndexLoader::SORT_UNSORTED;

    return true;
}

void DownloadMissionPackForm::PositionColumns() {
    // Calc tab stops in the list
    // S  Title # Players Downloads
    // S is + for installed, or ! for upgrade
    // Title is the title. Make this as wide as possible
    // # Players is as close to Downloads as is reasonable
    // # Missions is right aligned

    ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
    Font *pfnt = plstc->GetFont();
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcStatus);
#if 1
    plbl->SetText("");
    int cxPlus = pfnt->GetTextExtent("+");
#else
    int cxPlus = pfnt->GetTextExtent(plbl->GetText());
#endif
    plbl = (LabelControl *)GetControlPtr(kidcTitle);
    int cxTitle = pfnt->GetTextExtent(plbl->GetText());
    plbl = (LabelControl *)GetControlPtr(kidcNumPlayers);
    int cxNumPlayers = pfnt->GetTextExtent(plbl->GetText());
    int cxAsterisk = pfnt->GetTextExtent("*");
    plbl = (LabelControl *)GetControlPtr(kidcNumMissions);
    int cxNumMissions = pfnt->GetTextExtent(plbl->GetText());

    Rect rcList;
    plstc->GetRect(&rcList);

    int xStatus = rcList.left + 2;
    int xTitle = rcList.left + (rcList.Width() / 4 - cxTitle) / 2;
    int xNumMissions = m_rc.Width() - cxAsterisk - cxNumMissions;
    int xNumPlayers = xNumMissions - 10 - cxNumPlayers;

    // Calc the top of the list (past the arrow) for better column
    // label hittesting
    Size sizArrow;
    ListControl::s_ptbmScrollUpUp->GetSize(&sizArrow);
    int yListTop = rcList.top + sizArrow.cy;

    // Set the label positions
    // Enable the labels for hittesting

    Rect rc;
    plbl = (LabelControl *)GetControlPtr(kidcStatus);
    plbl->GetRect(&rc);
    rc.left = xStatus;
    rc.right = xTitle;
    rc.bottom += (yListTop - rc.bottom) / 2;
    plbl->SetRect(&rc);
    plbl->SetFlags(plbl->GetFlags() | kfLblHitTest);
    rcStatus_ = rc;

    plbl = (LabelControl *)GetControlPtr(kidcTitle);
    plbl->GetRect(&rc);
    rc.Offset(xTitle - rc.left, 0);
    rc.right += rc.Width();
    rc.bottom += (yListTop - rc.bottom) / 2;
    plbl->SetRect(&rc);
    plbl->SetFlags(plbl->GetFlags() | kfLblHitTest);
    rcTitle_ = rc;
    
    plbl = (LabelControl *)GetControlPtr(kidcNumPlayers);
    plbl->GetRect(&rc);
    rc.Offset(xNumPlayers - rc.left, 0);
    rc.bottom += (yListTop - rc.bottom) / 2;
    plbl->SetRect(&rc);
    plbl->SetFlags(plbl->GetFlags() | kfLblHitTest);
    rcNumPlayers_ = rc;

    plbl = (LabelControl *)GetControlPtr(kidcNumMissions);
    plbl->GetRect(&rc);
    rc.Offset(xNumMissions - rc.left, 0);
    rc.bottom += (yListTop - rc.bottom) / 2;
    plbl->SetRect(&rc);
    plbl->SetFlags(plbl->GetFlags() | kfLblHitTest);
    rcNumMissions_ = rc;

    // Remember the tab settings for later use

    x0_ = 0;
    x1_ = cxPlus + 4;
    x2_ = xNumPlayers + cxNumPlayers / 2;
    x3_ = xNumMissions + cxNumMissions / 2;
}

bool DownloadMissionPackForm::OnPenEvent(Event *pevt) {
    if (pevt->eType != penDownEvent) {
        return ShellForm::OnPenEvent(pevt);
    }

    Control *pctlCaptureBefore = GetControlCapture();
    bool f = ShellForm::OnPenEvent(pevt);
    Control *pctlCaptureAfter = GetControlCapture();

    if (pctlCaptureBefore == NULL && pctlCaptureAfter != NULL) {
        Sort(pctlCaptureAfter->GetId());
    }
    return f;
}

void DownloadMissionPackForm::Sort(word id) {
    word aidColumns[] = {
        kidcStatus, kidcTitle, kidcNumPlayers, kidcNumMissions
    };

    int iCol;
    for (iCol = 0; iCol < ARRAYSIZE(aidColumns); iCol++) {
        if (id == aidColumns[iCol]) {
            break;
        }
    }
    if (iCol >= ARRAYSIZE(aidColumns)) {
        return;
    }

    HttpIndexLoader::SortType asortFlip[] = {
        HttpIndexLoader::SORT_UNSORTED,
        HttpIndexLoader::SORT_INSTALLEDDESCENDING,
        HttpIndexLoader::SORT_INSTALLEDASCENDING,
        HttpIndexLoader::SORT_TITLEDESCENDING,
        HttpIndexLoader::SORT_TITLEASCENDING,
        HttpIndexLoader::SORT_PLAYERSDESCENDING,
        HttpIndexLoader::SORT_PLAYERSASCENDING,
        HttpIndexLoader::SORT_MISSIONSDESCENDING,
        HttpIndexLoader::SORT_MISSIONSASCENDING
    };

    // If this is already the sort column, flip the sort
    if (id == idSortIndicator_) {
        sort_ = asortFlip[sort_];
        idxl_.Sort(sort_);
        ShowIndex();
        return;
    }

    // Take sort indicator off
    char szT[32];
    if (idSortIndicator_ != 0) {
        LabelControl *plbl = (LabelControl *)GetControlPtr(idSortIndicator_);
        strncpyz(szT, plbl->GetText(), sizeof(szT));
        int cch = strlen(szT);
        if (szT[cch - 1] == '*') {
            szT[cch - 1] = 0;
            plbl->SetText(szT);
        }
        UpdateLabelRects(plbl);
    }

    // Put on new column
    idSortIndicator_ = id;
    LabelControl *plbl = (LabelControl *)GetControlPtr(idSortIndicator_);
    strncpyz(szT, plbl->GetText(), sizeof(szT));
    int cch = strlen(szT);
    if (szT[cch - 1] != '*') {
        szT[cch] = '*';
        szT[cch + 1] = 0;
        plbl->SetText(szT);
        UpdateLabelRects(plbl);
    }

    // Force this before the sort occurs, for immediate visual feedback

    gpmfrmm->DrawFrame(false);

    // Perform default sort for this column
    HttpIndexLoader::SortType asortDefault[] = {
        HttpIndexLoader::SORT_INSTALLEDDESCENDING,
        HttpIndexLoader::SORT_TITLEASCENDING,
        HttpIndexLoader::SORT_PLAYERSASCENDING,
        HttpIndexLoader::SORT_MISSIONSDESCENDING
    };

    sort_ = asortDefault[iCol];
    idxl_.Sort(sort_);
    ShowIndex();
}

void DownloadMissionPackForm::UpdateLabelRects(LabelControl *plbl) {
    switch (plbl->GetId()) {
    case kidcStatus:
        plbl->SetRect(&rcStatus_);
        break;

    case kidcTitle:
        plbl->SetRect(&rcTitle_);
        break;

    case kidcNumPlayers:
        plbl->SetRect(&rcNumPlayers_);
        break;

    case kidcNumMissions:
        plbl->SetRect(&rcNumMissions_);
        break;
    }
}

void DownloadMissionPackForm::ShowStatus(void *ctx, const char *psz) {
    if (ctx == CTX_INDEX) {
        ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
        plstc->Clear();
        plstc->SetTabStops(0);
        plstc->SetTabFlags(0);
        plstc->Add(psz, NULL);
    }

    if (ctx == CTX_INFO) {
        LabelControl *plbl =
                (LabelControl *)GetControlPtr(kidcMissionPackInfo);
        plbl->SetText((char *)psz);
    }

    // Force the frame draw, since this gets called while the main loop
    // is "asleep".
    gpmfrmm->DrawFrame(false);
}

void DownloadMissionPackForm::OnBegin(void *ctx, int cbLength) {
    OnProgress(ctx, 0, cbLength);
}

void DownloadMissionPackForm::OnProgress(void *ctx, int cbTotal, int cbLength) {
    ShowStatus(ctx, base::Format::ToString("Downloaded %d of %d bytes...",
            cbTotal, cbLength));
}

void DownloadMissionPackForm::OnError(void *ctx, const char *pszError) {
    // Don't show info errors since it's ok to not have info, and the error
    // is confusing
    if (ctx == CTX_INFO) {
        ShowStatus(ctx, "");
    } else {
        ShowStatus(ctx, base::Format::ToString("Download error: %s",
                pszError));
    }
}

void DownloadMissionPackForm::OnFinished(void *ctx) {
    if (ctx == CTX_INDEX) {
        ShowStatus(ctx, "Download Complete! Sorting results...");

        // Merge with installed packs so they are all listed

        idxl_.MergeInstalled(gppackm);

        // Add the fake entry for custom URLs

        PackId packid;
        memset(&packid, 0, sizeof(packid));
        packid.id = PACKID_CUSTOMURL;
        idxl_.AddFakeEntry(&packid, " Custom URL...", 999, 999, 0);

        // Sort the single list
    
        Sort(kidcNumMissions);
    }

    if (ctx == CTX_INFO) {
        ShowInfo(false);
    }

    // Wake up to process the paint

    base::Thread::current().Post(base::kidmNullEvent, NULL); 
}

const char *DownloadMissionPackForm::GetItemString(const IndexEntry *pentry) {
    // Figure out installed / upgrade state

    char s;
    switch (gppackm->IsInstalled(&pentry->packid)) {
    default:
    case 0:
        // Not installed
        s = ' ';
        break;

    case 1:
        // Installed, current version is up to date
        s = '+';
        break;

    case 2:
        // Installed, but needs upgrade
        s = '!';
        break;
    }

    char szPlayers[8];
    if (pentry->cPlayersMin == pentry->cPlayersMax) {
        strncpyz(szPlayers, base::Format::ToString("%d", pentry->cPlayersMin),
                sizeof(szPlayers));
    } else {
        strncpyz(szPlayers, base::Format::ToString("%d-%d",
                pentry->cPlayersMin, pentry->cPlayersMax), sizeof(szPlayers));
    }

    if (pentry->packid.id == PACKID_CUSTOMURL) {
        return base::Format::ToString(" \t%s\t \t ", pentry->title.c_str());
    } else {
        return base::Format::ToString("%c\t%s\t%s\t%d",
                s, pentry->title.c_str(), szPlayers, pentry->cMissions);
    }
}

void DownloadMissionPackForm::ShowIndex(const PackId *ppackidSelect) {
    // Add entries to the list

    ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
    plstc->SetTabStops(x0_, x1_, x2_, x3_);
    plstc->SetTabFlags(kfLstTabCenter, kfLstTabEllipsis, kfLstTabCenterOn,
            kfLstTabCenterOn);
    plstc->Clear();
    int iSelect = 0;
    int cAdded = 0;
    int c = idxl_.GetCount();
    for (int i = 0; i < c; i++) {
        const IndexEntry *pentry = idxl_.GetEntry(i);

        // Show either single player packs, or multiplayer packs.
        // If a pack has both single and multiplayer missions, show
        // these in both cases.

        if (pentry->packid.id != PACKID_CUSTOMURL) {
            if (fMultiplayer_) {
                if (pentry->cPlayersMax == 1) {
                    continue;
                }
            } else {
                if (pentry->cPlayersMin != 1) {
                    continue;
                }
            }
        }

        // Remember which entry to select

        if (iSelect == 0 && ppackidSelect != NULL) {
            if (memcmp(ppackidSelect, &pentry->packid,
                    sizeof(*ppackidSelect)) == 0) {
                iSelect = cAdded;
            }
        }

        plstc->Add(GetItemString(pentry), (void *)pentry);
        cAdded++;
    }

    if (cAdded > 0) {
        plstc->Select(iSelect, true, true);
    }
}

bool DownloadMissionPackForm::ShowInfo(bool fRequest) {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
    IndexEntry *entry = (IndexEntry *)plstc->GetSelectedItemData();
    if (entry == NULL) {
        GetControlPtr(kidcDiscuss)->Show(false);
        return false;
    }
    const json::JsonMap *map = gppim->GetInfoMap(&entry->packid);
    if (map == NULL) {
        if (fRequest) {
            if (entry->packid.id == PACKID_CUSTOMURL) {
               ShowStatus(CTX_INFO, "Press Download to download from a custom URL");
            } else {
                gppim->Start(&entry->packid, CTX_INFO, this);
                ShowStatus(CTX_INFO, "Loading Mission Pack info...");
            }
        }
        GetControlPtr(kidcDiscuss)->Show(false);
        return false;
    }

    const char *author = GetString(map, "a", "unknown");
    const char *desc = GetString(map, "d", "no description");
    const char *s = base::Format::ToString("By %s: %s", author, desc);
    delete map;

    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMissionPackInfo);
    plbl->SetText((char *)s);
    GetControlPtr(kidcDiscuss)->Show(true);
    return true;
}

const char *DownloadMissionPackForm::GetString(const json::JsonMap *map,
        const char *key, const char *missing) {
    const json::JsonObject *obj = map->GetObject(key);
    if (obj == NULL || obj->type() != json::JSONTYPE_STRING) {
        return missing;
    }
    const json::JsonString *s = (json::JsonString *)obj;

    bool fWhitespace = true;
    char ch; 
    const char *psz = s->GetString();
    while ((ch = *psz++) != 0) {
        if (!isspace(ch)) {
            fWhitespace = false;
            break;
        }
    }
    if (fWhitespace) {
        return missing;
    }
    return s->GetString();
}

void DownloadMissionPackForm::OnControlNotify(word idc, int nNotify) {
    if (idc == kidcMissionPackList && nNotify == knNotifySelectionChange) {
        HideShow();
    }
    Form::OnControlNotify(idc, nNotify);
}

void DownloadMissionPackForm::HideShow() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMissionPackInfo);
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    int selected = plstc->GetSelectedItemIndex();
    if (selected < 0) {
        gppim->Reset();
        plbl->SetText("");
        pbtn->Show(false);
        GetControlPtr(kidcDiscuss)->Show(false);
        return;
    }

    // Get the pack info for this pack
       
    IndexEntry *entry = (IndexEntry *)plstc->GetSelectedItemData();
    char *psz = NULL;
    switch (gppackm->IsInstalled(&entry->packid)) {
    case 0:
        psz = "DOWNLOAD";
        break;

    case 1:
        psz = "REMOVE";
        break;

    case 2:
        psz = "UPGRADE";
        break;
    }
    if (psz != NULL) {
        pbtn->SetText(psz);
    }

    pbtn->Show(true);
    ShowInfo(true);
}

void DownloadMissionPackForm::OnControlSelected(word idc) {
    if (idc == kidcCancel) {
        EndForm(idc);
        return;
    }

    ListControl *plstc = (ListControl *)GetControlPtr(kidcMissionPackList);
    IndexEntry *entry = (IndexEntry *)plstc->GetSelectedItemData();
    if (entry == NULL) {
        return;
    }

    switch (idc) {
    case kidcOk:
        switch (gppackm->IsInstalled(&entry->packid)) {
        case 0:
            // If this is a custom URL entry handle specially
            if (entry->packid.id == PACKID_CUSTOMURL) {
                HandleCustomURL();
                break;
            }
            // fall through

        case 2:
            // Pack not install or needs upgrade. In either case, download
            if (DownloadMissionPack(&entry->packid, NULL, true)) {
                fWantsPlay_ = true;
                packidPlay_ = entry->packid;
                EndForm(kidcOk);
            }
            plstc->SetSelectedItemText(GetItemString(entry));
            HideShow();
            break;

        case 1:
            // Pack is installed, so this action means remove
            if (HtMessageBox(kidfMessageBoxQuery,
                    kfMbWhiteBorder | kfMbKeepTimersEnabled,
                    "Remove Mission Pack", "Are you sure?")) {
                gppackm->Remove(&entry->packid);
                if (idxl_.OnRemoved(&entry->packid)) {
                    ShowIndex();
                } else {
                    plstc->SetSelectedItemText(GetItemString(entry));
                }
                HideShow();
            }
            break;
        }
        break;

    case kidcDiscuss:
        {
            json::JsonMap *map = gppim->GetInfoMap(&entry->packid);
            if (map != NULL) {
                const json::JsonObject *obj = map->GetObject("di");
                if (obj != NULL && obj->type() == json::JSONTYPE_STRING) {
                    HostOpenUrl(((const json::JsonString *)obj)->GetString());
                }
                delete map;
            }
        }
        break;
    }
}

bool DownloadMissionPackForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType != askStringEvent) {
        return false;
    }

    // Get the URL and Save preferences so this url is remembered
    HostGetAskString(gszAskURL, sizeof(gszAskURL));
    ggame.SavePreferences();

    // Download this custom pack
    PackId packid;
    bool play = false;
    if (!DownloadMissionPackFromURL(gszAskURL, &packid, &play)) {
        // Some kind of error occured
        return true;
    }

    // User wants to play now?
    if (play) {
        // The user wants to play the mission
        fWantsPlay_ = true;
        packidPlay_ = packid;
        EndForm(kidcOk);
        return true;
    }

    // Add this mission pack to the index, and re-fill the list control,
    // and select it.
    idxl_.AddIndexEntry(&packid);
    idxl_.Sort(sort_);
    ShowIndex(&packid);
    return true;
}

void DownloadMissionPackForm::HandleCustomURL() {
    HostInitiateAsk("Custom URL", -1, gszAskURL, knKeyboardAskURL);
}

} // namespace wi
