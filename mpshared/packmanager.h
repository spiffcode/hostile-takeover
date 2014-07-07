#ifndef __PACKMANAGER_H__
#define __PACKMANAGER_H__

#include "inc/basictypes.h"
#include "mpshared/enum.h"
#include "mpshared/mpht.h"
#include "mpshared/packfile.h"
#include "base/messagehandler.h"
#include "base/messagequeue.h"
#include "base/thread.h"
#include <string>
#include <map>
#include <sys/types.h>

namespace wi {

class PackManager : base::MessageHandler { // packm
public:
    PackManager(const char *cachedir);
    ~PackManager();

    void InitFromInstalled();
    void WatchIndex(const char *indexfile,
            int watch_interval_ticks = 100 * 60 * 5);

    bool EnumPacks(Enum *penm, PackId *ppackid);
    int IsInstalled(const PackId *ppackid, PackId *ppackidUpgrade = NULL);
    bool Remove(const PackId *ppackid, bool fCheckHash = true);
    bool Mount(PackFileReader& pakr, const PackId *ppackid);
    bool Unmount(PackFileReader& pakr, const PackId *ppackid);
    static const char *GetPackFilename(const PackId *ppackid);

protected:
    const char *GetPackFilepath(const PackId *ppackid);
    bool ParseFilename(const char *filename, PackId *ppackid);
    void WatcherStart(void *pv);
    std::map<dword,PackId> *LoadIndex();

    // MessageHandler
    void OnMessage(base::Message *msg);

    std::string cachedir_;
    std::string indexfile_;
    time_t mtime_;

    typedef std::map<dword,PackId> PackMap;
    PackMap map_;

    struct WatcherData : base::MessageData {
        WatcherData(PackMap *map) : map_(map) {}
        ~WatcherData() { delete map_; }
        PackMap *map_;
    };
    base::Thread watcher_;
    int watch_interval_ticks_;
};

} // namespace wi

#endif // __PACKMANAGER_H__
