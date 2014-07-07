#ifndef __MISSIONLIST_H__
#define __MISSIONLIST_H__

#include "game/ht.h"

namespace wi {

enum MissionType { kmtStory, kmtChallenge, kmtAddOn, kmtDemo,
        kmtMultiplayerChallenge, kmtMultiplayerAddOn, kmtUnknown };

struct MissionDescription { // md
    char szPackName[kcbLevelTitle];
    char szLvlTitle[kcbLevelTitle];
    int cPlayersMin;
    int cPlayersMax;
    MissionType mt;
};

struct MissionIdentifier { // miid
    PackId packid;
    char szLvlFilename[kcbFilename];
};

enum MissionListType { kmltAll, kmltSinglePlayer, kmltMultiplayer };

class MissionList
{
public:
    MissionList();
    ~MissionList();

    bool Init(const PackId *ppackid, MissionListType type);
    int GetCount();
    bool GetMissionDescription(int i, MissionDescription *pmd);
    bool GetMissionIdentifier(int i, MissionIdentifier *pmiid);
    bool IsMultiplayerMissionType(MissionType mt) {
        return mt == kmtMultiplayerChallenge || mt == kmtMultiplayerAddOn;
    }

private:
    struct LvlItem { // lvli
        const char *pszFilename;
        MissionType mt;
        LvlItem *plvliNext;
    };

    struct PdbItem { // pdbi
        PackId packid;
        const char *pszTitle;
        int clvli;
        LvlItem *plvliFirst;
        PdbItem *ppdbiNext;
    };

    LvlItem *FindLevelItem(int i, PdbItem **pppdbi);
    PdbItem *AddLevelFiles(const PackId *ppackid);
    PdbItem *AddPdbItem(const PackId *ppackid);
    LvlItem *AddLvlItem(PdbItem *ppdbi, char *pszLvl);
    void ResortMainMissions(PdbItem *ppdbi);
    bool PassesDemoFilter(char *psz, bool fAddOn);
    MissionType GetMissionType(const PackId *ppackid, const char *pszLvl);
    const char *ExtractPackTitle(const PackId *ppackid);

    PdbItem *m_ppdbiFirst;
    MissionListType m_mlt;
};

extern MissionList *CreateMissionList(const PackId *ppackid,
        MissionListType mlt);

} // namespace wi

#endif // __MISSIONLIST_H__
