#ifndef __LEVELINFOCACHE_H__
#define __LEVELINFOCACHE_H__

#include "inc/basictypes.h"
#include "server/levelinfo.h"
#include "mpshared/packmanager.h"
#include "mpshared/mpht.h"
#include <map>
#include <string>
#include <vector>

namespace wi {

typedef std::map<const std::string, LevelInfo> LevelInfoMap;
typedef std::map<dword, std::pair<PackId, LevelInfoMap> > PackMap;

class LevelInfoCache
{
public:
    LevelInfoCache(PackManager& packm) : packm_(packm) {}

    bool Submit(PackFileReader& pakr, const PackId& packid);
    bool SubmitHelper(const PackId& packid);
    void SubmitIndex(const std::string& indexfile);
    int FindInfo(const PackId& packid, const std::string& level,
            LevelInfo *pinfo, PackId *ppackidUpgrade = NULL);
    void RemoveInfo(const PackId& packid);
    bool empty() { return map_.empty(); }

private:
    void BuildCache();
    void CreateLevelInfoMap(PackFileReader& pakr, const PackId& packid,
            LevelInfoMap *infomap);

    PackMap map_;
    PackManager& packm_;
};

} // namespace wi

#endif // __LEVELINFOCACHE_H__
