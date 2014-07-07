#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "inc/basictypes.h"
#include "mpshared/mpht.h"
#include "server/endpoint.h"

#define kcmsRateMax 250
#define kcmsLagging 5000

namespace wi {

struct PlayerStats {
    char name[kcbPlayerName];
    Pid pid;
    char ip[32];
    char did[64];
    WinStats ws;
};

struct LatencyRecord { // latr
    long cUpdatesBlock;
    long cmsLatency;
};

const int kcLatencyRecordsMax = 8;

class Player {
    friend class PlayerMgr;

public:
    Player();

    bool Init(Pid pid);
    Pid pid() const { return pid_; }
    void SetEndpoint(Endpoint *endpoint);
    Endpoint *endpoint() const { return endpoint_; }
    void SetName(const char *name) { strncpyz(name_, name, sizeof(name_)); }
    const char *name() const { return name_; }
    void SetSide(Side side) { side_ = side; }
    Side side() const { return side_; }
	void SetFlags(word wf) { wf_ = wf; }
    word flags() const { return wf_; }
    void SaveWinStats(const WinStats& ws, bool lock);
    const WinStats& winstats() { return ws_; }
    void GetPlayerStats(PlayerStats *ps);
    int lag() const { return nLagState_; }
	int GetLagTimeout();
    void SetLagState(int nLagState);
	int UpdateLagState(long updates);
    void AddLatencyRecord(LatencyRecord *platr);
    int GetLatencyRecordCount();
    void ClearLatencyRecords() { clatr_ = 0; }
    const LatencyRecord *GetLatencyRecord(int i);
    void SetDid(const char *did) { strncpyz(did_, did, sizeof(did_)); }

    long updates() { return updates_; }
    bool havestats() { return havestats_; }
    bool statslocked() { return havestats_ && (ws_.ff & kfwsLocked) != 0; }
    const base::SocketAddress& address() { return address_; }
    const char *did() { return did_; }
    
private:
    base::SocketAddress address_;
    WinStats ws_;
    word wf_;
    Pid pid_;
    Side side_;
    int lag_;
    Endpoint *endpoint_;
	int nLagState_;
	long64 tLagStart_;
	long64 tLastLag_;
    char name_[kcbPlayerName];
    LatencyRecord alatr_[kcLatencyRecordsMax];
    int clatr_;
    long updates_;
    UpdateResult aur_[2 * kcmsLagging / kcmsRateMax];
    int cur_;
    bool havestats_;
    bool anonymous_;
    char did_[64];
};

} // namespace wi

#endif // __PLAYER_H__
