#ifndef __COMPETEMANAGER_H__
#define __COMPETEMANAGER_H__

#include "game/ht.h"
#include <map>
#include <string>

namespace wi {

struct MissionHash {
    byte hash[16];
};

class CompleteManager {
public:
    CompleteManager(const char *completedir) : completedir_(completedir) {}
    ~CompleteManager() {}

    void Init();
    bool IsComplete(const MissionIdentifier *pmiid);
    void MarkComplete(const MissionIdentifier *pmiid);

private:
    dword BadHashString(const char *s);
    bool ParseFilename(const char *filename, dword *pkey, byte *hash);
    const char *GetFilename(dword key, MissionHash *pmh);
    bool GetMissionHash(const MissionIdentifier *pmiid, MissionHash *pmh);
    dword GetKey(const MissionIdentifier *pmiid);

    typedef std::multimap<dword, MissionHash> CompleteMap;
    CompleteMap map_;
    std::string completedir_;
};
extern CompleteManager *gpcptm;

} // namespace wi

#endif // __COMPETEMANAGER_H__
