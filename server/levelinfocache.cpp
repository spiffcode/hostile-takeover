#include "server/levelinfocache.h"
#include "server/ncpackfile.h"
#include "mpshared/indexloader.h"
#include "base/log.h"

namespace wi {

void LevelInfoCache::SubmitIndex(const std::string& indexfile) {
    IndexLoader index;
    if (index.InitFromFile(indexfile.c_str())) {
        int c = index.GetCount();
        for (int i = 0; i < c; i++) {
            const IndexEntry *entry = index.GetEntry(i);
            SubmitHelper(entry->packid);
        }
    }
}

bool LevelInfoCache::SubmitHelper(const PackId& packid) {
    bool success = false;
    NoCachePackFileReader pakr;
    if (packm_.Mount(pakr, &packid)) {
        success = Submit(pakr, packid);
        packm_.Unmount(pakr, &packid);
    }
    return success;
}

bool LevelInfoCache::Submit(PackFileReader& pakr, const PackId& packid) {
    // Create new infomap before removing old (if it exists), in case invalid
    LevelInfoMap infomap;
    CreateLevelInfoMap(pakr, packid, &infomap);
    if (infomap.size() == 0) {
        return false;
    }
    // Remove the old pack, if present, then add this pack
    PackMap::iterator it = map_.find(packid.id);
    if (it != map_.end()) {
        map_.erase(it);
    }
    map_.insert(PackMap::value_type(packid.id,
            std::make_pair(packid, infomap)));
    RLOG() << "Added: " << PackManager::GetPackFilename(&packid);

    return true;
}

void LevelInfoCache::CreateLevelInfoMap(PackFileReader& pakr,
        const PackId& packid, LevelInfoMap *infomap) {
    Enum enm;
    char szFile[256];
    while (pakr.EnumFiles(&enm, PACKENUM_LAST, szFile, sizeof(szFile))) {
        if (strlen(szFile) <= 4) {
            continue;
        }
        if (strcmp(&szFile[strlen(szFile) - 4], ".lvl") != 0) {
            continue;
        }
        LevelInfo info;
        if (!info.Load(pakr, szFile)) {
            continue;
        }
        infomap->insert(LevelInfoMap::value_type(std::string(szFile), info));
    }
}

int LevelInfoCache::FindInfo(const PackId& packid, const std::string& level,
        LevelInfo *pinfo, PackId *ppackidUpgrade) {
    if (packid.id != PACKID_MAIN) {
        switch (packm_.IsInstalled(&packid, ppackidUpgrade)) {
        case 0:
            // Don't know about this pack at all
            return 0;

        case 1:
            // Know about this pack, and have this version
            break;

        case 2:
            // Know about this pack, but client has wrong version
            return 2;
        }
    }

    // Lazy load the LevelInfo if necessary
    PackMap::iterator it0 = map_.find(packid.id);
    if (it0 == map_.end()) {
        if (!SubmitHelper(packid)) {
            return 0;
        }
        it0 = map_.find(packid.id);
        if (it0 == map_.end()) {
            return 0;
        }
    }

    // Same packid? If not, re-submit
    if (memcmp(&it0->second.first, &packid, sizeof(packid)) != 0) {
        if (!SubmitHelper(packid)) {
            return 0;
        }
        it0 = map_.find(packid.id);
        if (it0 == map_.end()) {
            return 0;
        }
    }

    LevelInfoMap::iterator it1 = it0->second.second.find(level);
    if (it1 == it0->second.second.end()) {
        return 0;
    }

    if (pinfo != NULL) {
        *pinfo = it1->second;
    }
    return 1;
}

void LevelInfoCache::RemoveInfo(const PackId& packid) {
    PackMap::iterator it = map_.find(packid.id);
    if (it != map_.end()) {
        if (memcmp(&it->second.first, &packid, sizeof(packid)) == 0) {
            map_.erase(it);
        }
    }
}

} // namespace wi
