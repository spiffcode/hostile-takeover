#include "game/ht.h"
#include "base/format.h"
#include "yajl/wrapper/jsontypes.h"
#include "game/httppackinfomanager.h"
#include "game/completemanager.h"

namespace wi {

#define kcUnlockAhead 1
#define kfItemLocked 0x8000

// Should be moved into PackInfoManager, but in a better form,
// for getting specific properties without needing to know the key.

const char *GetString(const json::JsonMap *map, const char *key) {
    const json::JsonObject *obj = map->GetObject(key);
    if (obj == NULL || obj->type() != json::JSONTYPE_STRING) {
        return NULL;
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
        return NULL;
    }
    return s->GetString();
}

SelectMissionForm::SelectMissionForm(MissionList *pml,
        const MissionIdentifier *pmiidFind) {
    m_pml = pml;
    m_pmiidFind = pmiidFind;
    m_mt = kmtStory;
    m_cMagic = 0;
    m_fMagicUnlock = false;
#ifdef DEBUG
    m_fMagicUnlock = true;
#endif
    memset(m_aplstc, 0, sizeof(m_aplstc));
}

bool SelectMissionForm::Init(FormMgr *pfrmm, IniReader *pini, word idf) {
	if (!ShellForm::Init(pfrmm, pini, idf))
		return false;

    // Format the lists. 3 lists are used as a simple cache.

    int aidcList[] = { kidcStoryList, kidcChallengeList, kidcAddOnList };
    for (int i = 0; i < ARRAYSIZE(aidcList); i++) {
        ListControl *plstc = (ListControl *)GetControlPtr(aidcList[i]);
        m_aplstc[i] = plstc;
        Rect rc;
        plstc->GetRect(&rc);
        Font *pfnt = gapfnt[kifntShadow];
        int cxComplete = pfnt->GetTextExtent("Complete");
        int xTitle = rc.Width() / 2 - cxComplete * 3 / 2;
        int xRightComplete = rc.Width() - 10;
        int xLeftComplete = xRightComplete - cxComplete - cxComplete / 2;
        plstc->SetTabStops(xTitle, xLeftComplete, xRightComplete);
        plstc->SetTabFlags(kfLstTabEllipsis, kfLstTabCenter, 0);
        plstc->SetFlags(plstc->GetFlags() | kfLstcKeepInteriorPositioning);
    }

    // If asked to find a certain mission, find it first to see what
    // type it is, and switch the radio button bar to that type.

    int iPack = -1;
    int iMission = -1;
	int cMissions = m_pml->GetCount();
    if (m_pmiidFind != NULL) {
        for (int i = 0; i < cMissions; i++) {
            MissionIdentifier miid;
            m_pml->GetMissionIdentifier(i, &miid);
            if (memcmp(&miid.packid, &m_pmiidFind->packid,
                    sizeof(miid.packid)) == 0) {
                if (iPack == -1) {
                    iPack = i;
                }
                if (strcmp(miid.szLvlFilename,
                        m_pmiidFind->szLvlFilename) == 0) {
                    iMission = i;
                    break;
                }
            }
        }
        if (iMission == -1) {
            iMission = iPack;
        }
    }
    int iMissionSelect = iMission;

    // Init the lists

    MissionType mt = InitLists(iMissionSelect);
    SwitchToMissionType(mt);

    // Hide this label - only show it if there are no addon packs
    GetControlPtr(kidcAddOnMessage)->Show(false);

	return true;
}

int SelectMissionForm::IndexFromMissionType(MissionType mt) {
    switch (mt) {
    case kmtStory:
        return 0;
    case kmtChallenge:
        return 1;
    case kmtAddOn:
        return 2;
    default:
        return -1;
    }
}

MissionType SelectMissionForm::MissionTypeFromIndex(int i) {
    switch (i) {
    case 0:
        return kmtStory;
    case 1:
        return kmtChallenge;
    case 2:
        return kmtAddOn;
    default:
        return kmtUnknown;
    }
}

void SelectMissionForm::SwitchToMissionType(MissionType mt) {
    m_mt = mt;
    RadioButtonBarControl *prbbc =
            (RadioButtonBarControl *)GetControlPtr(kidcCategories);
    prbbc->SetSelectionIndex(IndexFromMissionType(mt));
    for (int i = 0; i < ARRAYSIZE(m_aplstc); i++) {
        bool fShow = false;
        if (i == IndexFromMissionType(mt)) {
            fShow = true;
        }
        m_aplstc[i]->Show(fShow);
    }
    UpdateDescription();
}

MissionType SelectMissionForm::InitLists(int iMissionSelect) {
    // Fill in the lists, and along the way keep track of useful selection
    // indexes.

    int ailiFirstIncomplete[kmtUnknown + 1];
    memset(ailiFirstIncomplete, 0xff, sizeof(ailiFirstIncomplete));
    int iliLastCompleteStory = -1;
    int iliSelectSpecial = -1;
    MissionType mtSelectSpecial = kmtUnknown;

    int cMissions = m_pml->GetCount();
	for (int i = 0; i < cMissions; i++) {
        MissionDescription md;
        if (!m_pml->GetMissionDescription(i, &md)) {
            continue;
        }
        if (md.mt != kmtStory && md.mt != kmtChallenge && md.mt != kmtAddOn) {
            continue;
        }

        // The first locked mission is kcUnlockAhead missions ahead of the
        // last complete story mission.

        ListControl *plstc = m_aplstc[IndexFromMissionType(md.mt)];
        bool fLocked = false;
        if (md.mt == kmtStory) {
            int iliMissionLocked = iliLastCompleteStory + 1 + kcUnlockAhead;
            int iliNext = plstc->GetCount();
            if (iliNext >= iliMissionLocked) {
                fLocked = true;
            }
        }

        // Get the status - LOCKED, Complete, or nothing

        MissionIdentifier miid;
        m_pml->GetMissionIdentifier(i, &miid);
        dword dw = (dword)i;
        const char *pszStatus = "";
        if (fLocked) {
            pszStatus = "LOCKED";
            dw |= kfItemLocked;
        } else {
            if (gpcptm->IsComplete(&miid)) {
                pszStatus = "Complete";
            }
        }

        // Add the item

        plstc->Add(base::Format::ToString("%s\t%s", md.szLvlTitle,
                pszStatus), (void *)dw);

        // Track the first incomplete for each mission type.

        if (ailiFirstIncomplete[md.mt] == -1) {
            if (!gpcptm->IsComplete(&miid)) {
                ailiFirstIncomplete[md.mt] = plstc->GetCount() - 1;
            }
        }

        // Track the last complete for story missions, for mission unlocking

        if (md.mt == kmtStory) {
            if (gpcptm->IsComplete(&miid)) {
                iliLastCompleteStory = plstc->GetCount() - 1;
            }
        }

        // This is passed in when the form needs to select a certain
        // mission when it first shows.

        if (i == iMissionSelect) {
            iliSelectSpecial = plstc->GetCount() - 1;
            mtSelectSpecial = md.mt;
        }
    }

    // The initially selected missions are the first incomplete missions
    // for each mission type.

    MissionType mtSelect = kmtStory;
    for (int i = 0; i < ARRAYSIZE(m_aplstc); i++) {
        ListControl *plstc = m_aplstc[i];

        // Is this the list that is awarded the special selection?

        if (i == IndexFromMissionType(mtSelectSpecial)) {
            plstc->Select(iliSelectSpecial, true, true);
            mtSelect = mtSelectSpecial;
            continue;
        }

        int iliSelect = ailiFirstIncomplete[MissionTypeFromIndex(i)];
        if (iliSelect < 0) {
            iliSelect = 0;
        }
        plstc->Select(iliSelect, true, true);
    }
    return mtSelect;
}

bool SelectMissionForm::OnPenEvent(Event *pevt) {
    if (m_mt != kmtStory || pevt->eType != penUpEvent) {
        return ShellForm::OnPenEvent(pevt);
    }

    Control *pctlCaptureBefore = GetControlCapture();
    bool f = ShellForm::OnPenEvent(pevt);
    Control *pctlCaptureAfter = GetControlCapture();

#define kcMagic 5

    // Tap down on back, move your finger off so it unhighlights, then
    // let up. Do this  kcMagic times and get a play button.
    if (pctlCaptureBefore != NULL && pctlCaptureAfter == NULL) {
        if (pctlCaptureBefore->GetId() == kidcCancel) {
            m_cMagic++;
            if (m_cMagic >= kcMagic) {
                m_cMagic = 0;
                m_fMagicUnlock = true;
#ifdef BETA_TIMEOUT
                GetControlPtr(kidcOk)->Show(true);
#endif
            }
        }
    }
    return f;
}

void SelectMissionForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcOk:
    case kidcCancel:
        EndForm(idc);
        break;

    case kidcSetupGame:
        {
            ShellForm *pfrm = (ShellForm *)gpmfrmm->LoadForm(gpiniForms,
                    kidfInGameOptions, new InGameOptionsForm());
            if (pfrm != NULL) {
                pfrm->DoModal();
                delete pfrm;
            }
        }
        break;
    }
}

void SelectMissionForm::OnControlNotify(word idc, int nNotify) {
    if (idc == kidcCategories) {
        RadioButtonBarControl *prbbc =
                (RadioButtonBarControl *)GetControlPtr(kidcCategories);
        int iButtonSelected = prbbc->GetSelectionIndex();
        if (iButtonSelected < 0) {
            iButtonSelected = 0;
        }
        MissionType mtNew = MissionTypeFromIndex(iButtonSelected);
        if (mtNew == m_mt) {
            return;
        }
        SwitchToMissionType(mtNew);

        // If in Add-On category, and there is nothing there, show this
        // label, otherwise hide it

        bool fShowLabel = false;
        ListControl *plstc = m_aplstc[IndexFromMissionType(kmtAddOn)];
        if (m_mt == kmtAddOn) {
            if (plstc->GetCount() == 0) {
                fShowLabel = true;
            }
        }

        LabelControl *plbl = (LabelControl *)GetControlPtr(kidcAddOnMessage);
        if (fShowLabel) {
            plbl->Show(true);
            if (m_mt == kmtAddOn) {
                plstc->Show(false);
            }
        } else {
            plbl->Show(false);
            if (m_mt == kmtAddOn) {
                plstc->Show(true);
            }
        }
    }

    if (idc == kidcStoryList || idc == kidcChallengeList ||
            idc == kidcAddOnList) {
        // Update the mission pack description
        UpdateDescription();
    }

    // Handle button hiding

    bool fShow = true;
    ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
    if (plstc->GetSelectedItemIndex() == -1) {
        fShow = false;
    }
    if (m_mt == kmtStory) {
        if (IsSelectedMissionLocked(plstc)) {
            fShow = false;
        }
    }
    if (m_fMagicUnlock) {
        fShow = true;
    }
    GetControlPtr(kidcOk)->Show(fShow);
}

int SelectMissionForm::GetSelectedMissionIndex(ListControl *plstc) {
    dword dw = (dword)plstc->GetSelectedItemData();
    return (int)(dw & ~kfItemLocked);
}

bool SelectMissionForm::IsSelectedMissionLocked(ListControl *plstc) {
    dword dw = (dword)plstc->GetSelectedItemData();
    return (dw & kfItemLocked) != 0;
}

void SelectMissionForm::UpdateDescription() {
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcMissionPackInfo);
    ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
    if (plstc->GetSelectedItemIndex() == -1) {
        plbl->SetText("");
        return;
    }
    int i = GetSelectedMissionIndex(plstc);
    MissionIdentifier miid;
    if (!m_pml->GetMissionIdentifier(i, &miid)) {
        plbl->SetText("");
        return;
    }
    json::JsonMap *map = gppim->GetInfoMap(&miid.packid);
    if (map == NULL) {
        MissionDescription md;
        if (!m_pml->GetMissionDescription(i, &md)) {
            plbl->SetText("");
            return;
        }
        const char *s;
        if (miid.packid.id == PACKID_MAIN) {
            s = base::Format::ToString("%s by Spiffcode, Inc.", md.szPackName);
        } else {
            s = md.szPackName;
        }
        plbl->SetText(s);
        return;
    }
    const char *szAuthor = GetString(map, "a");
    const char *szTitle = GetString(map, "t");
    const char *s = base::Format::ToString("%s by %s", szTitle, szAuthor);
    plbl->SetText(s);
    delete map;
}

bool SelectMissionForm::GetSelectedMission(MissionIdentifier *pmiid) {
    ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
    int i = GetSelectedMissionIndex(plstc);
    return m_pml->GetMissionIdentifier(i, pmiid);
}

} // namespace wi
