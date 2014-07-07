#ifndef __STATSPOSTER_H__
#define __STATSPOSTER_H__

#include "base/socketaddress.h"
#include "base/thread.h"
#include "base/messagequeue.h"
#include "base/bytebuffer.h"
#include "base/sigslot.h"
#include "server/player.h"
#include "server/game.h"
#include "server/httppost.h"
#include "yajl/api/yajl_gen.h"
#include "yajl/src/yajl_buf.h"
#include "yajl/src/yajl_encode.h"
#include <string>
#include <deque>

namespace wi {

struct GameStatsData : base::MessageData {
    GameStatsData(const GameStats& stats) : stats(stats) { }
    GameStats stats;
};

class StatsPoster : base::MessageHandler, public base::has_slots<> {
public:
    StatsPoster(const base::SocketAddress& addr, const std::string& path);
    ~StatsPoster();

    void Submit(const GameStats& stats);

private:
    void ThreadStart(void *pv);
    void OnPostRequest(const GameStats& stats);
    void SchedulePosts();
    void OnPostComplete(HttpPost *post, int status_code, int error,
            const base::ByteBuffer& response);
    base::ByteBuffer *MakeBody(const GameStats& stats);
    std::string ToJson(const GameStats& stats);
    void GenNum(yajl_gen g, const char *key, dword value);
    void GenString(yajl_gen g, const char *key, const char *value);

    // MessageHandler interface
    virtual void OnMessage(base::Message *pmsg);

    base::Thread worker_; 
    std::deque<HttpPost *> waiting_;
    std::deque<HttpPost *> posting_;
    base::SocketAddress addr_;
    std::string path_;
};

} // namespace wi

#endif // __STATSPOSTER_H__
