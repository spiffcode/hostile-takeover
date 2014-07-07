#include "game/completemanager.h"
#include "mpshared/misc.h"
#include "base/format.h"
#include "game/httppackmanager.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

namespace wi {

void CompleteManager::Init() {
    map_.clear();
    DIR *pdir = opendir(completedir_.c_str());
    dirent *pdent;
    while ((pdent = readdir(pdir)) != NULL) {
        dword key;
        MissionHash mh;
        if (!ParseFilename(pdent->d_name, &key, mh.hash)) {
            continue;
        }
        map_.insert(CompleteMap::value_type(key, mh));
    }
    closedir(pdir);
}

bool CompleteManager::ParseFilename(const char *filename, dword *pkey,
        byte *hash) {

    LOG() << filename;

    // key     -hash
    // kkkkkkkk-hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
    if (strlen(filename) != 8 + 1 + 32) {
        return false;
    }
    char szId[9];
    strncpyz(szId, filename, 9);
    if (!base::Format::ToDword(szId, 16, pkey)) {
        return false;
    }
    return base::Format::FromHex(filename + 8 + 1, hash, 16);
}

const char *CompleteManager::GetFilename(dword key, MissionHash *pmh) {
    char szHash[33];
    strncpyz(szHash, base::Format::ToHex(pmh->hash, 16), sizeof(szHash));
    return base::Format::ToString("%08x-%s", key, szHash);
}

bool CompleteManager::IsComplete(const MissionIdentifier *pmiid) {
    // Quickly fail
    dword key = GetKey(pmiid);
    if (map_.count(key) == 0) {
        return false;
    }

    // Slow verification
    MissionHash mh;
    if (!GetMissionHash(pmiid, &mh)) {
        return false;
    }

    // See if this hash is known.
    // Using a multimap for backwards compat reasons. See BadHashString.
    std::pair<CompleteMap::iterator, CompleteMap::iterator> range;
    range = map_.equal_range(key);
    for (CompleteMap::iterator it = range.first; it != range.second; it++) {
        if (memcmp(mh.hash, it->second.hash, sizeof(mh.hash)) == 0) {
            return true;
        }
    }

    return false;
}

bool CompleteManager::GetMissionHash(const MissionIdentifier *pmiid, 
        MissionHash *pmh) {
    gppackm->Mount(gpakr, &pmiid->packid);
    if (!gpakr.HashFile(pmiid->szLvlFilename, pmh->hash)) {
        gppackm->Unmount(gpakr, &pmiid->packid);
        return false;
    }
    gppackm->Unmount(gpakr, &pmiid->packid);
    return true;
}

dword CompleteManager::GetKey(const MissionIdentifier *pmiid) {
    const char *s = base::Format::ToString("%08x", pmiid->packid.id);
    return BadHashString(s) ^ BadHashString(pmiid->szLvlFilename);
}

void CompleteManager::MarkComplete(const MissionIdentifier *pmiid) {
    MissionHash mh;
    if (!GetMissionHash(pmiid, &mh)) {
        return;
    }
    dword key = GetKey(pmiid);
    const char *filename = GetFilename(key, &mh);
    char szFilename[8 + 1 + 32 + 1];
    strncpyz(szFilename, filename, sizeof(szFilename));
    const char *filepath = base::Format::ToString("%s/%s",
            completedir_.c_str(), szFilename);

    // This will create a zero length file. Perhaps in the future stats can
    // be stored in this file, and this will become StatManager instead.

    FILE *pf = fopen(filepath, "w");
    if (pf != NULL) {
        fclose(pf);
        map_.insert(CompleteMap::value_type(key, mh));
    }
}

dword CompleteManager::BadHashString(const char *s) {
    // This string hash is here for a few reasons:
    // 1. WI v1.5 CompleteManager uses this hash algorithm, and has
    //    committed its values to storage (oops)
    // 2. This used to be the default string hash algorithm. It is no longer,
    //    because it is broken. For example, s_05.lvl hashes to the same value
    //    as s_10.lvl (as well as 6 to 11, 7 to 12, 8 to 13, and 9 to 14).
    // 3. The easiest thing to do is to continue using it, and use a multimap
    //    for lookup, instead of a map (which is now the case).
    dword h = 0;
    for (; *s != 0; s++) {
        h = 5 * h + *s;
    }
    return h;
}

} // namespace wi
