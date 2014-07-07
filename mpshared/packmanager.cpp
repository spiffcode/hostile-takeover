#include "mpshared/packmanager.h"
#include "mpshared/indexloader.h"
#include "mpshared/mpht.h"
#include "mpshared/misc.h"
#include "inc/rip.h"
#include "base/md5.h"
#include "base/format.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

namespace wi {

PackManager::PackManager(const char *cachedir) {
    cachedir_ = cachedir;
}

PackManager::~PackManager() {
}

bool PackManager::EnumPacks(Enum *penm, PackId *ppackid) {
    if (penm->m_wUser == (word)kEnmFirst) {
        penm->m_wUser = 0;
        penm->m_pvNext = (void *)new PackMap::iterator(map_.begin());
    }

    PackMap::iterator *pit = (PackMap::iterator *)penm->m_pvNext;
    if (*pit == map_.end()) {
        delete pit;
        return false;
    }
    *ppackid = (*pit)->second;
    (*pit)++;
    return true;
}

int PackManager::IsInstalled(const PackId *ppackid, PackId *ppackidUpgrade) {
    // This is temporary. Ideally, the "main" packs use the same
    // versioning system.

    if (ppackid->id == PACKID_MAIN) {
        // Exact pack found
        return 1;
    }

    // This needs to be fast, so a map is used.
    PackMap::const_iterator it = map_.find(ppackid->id);
    if (it == map_.end()) {
        // Not found
        return 0;
    }
    if (memcmp(ppackid, &it->second, sizeof(*ppackid)) == 0) {
        // Exact pack found
        return 1;
    }

    // Pack found, but needs upgrade (hash mismatch)
    if (ppackidUpgrade != NULL) {
        *ppackidUpgrade = it->second;
    }
    return 2;
}

bool PackManager::Remove(const PackId *ppackid, bool fCheckHash) {
    int result = IsInstalled(ppackid);
    if (result == 0) {
        return false;
    }
    if (fCheckHash && result == 2) {
        return false;
    }

    PackMap::iterator it = map_.find(ppackid->id);
    if (it == map_.end()) {
        // It should always be found at this point
        Assert();
        return false;
    }

    // Remove the file.
    if (unlink(GetPackFilepath(&it->second)) < 0) {
        // This shouldn't fail
        Assert();
        return false;
    }

    map_.erase(it);
    return true;
}

bool PackManager::Mount(PackFileReader& pakr, const PackId *ppackid) {
    if (ppackid->id == PACKID_MAIN) {
        return true;
    }
    return pakr.Push(NULL, GetPackFilepath(ppackid));
}

bool PackManager::Unmount(PackFileReader& pakr, const PackId *ppackid) {
    if (ppackid->id == PACKID_MAIN) {
        return true;
    }
    return pakr.Pop();
}

void PackManager::InitFromInstalled() {
    map_.clear();
    DIR *pdir = opendir(cachedir_.c_str());
    dirent *pdent;
    while ((pdent = readdir(pdir)) != NULL) {
        PackId packid;
        if (!ParseFilename(pdent->d_name, &packid)) {
            continue;
        }
        map_.insert(PackMap::value_type(packid.id, packid));
    }
    closedir(pdir);
}

const char *PackManager::GetPackFilename(const PackId *ppackid) {
    char szHash[33];
    strncpyz(szHash, base::Format::ToHex(ppackid->hash, 16), sizeof(szHash));
    return base::Format::ToString("%08x-%s", ppackid->id, szHash);
}

const char *PackManager::GetPackFilepath(const PackId *ppackid) {
    char szFilename[8 + 1 + 32 + 1];
    strncpyz(szFilename, GetPackFilename(ppackid), sizeof(szFilename));
    return base::Format::ToString("%s/%s", cachedir_.c_str(), szFilename);
}

bool PackManager::ParseFilename(const char *filename, PackId *ppackid) {
    // id      -hash
    // iiiiiiii-hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
    if (strlen(filename) != 8 + 1 + 32) {
        return false;
    }
    char szId[9];
    strncpyz(szId, filename, 9);
    if (!base::Format::ToDword(szId, 16, &ppackid->id)) {
        return false;
    }
    return base::Format::FromHex(filename + 8 + 1, ppackid->hash, 16);
}

void PackManager::WatchIndex(const char *indexfile, int watch_interval_ticks) {
    // Load on a separate thread, so the game thread doesn't have to do this
    indexfile_ = indexfile;

    // Remember the last modification time of the file
    struct stat s;
    memset(&s, 0, sizeof(s));
    stat(indexfile_.c_str(), &s);
    mtime_ = s.st_mtime;
   
    // Load the index a first time synchronously
    map_ = *LoadIndex();

    // Now update asynchronously
    watch_interval_ticks_ = watch_interval_ticks;
    watcher_.Start(this, &PackManager::WatcherStart);
}

void PackManager::WatcherStart(void *pv) {
    while (!watcher_.IsStopping()) {
        // Only reload if the file has been modified
        struct stat s;
        memset(&s, 0, sizeof(s));
        stat(indexfile_.c_str(), &s);
        if (s.st_mtime != mtime_) {
            mtime_ = s.st_mtime;
            PackMap *map = LoadIndex();
            if (map != NULL) {
                thread_.Post(1, this, new WatcherData(map));
            }
        }
        watcher_.RunLoop(watch_interval_ticks_);
    }
}

void PackManager::OnMessage(base::Message *msg) {
    // Executing on game thread. Accept new map.
    if (msg->id == 1) {
        RLOG() << "Importing new index...";
        WatcherData *data = (WatcherData *)msg->data;
        map_ = *(data->map_);
        delete data;
    }
}

std::map<dword,PackId> *PackManager::LoadIndex() {
    // Load the index and return it as a map
    IndexLoader index;
    if (!index.InitFromFile(indexfile_.c_str())) {
        return NULL;
    }
    PackMap *map = new PackMap;
    int c = index.GetCount();
    for (int i = 0; i < c; i++) {
        const IndexEntry *entry = index.GetEntry(i);
        map->insert(PackMap::value_type(entry->packid.id, entry->packid));
    }
    return map;
}

} // namespace wi
