#ifndef __PACKINFOMANAGER_H__
#define __PACKINFOMANAGER_H__

#include "inc/basictypes.h"
#include "mpshared/mpht.h"
#include "yajl/wrapper/jsonbuilder.h"
#include <string>

namespace wi {

class PackInfoManager {
public:
    PackInfoManager(const char *cachedir);
    ~PackInfoManager();

    json::JsonMap *GetInfoMap(const PackId *ppackid);

protected:
    bool GetPackId(json::JsonMap *map, PackId *ppackid);
    const char *GetString(const json::JsonMap *map, const char *key);
    json::JsonMap *LoadInfoMap(FILE *file);
    const char *GetInfoFilename(const PackId *ppackid);

    std::string cachedir_;
};

} // namespace wi

#endif // __PACKINFOMANAGER_H__
