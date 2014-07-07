#include "game/ht.h"
#include "game/creategameform.h"
#include "game/completemanager.h"
#include "game/httppackmanager.h"
#include "game/httppackinfomanager.h"
#include "yajl/wrapper/jsontypes.h"

namespace wi {
    
const char *GetString(const json::JsonMap *map, const char *key);

CreateGameForm::CreateGameForm(LoginHandler& handler, const PackId *ppackidFind,
            Chatter& chatter, GameParams *prams) : handler_(handler),
            chatter_(chatter), m_prams(prams), m_pml(NULL),
            m_mt(kmtMultiplayerChallenge) {
    memset(&m_miidFind, 0, sizeof(m_miidFind));
    if (ppackidFind != NULL) {
        m_miidFind.packid = *ppackidFind;
    }
    chatter_.SetChatTitle("Room Chat");
    chatter_.SignalOnBlink.connect(this, &CreateGameForm::OnChatBlink);
}

CreateGameForm::~CreateGameForm() {
    delete m_pml;
    chatter_.SignalOnBlink.disconnect(this);
    chatter_.HideChat();
}

bool CreateGameForm::DoForm(LoginHandler& handler, const PackId *ppackidFind,
        Chatter& chatter, GameParams *params) {
    CreateGameForm *pfrm = (CreateGameForm *)gpmfrmm->LoadForm(gpiniForms,
            kidfCreateGameWide, new CreateGameForm(handler, ppackidFind,
            chatter, params));
    if (pfrm == NULL) {
        return false;
    }
    int idc = kidcCancel;
    pfrm->DoModal(&idc);
    delete pfrm;
    return idc == kidcOk;
}

bool CreateGameForm::DoModal(int *pnResult) {
    // Format the lists.
    
    int aidcList[] = { kidcChallengeList, kidcAddOnList };
    for (int i = 0; i < ARRAYSIZE(aidcList); i++) {
        ListControl *plstc = (ListControl *)GetControlPtr(aidcList[i]);
        m_aplstc[i] = plstc;
        Rect rc;
        plstc->GetRect(&rc);
        Font *pfnt = gapfnt[kifntShadow];
        int cxComplete = pfnt->GetTextExtent("Complete");
        int xTitle = rc.Width() / 2 - cxComplete * 3 / 2;
        plstc->SetTabStops(xTitle);
        plstc->SetTabFlags(kfLstTabEllipsis);
        plstc->SetFlags(plstc->GetFlags() | kfLstcKeepInteriorPositioning);
        plstc->Clear();
    }
    
    if (m_pml == NULL) {
        m_pml = CreateMissionList(NULL, kmltMultiplayer);
    }
    if (m_pml == NULL) {
        return false;
    }
    
    // If asked to find a certain mission, find it first to
    // see what type it is, and switch the radio button bar to
    // that type.
    
    int iPack = -1;
    int iMission = -1;
    int cLevels = m_pml->GetCount();
    for (int nLevel = 0; nLevel < cLevels; nLevel++) {
        MissionIdentifier miid;
        m_pml->GetMissionIdentifier(nLevel, &miid);
        if (memcmp(&miid.packid, &m_miidFind.packid,
                   sizeof(miid.packid)) == 0) {
            if (iPack == -1) {
                iPack = nLevel;
            }
            if (strcmp(miid.szLvlFilename,
                       m_miidFind.szLvlFilename) == 0) {
                iMission = nLevel;
                break;
            }
        }
    }
    if (iMission == -1) {
        iMission = iPack;
    }
    int iMissionSelect = iMission;
    
    // Init the lists
    
    MissionType mt = InitLists(iMissionSelect);
    SwitchToMissionType(mt);
    
    // Game Speed
    
    m_tGameSpeed = gtGameSpeed;
    if (m_tGameSpeed < 4)
        m_tGameSpeed = 4;
    SliderControl *psldr = (SliderControl *)GetControlPtr(kidcGameSpeed);
    psldr->SetRange(2, ARRAYSIZE(gatGameSpeeds) - 1 - 3);	// bring the extremes in a bit for multiplayer
    psldr->SetValue(8);
    for (int i = 0; i < ARRAYSIZE(gatGameSpeeds); i++) {
        if (gatGameSpeeds[i] == m_tGameSpeed) {
            psldr->SetValue(i);
            break;
        }
    }
    
    // Hide this label. If we're finding an Add-On mission, then
    // there are add-on missions and this label isn't meant to be visible.
    
    GetControlPtr(kidcAddOnMessage)->Show(false);
    
    UpdateLabels();
    
    gptra->SetCallback(this);
    bool fSuccess = ShellForm::DoModal(pnResult);
    gptra->SetCallback(NULL);
    delete m_pml;
    m_pml = NULL;
    return fSuccess;
}

void CreateGameForm::OnChatBlink(bool on) {
    GetControlPtr(kidcChat)->Show(on);
}

void CreateGameForm::OnConnectionClose() {
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

void CreateGameForm::OnShowMessage(const char *message) {
    message_ = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool CreateGameForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType == connectionCloseEvent) {
        chatter_.HideChat();
        HtMessageBox(kfMbWhiteBorder, "Comm Problem", "The server has closed your connection.");
        EndForm(kidcCancel);
        return true;
    }

    if (pevt->eType == showMessageEvent) {
        chatter_.HideChat();
        HtMessageBox(kfMbWhiteBorder, "Server Message", message_.c_str());
        message_ = "";
        return true;
    }
    return false;
}

int CreateGameForm::IndexFromMissionType(MissionType mt) {
    switch (mt) {
    case kmtMultiplayerChallenge:
        return 0;
    case kmtMultiplayerAddOn:
        return 1;
    default:
        return -1;
    }
}

MissionType CreateGameForm::MissionTypeFromIndex(int i) {
    switch (i) {
    case 0:
        return kmtMultiplayerChallenge;
    case 1:
        return kmtMultiplayerAddOn;
    default:
        return kmtUnknown;
    }
}

void CreateGameForm::SwitchToMissionType(MissionType mt) {
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
    UpdateLabels();
}

MissionType CreateGameForm::InitLists(int iMissionSelect) {
    // Fill in the lists, and along the way keep track of useful selection
    // indexes.
    
    int ailiFirstIncomplete[kmtUnknown + 1];
    memset(ailiFirstIncomplete, 0xff, sizeof(ailiFirstIncomplete));
    int iliSelectSpecial = -1;
    MissionType mtSelectSpecial = kmtUnknown;
    
    int cMissions = m_pml->GetCount();
    for (int i = 0; i < cMissions; i++) {
        MissionDescription md;
        if (!m_pml->GetMissionDescription(i, &md)) {
            continue;
        }
        if (md.mt != kmtMultiplayerChallenge && 
            md.mt != kmtMultiplayerAddOn) {
            continue;
        }
        
        // Add the item.
        
        const char *pszT;
        if (md.cPlayersMin == md.cPlayersMax) {
            pszT = base::Format::ToString("%s (%d players)",
                                          md.szLvlTitle, md.cPlayersMin);
        } else {
            pszT = base::Format::ToString("%s (%d-%d players)",
                                          md.szLvlTitle, md.cPlayersMin, md.cPlayersMax);
        }
        ListControl *plstc = m_aplstc[IndexFromMissionType(md.mt)];
        plstc->Add(pszT, (void *)i);
        
        // Track the first incomplete, for later selection
        
        if (ailiFirstIncomplete[md.mt] == -1) {
            MissionIdentifier miid;
            m_pml->GetMissionIdentifier(i, &miid);
            if (!gpcptm->IsComplete(&miid)) {
                ailiFirstIncomplete[md.mt] = plstc->GetCount() - 1;
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
    
    MissionType mtSelect = kmtMultiplayerChallenge;
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

void CreateGameForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcPasswordPanel:
        DoInputPanelForm(this, kidcPasswordLabel, kidcPassword);
        break;
        
    case kidcGameSpeed:
        {
            SliderControl *psldr = (SliderControl *)GetControlPtr(
                    kidcGameSpeed);
            m_tGameSpeed = gatGameSpeeds[psldr->GetValue()];
            UpdateLabels();
        }
        break;
       
    case kidcChat:
        chatter_.ShowChat();
        break;
 
    case kidcCancel:
        EndForm(idc);
        break;
        
    case kidcOk:
        // Fill in GameParams
        
        ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
        int nLevel = (int)plstc->GetSelectedItemData();
        
        MissionIdentifier miid;
        if (!m_pml->GetMissionIdentifier(nLevel, &miid)) {
            HtMessageBox(kfMbWhiteBorder, "Error!",
                         "First you must select a map to play on.");
            return;
        }
        
        // Mount the pdb
        if (!gppackm->Mount(gpakr, &miid.packid)) {
            HtMessageBox(kfMbWhiteBorder, "Error!",
                         "Cannot load mission pack.");
            return;
        }
        
        // Remember for later selecting from the list
        
        m_miidFind = miid;
        
        // Read the level info
        
        Level *plvl = new Level();
        if (plvl != NULL) {
            if (plvl->LoadLevelInfo(miid.szLvlFilename)) {
                m_prams->packid = miid.packid;
                m_prams->dwVersionSimulation = knVersionSimulation;
                m_prams->tGameSpeed = m_tGameSpeed;
                strncpyz(m_prams->szLvlFilename, miid.szLvlFilename,
                         sizeof(m_prams->szLvlFilename));
            }
            delete plvl;
        } else {
            HtMessageBox(kfMbWhiteBorder, "Error", "Out of memory!");
            idc = kidcCancel;
        }
        
        gppackm->Unmount(gpakr, &miid.packid);
        
        // Keep this game speed around for next time
        
        gtGameSpeed = m_tGameSpeed;
        ggame.SavePreferences();
        
        EndForm(idc);
        break;
    }
}

void CreateGameForm::OnControlNotify(word idc, int nNotify) {
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
        ListControl *plstc =
        m_aplstc[IndexFromMissionType(kmtMultiplayerAddOn)];
        if (m_mt == kmtMultiplayerAddOn) {
            if (plstc->GetCount() == 0) {
                fShowLabel = true;
            }
        }
        
        LabelControl *plbl =
        (LabelControl *)GetControlPtr(kidcAddOnMessage);
        if (fShowLabel) {
            plbl->Show(true);
            if (m_mt == kmtMultiplayerAddOn) {
                plstc->Show(false);
            }
        } else {
            plbl->Show(false);
            if (m_mt == kmtMultiplayerAddOn) {
                plstc->Show(true);
            }
        }
    }
    
    if (idc == kidcChallengeList || idc == kidcAddOnList) {
        UpdateLabels();
    }
    
    // Handle button hiding
    
    bool fShow = true;
    ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
    if (plstc->GetSelectedItemIndex() == -1) {
        fShow = false;
    }
    GetControlPtr(kidcOk)->Show(fShow);
}

void CreateGameForm::UpdateLabels() {
    // Update Game Speed label
    
    char szT[80];
    GetSpeedMultiplierString(szT, m_tGameSpeed);
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcGameSpeedLabel);
    plbl->SetText(szT);
    
    // Update Mission Pack Info label
    
    plbl = (LabelControl *)GetControlPtr(kidcMissionPackInfo);
    ListControl *plstc = m_aplstc[IndexFromMissionType(m_mt)];
    if (plstc->GetSelectedItemIndex() == -1) {
        plbl->SetText("");
        return;
    }
    int i = (int)plstc->GetSelectedItemData();
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
            s = base::Format::ToString("%s by Spiffcode, Inc.",
                                       md.szPackName);
        } else {
            s = md.szPackName;
        }
        plbl->SetText((char *)s);
        return;
    }
    const char *szAuthor = GetString(map, "a");
    const char *szTitle = GetString(map, "t");
    const char *s = base::Format::ToString("%s by %s", szTitle, szAuthor);
    plbl->SetText((char *)s);
    delete map;
}

} // namespace wi
