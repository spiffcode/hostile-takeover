#include "game/ht.h"
#include "game/httppackmanager.h"
#include "game/httppackinfomanager.h"
#include "yajl/wrapper/jsontypes.h"

namespace wi {

MissionList *CreateMissionList(const PackId *ppackid, MissionListType mlt) {
    MissionList *pml = new MissionList;
    if (pml == NULL) {
        return NULL;
    }
    if (!pml->Init(ppackid, mlt)) {
        delete pml;
        return NULL;
    }
    return pml;
}

MissionList::MissionList() {
    m_ppdbiFirst = NULL;
}

MissionList::~MissionList() {
    // Delete the mission list

    while (m_ppdbiFirst != NULL) {
        PdbItem *ppdbiT = m_ppdbiFirst;
        m_ppdbiFirst = m_ppdbiFirst->ppdbiNext;
        while (ppdbiT->plvliFirst != NULL) {
            LvlItem *plvliT = ppdbiT->plvliFirst;
            ppdbiT->plvliFirst = ppdbiT->plvliFirst->plvliNext;
            delete[] plvliT->pszFilename;
            delete plvliT;
        }
        delete[] ppdbiT->pszTitle;
        delete ppdbiT;
    }
}

bool MissionList::Init(const PackId *ppackid, MissionListType mlt) {
    if (m_ppdbiFirst != NULL) {
        Assert();
        return true;
    }
    m_mlt = mlt;

    if (ppackid != NULL) {
        // Add levels from just this pack

        AddLevelFiles(ppackid);
    } else {
        // First, enum the main game files

        PackId packidMain;
        memset(&packidMain, 0, sizeof(packidMain));
        packidMain.id = PACKID_MAIN;
        PdbItem *ppdbi = AddLevelFiles(&packidMain);
        if (ppdbi == NULL) {
            Assert();
            return false;
        }

        // Now take this off the list for now, because the sorting will be
        // forced later to be at the top

        PdbItem *ppdbiMain = ppdbi;
        m_ppdbiFirst = NULL;

        // Fix up the main sort single player missions, so S_ goes before
        // C_.

        if (m_mlt == kmltAll || m_mlt == kmltSinglePlayer) {
            ResortMainMissions(ppdbi);
        }

        // Now add addon missions
        Enum enm;
        PackId packid;
        while (gppackm->EnumPacks(&enm, &packid)) {
            AddLevelFiles(&packid);
        }

        // Now re-insert main to be first

        ppdbiMain->ppdbiNext = m_ppdbiFirst;
        m_ppdbiFirst = ppdbiMain;
    }

    return true;
}

int MissionList::GetCount() {
    int clvli = 0;
    for (PdbItem *ppdbi = m_ppdbiFirst; ppdbi != NULL;
            ppdbi = ppdbi->ppdbiNext) {
        clvli += ppdbi->clvli;
    }
    return clvli;
}

bool MissionList::GetMissionDescription(int i, MissionDescription *pmd) {
    PdbItem *ppdbi;
    LvlItem *plvli = FindLevelItem(i, &ppdbi);
    if (plvli == NULL) {
        return false;
    }
    if (!gppackm->Mount(gpakr, &ppdbi->packid)) {
        return false;
    }

    const char *pszName;
    switch (plvli->mt) {
    case kmtStory:
        pszName = "Story Missions";
        break;

    case kmtMultiplayerChallenge:
    case kmtChallenge:
        pszName = "Challenge Missions";
        break;

    case kmtDemo:
        pszName = "Demo Missions";
        break;

    case kmtMultiplayerAddOn:
    case kmtAddOn:
        pszName = ppdbi->pszTitle;
        if (strlen(pszName) == 0) {
            pszName = ExtractPackTitle(&ppdbi->packid);
            if (pszName == NULL) {
                pszName = "Add-On Mission Pack";
            }
        }
        break;

    default:
        pszName = "<untitled>";
        break;
    }
    strncpyz(pmd->szPackName, pszName, sizeof(pmd->szPackName));
    pmd->mt = plvli->mt;

    IniReader *pini = LoadIniFile(gpakr, plvli->pszFilename);
    if (pini == NULL) {
        gppackm->Unmount(gpakr, &ppdbi->packid);
        return false;
    }

    strncpyz(pmd->szLvlTitle, "<untitled>", sizeof(pmd->szLvlTitle));
    pini->GetPropertyValue("General", "Title", pmd->szLvlTitle,
            sizeof(pmd->szLvlTitle));

    if (IsMultiplayerMissionType(plvli->mt)) {
        pmd->cPlayersMin = 2;
        pini->GetPropertyValue("General", "MinPlayers", "%d",
                &pmd->cPlayersMin);
        if (pmd->cPlayersMin < 2) {
            pmd->cPlayersMin = 2;
        }
        pmd->cPlayersMax = 2;
        pini->GetPropertyValue("General", "MaxPlayers", "%d",
                &pmd->cPlayersMax);
        if (pmd->cPlayersMax > 4) {
            pmd->cPlayersMax = 4;
        }
    } else {
        pmd->cPlayersMin = 1;
        pmd->cPlayersMax = 1;
    }

    delete pini;
    gppackm->Unmount(gpakr, &ppdbi->packid);
    return true;
}

const char *MissionList::ExtractPackTitle(const PackId *ppackid) {
    const json::JsonMap *map = gppim->GetInfoMap(ppackid);
    if (map == NULL) {
        return NULL;
    }

    const json::JsonObject *obj = map->GetObject("t");
    if (obj == NULL || obj->type() != json::JSONTYPE_STRING) {
        delete map;
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
        delete map;
        return NULL;
    }

    const char *title = base::Format::ToString("%s", s->GetString());
    delete map;
    return title;
}

bool MissionList::GetMissionIdentifier(int i, MissionIdentifier *pmiid) {
    PdbItem *ppdbi;
    LvlItem *plvli = FindLevelItem(i, &ppdbi);
    if (plvli == NULL) {
        return false;
    }
    pmiid->packid = ppdbi->packid;
    strncpyz(pmiid->szLvlFilename, plvli->pszFilename,
            sizeof(pmiid->szLvlFilename));
    return true;
}

MissionList::LvlItem *MissionList::FindLevelItem(int i, PdbItem **pppdbi) {
    if (i < 0) {
        return NULL;
    }
    bool fFound = false;
    PdbItem *ppdbi = m_ppdbiFirst;
    for (; ppdbi != NULL; ppdbi = ppdbi->ppdbiNext) {
        if (i < ppdbi->clvli) {
            fFound = true;
            break;
        }
        i = i - ppdbi->clvli;
    }
    if (!fFound) {
        return NULL;
    }
    for (LvlItem *plvli = ppdbi->plvliFirst; plvli != NULL;
            plvli = plvli->plvliNext) {
        if (i == 0) {
            *pppdbi = ppdbi;
            return plvli;
        }
        i--;
    }
    return NULL;
}

MissionList::PdbItem *MissionList::AddLevelFiles(const PackId *ppackid) {
    if (!gppackm->Mount(gpakr, ppackid)) {
        return NULL;
    }

    char szFn[kcbFilename];
    PdbItem *ppdbi = NULL;
    int key = (ppackid->id == PACKID_MAIN) ? PACKENUM_FIRST : PACKENUM_LAST;
    Enum enm;
    while (gpakr.EnumFiles(&enm, key, szFn, sizeof(szFn))) {
        int cch = (int)strlen(szFn);
        if (cch < 4) {
            continue;
        }
        if (strcmp(&szFn[cch - 4], ".lvl") != 0) {
            continue;
        }
        MissionListType mlt = kmltSinglePlayer;
        if (IsMultiplayerMissionType(GetMissionType(ppackid, szFn))) {
            mlt = kmltMultiplayer;
        } else {
            mlt = kmltSinglePlayer;
        }
        if (mlt != m_mlt && m_mlt != kmltAll) {
            continue;
        }
        if (!PassesDemoFilter(szFn, ppackid->id != PACKID_MAIN)) {
            continue;
        }

        // Alloc PdbItem only if there are the desired missions
        if (ppdbi == NULL) {
            ppdbi = AddPdbItem(ppackid);
            if (ppdbi == NULL) {
                continue;
            }
        }
        AddLvlItem(ppdbi, szFn);
    }
    gppackm->Unmount(gpakr, ppackid);

    return ppdbi;
}

MissionList::PdbItem *MissionList::AddPdbItem(const PackId *ppackid) {
    PdbItem *ppdbi = new PdbItem;
    if (ppdbi == NULL) {
        return NULL;
    }
    ppdbi->packid = *ppackid;
    ppdbi->plvliFirst = NULL;
    ppdbi->ppdbiNext = NULL;
    ppdbi->clvli = 0;
    ppdbi->pszTitle = NULL;

    if (ppackid->id == PACKID_MAIN) {
        ppdbi->pszTitle = AllocString("Main Game");
    } else {
        const json::JsonMap *map = gppim->GetInfoMap(ppackid);
        if (map != NULL) {
            const json::JsonObject *obj = map->GetObject("t");
            if (obj != NULL && obj->type() == json::JSONTYPE_STRING) {
                const json::JsonString *title = (const json::JsonString *)obj;
                ppdbi->pszTitle = AllocString(title->GetString());
            }
            delete map;
        }
        if (ppdbi->pszTitle == NULL) {
            ppdbi->pszTitle = AllocString("");
        }
    }

    bool fInserted = false;
    PdbItem **pppdbiT = &m_ppdbiFirst;
    while ((*pppdbiT) != NULL) {
        PdbItem *ppdbiT = *pppdbiT;
        if (strcmp(ppdbi->pszTitle, ppdbiT->pszTitle) < 0) {
            ppdbi->ppdbiNext = ppdbiT;
            *pppdbiT = ppdbi;
            fInserted = true;
            break;
        }
        pppdbiT = &(*pppdbiT)->ppdbiNext;
    }
    if (!fInserted) {
        *pppdbiT = ppdbi;
    }
    return ppdbi;
}

MissionList::LvlItem *MissionList::AddLvlItem(PdbItem *ppdbi, char *pszLvl) {
    LvlItem *plvli = new LvlItem;
    if (plvli == NULL) {
        return NULL;
    }
    plvli->pszFilename = AllocString(pszLvl);
    if (plvli->pszFilename == NULL) {
        delete plvli;
        return NULL;
    }
    plvli->mt = GetMissionType(&ppdbi->packid, pszLvl);
    plvli->plvliNext = NULL;

    bool fInserted = false;
    LvlItem **pplvliT = &ppdbi->plvliFirst;
    while ((*pplvliT) != NULL) {
        LvlItem *plvliT = *pplvliT;
        if (strcmp(plvli->pszFilename, plvliT->pszFilename) < 0) {
            plvli->plvliNext = plvliT;
            *pplvliT = plvli;
            fInserted = true;
            break;
        }
        pplvliT = &(*pplvliT)->plvliNext;
    }
    if (!fInserted) {
        *pplvliT = plvli;
    }
    ppdbi->clvli++;
    return plvli;
}

void MissionList::ResortMainMissions(PdbItem *ppdbi) {
    // Move the c_ missions after the s_ missions.
    // Find the end of the list

    LvlItem **pplvliT = &ppdbi->plvliFirst;
    while ((*pplvliT) != NULL) {
        pplvliT = &(*pplvliT)->plvliNext;
    }

    // Move all non-s_'s to the end. Keep relative order.

    while (strncmp(ppdbi->plvliFirst->pszFilename, "s_", 2) != 0) {
        LvlItem *plvliT = ppdbi->plvliFirst;
        ppdbi->plvliFirst = plvliT->plvliNext;
        plvliT->plvliNext = NULL;
        *pplvliT = plvliT;
        pplvliT = &plvliT->plvliNext;
    }
}

bool MissionList::PassesDemoFilter(char *psz, bool fAddOn) {
    // Everything is available if not demo
    if (!gfDemo) {
        return true;
    }
    // Addon's not available if demo
    if (fAddOn) {
        return false;
    }
    // Only m_12 available as multiplayer
    if (m_mlt == kmltAll || m_mlt == kmltMultiplayer) {
        return strcmp(psz, "m_12.lvl") == 0;
    }
    // Only s_00-03 available single player
    return strcmp(psz, "s_00.lvl") == 0 || strcmp(psz, "s_01.lvl") == 0 ||
            strcmp(psz, "s_02.lvl") == 0 || strcmp(psz, "s_03.lvl") == 0;
}

MissionType MissionList::GetMissionType(const PackId *ppackid,
        const char *pszLvl) {

    if (ppackid->id == PACKID_MAIN) {
        if (pszLvl[1] == '_') {
            switch (pszLvl[0]) {
            case 's':
                return kmtStory;

            case 'c':
                return kmtChallenge;

            case 'd':
                // HACK: D3 is the only demo mission in the database
                // let's considder it a challenge map
                return kmtChallenge; // return kmtDemo;

            case 'm':
                return kmtMultiplayerChallenge;
            }
        }
        return kmtUnknown;
    }

    if (pszLvl[0] == 'm' && pszLvl[1] == '_') {
        return kmtMultiplayerAddOn;
    }

    return kmtAddOn;
}

} // namespace wi
