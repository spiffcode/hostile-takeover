#include "server/levelinfo.h"
#include "mpshared/ini.h"
#include "mpshared/misc.h"
#include "inc/rip.h"
#include "base/log.h"

namespace wi {

bool LevelInfo::Load(PackFileReader& pak, const char *file) {
    IniReader *pini = LoadIniFile(pak, file);

	// Get level version
	if (pini->GetPropertyValue("General", "Version", "%d", &version_) == 0) {
		version_ = 0;
    }

	// Can't run if the level version is newer than what the game supports
	if (version_ > knVersionLevelSupported) {
        delete pini;
        return false;
    }

    // Load General level data
    if (!pini->GetPropertyValue("General", "Title", title_, sizeof(title_))) {
        strcpy(title_, "<untitled>");
    }
    if (!pini->GetPropertyValue("General", "MinPlayers", "%d", &minplayers_)) {
        delete pini;
        return false;
    }
    if (!pini->GetPropertyValue("General", "MaxPlayers", "%d", &maxplayers_)) {
        delete pini;
        return false;
    }

    // Read in the intelligence for each side
    for (Side side = ksideNeutral; side < kcSides; side++) {
        char name[16];
        if (side == ksideNeutral) {
            strcpy(name, "sideNeutral");
        } else {
            strcpy(name, "side0");
            name[4] = '0' + side;
        }
        intelligences_[side] = knIntelligenceComputer;
        pini->GetPropertyValue(name, "Intelligence", "%d",
                &intelligences_[side]);
    }
    delete pini;

    // If there is no human side, assume it is side 1
    bool fHumanFound = false;
    for (Side side = ksideNeutral; side < kcSides; side++) {
        if (intelligences_[side] == knIntelligenceHuman) {
            fHumanFound = true;
        }
    }
    if (!fHumanFound) {
        intelligences_[kside1] = knIntelligenceHuman;
	}

    strncpyz(filename_, file, sizeof(filename_));
    return true;
}

} // namespace wi
