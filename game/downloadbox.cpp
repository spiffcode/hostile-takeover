#include "game/ht.h"
#include "game/httppackmanager.h"
#include "game/httppackinfomanager.h"
#include "game/progresscallback.h"
#include "base/messagequeue.h"
#include "base/format.h"
#include "base/thread.h"
#include "yajl/wrapper/jsontypes.h"

namespace wi {

class DownloadBox : public DialogForm, ProgressCallback {
public:
    DownloadBox(const PackId *ppackid, const char *title, bool fPlayButton);
    DownloadBox(const char *pszURL);
    virtual ~DownloadBox();

	virtual bool DoModal(int *pnResult = NULL, Sfx sfxShow = ksfxGuiFormShow,
            Sfx sfxHide = ksfxGuiFormHide);
    const PackId& packid() { return packid_; }
    bool error() { return error_; }

private:
    void ShowStatus(const char *pszStatus);

    // ProgressCallback
    virtual void OnBegin(void *ctx, int cbLength);
    virtual void OnProgress(void *ctx, int cbTotal, int cbLength);
    virtual void OnFinished(void *ctx);
    virtual void OnError(void *ctx, const char *pszError);

    PackId packid_;
    const char *pszTitle_;
    int xOk_;
    bool fPlayButton_;
    const char *pszURL_;
    bool error_;
};

bool DownloadMissionPack(const PackId *ppackid, const char *pszTitle,
        bool fPlayButton) {
	DownloadBox *pfrm = (DownloadBox *)gpmfrmm->LoadForm(gpiniForms,
            kidfDownloadBox, new DownloadBox(ppackid, pszTitle, fPlayButton));
    int nResult = 0;
    if (pfrm != NULL) {
        pfrm->DoModal(&nResult, (Sfx)-1, (Sfx)-1);
        delete pfrm;
    }
    return nResult == kidcPlay;
}

bool DownloadMissionPackFromURL(const char *pszURL, PackId *ppackid,
        bool *play) {
	DownloadBox *pfrm = (DownloadBox *)gpmfrmm->LoadForm(gpiniForms,
            kidfDownloadBox, new DownloadBox(pszURL));
    int nResult = 0;
    bool error = false;
    if (pfrm != NULL) {
        pfrm->DoModal(&nResult, (Sfx)-1, (Sfx)-1);
        *ppackid = pfrm->packid();
        error = pfrm->error();
        delete pfrm;
    }
    if (error) {
        return false;
    }
    *play = (nResult == kidcPlay);
    return true;
}

bool AskInstallMissionPack(const PackId *ppackid, const char *pszUITitle,
        const char *pszPackTitle) {
    char *pszT = NULL;
    switch (gppackm->IsInstalled(ppackid)) {
    case 0:
        // Not installed
        pszT = "You must install the mission pack first. Download now?";
        break;

    case 1:
        // Installed already
        return true;

    case 2:
        // needs upgrade
        pszT = "You must upgrade the mission pack first. Download now?";
        break;
    }

    // Ask the user
    if (HtMessageBox(kidfMessageBoxQuery, kfMbWhiteBorder |
            kfMbKeepTimersEnabled, pszUITitle, pszT)) {

        // Load the pack info so it's available when needed
        gppim->Start(ppackid, NULL, NULL);

        // Load the pack itself with a supplied title
        DownloadMissionPack(ppackid, pszPackTitle, false);

        // If installed now, return success
        if (gppackm->IsInstalled(ppackid) == 1) {
            return true;
        }
    }
    return false;
}

DownloadBox::DownloadBox(const PackId *ppackid, const char *pszTitle,
        bool fPlayButton) : packid_(*ppackid), pszTitle_(pszTitle),
        fPlayButton_(fPlayButton), pszURL_(NULL), error_(false) {
}

DownloadBox::DownloadBox(const char *pszURL) :
        pszTitle_(pszURL), pszURL_(pszURL), fPlayButton_(true), error_(false) {
    memset(&packid_, 0, sizeof(packid_));
}

DownloadBox::~DownloadBox() {
    gppackm->Cancel();
}

bool DownloadBox::DoModal(int *pnResult, Sfx sfxShow, Sfx sfxHide) {
    // Set colors
	SetTitleColor(GetColor(kiclrBlueSideFirst));
    SetBorderColorIndex(kiclrWhite);
	SetBackgroundColorIndex(kiclrShadow2x);

    // Set title

    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMessage);
    if (pszTitle_ != NULL) {
        plbl->SetText(base::Format::ToString("Downloading %s", pszTitle_));
    } else {
        json::JsonMap *map = gppim->GetInfoMap(&packid_);
        if (map != NULL) {
            const json::JsonObject *obj = map->GetObject("t");
            if (obj != NULL && obj->type() == json::JSONTYPE_STRING) {
                plbl->SetText(base::Format::ToString("Downloading %s",
                        ((json::JsonString *)obj)->GetString()));
            }
            delete map;
        } else {
            plbl->SetText("Downloading...");
        }
    }
            
    // Start the download
    ShowStatus("Starting download...");
    if (pszURL_ != NULL) {
        gppackm->Install(pszURL_, &packid_, NULL, this);
    } else {
        gppackm->Install(&packid_, NULL, this);
    }

    // Remember the button positions
    // Center OK, change to "CANCEL", hide Play

    Control *pctl = GetControlPtr(kidcOk);
    Rect rcT;
    pctl->GetRect(&rcT);
    xOk_ = rcT.left;
    rcT.Offset((m_rc.Width() - rcT.Width()) / 2 - rcT.left, 0);
    pctl->SetRect(&rcT);
    ButtonControl *pbtn = (ButtonControl *)pctl;
    pbtn->SetText("CANCEL");

    pctl = GetControlPtr(kidcPlay);
    pctl->Show(false);

    // Show the form
    return DialogForm::DoModal(pnResult, sfxShow, sfxHide);
}

void DownloadBox::OnBegin(void *ctx, int cbLength) {
    OnProgress(ctx, 0, cbLength);
}

void DownloadBox::OnProgress(void *ctx, int cbTotal, int cbLength) {
    ShowStatus(base::Format::ToString("Downloaded %d of %d bytes",
            cbTotal, cbLength));
}

void DownloadBox::OnFinished(void *ctx) {
    // Reposition and show both buttons, change kidcOk back to OK
    
    Control *pctl = GetControlPtr(kidcOk);
    ButtonControl *pbtn = (ButtonControl *)pctl;
    pbtn->SetText("OK");
    if (fPlayButton_) {
        Rect rcT;
        pctl->GetRect(&rcT);
        rcT.Offset(xOk_ - rcT.left, 0);
        pctl->SetRect(&rcT);
        pctl = GetControlPtr(kidcPlay);
        pctl->Show(true);
    }

    // Create a message about where the missions went, so it is easy(ier)
    // to understand what is going on

    MissionList *pml = CreateMissionList(&packid_, kmltAll);
    if (pml == NULL || pml->GetCount() == 0) {
        ShowStatus("Download complete, but mission pack has no missions!");
        delete pml;
        return;
    }

    int cSingle = 0;
    int cMulti = 0;
    int cTotal = pml->GetCount();
    for (int i = 0; i < cTotal; i++) {
        MissionDescription md;
        if (pml->GetMissionDescription(i, &md)) {
            if (pml->IsMultiplayerMissionType(md.mt)) {
                cMulti++;
            } else {
                cSingle++;
            }
        }
    }
    delete pml;

    std::string single;
    if (cSingle != 0) {
        single = base::Format::ToString(
                "%d single player mission%s installed.",
                cSingle, cSingle == 1 ? "" : "s");
    }

    std::string multi;
    if (cMulti != 0) {
        multi = base::Format::ToString(
                "%d multiplayer mission%s installed.",
                cMulti, cMulti == 1 ? "" : "s");
    }

    ShowStatus(base::Format::ToString("Download Complete! %s %s",
            single.c_str(), multi.c_str()));
}

void DownloadBox::OnError(void *ctx, const char *pszError) {
    error_ = true;
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    pbtn->SetText("OK");
    ShowStatus(base::Format::ToString("Download error: %s", pszError));
}

void DownloadBox::ShowStatus(const char *pszStatus) {
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcStatus);
    plbl->SetText(pszStatus);

    // Wake up to process the paint

    base::Thread::current().Post(base::kidmNullEvent, NULL); 
}

} // namespace wi
