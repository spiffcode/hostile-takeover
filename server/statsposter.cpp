#include "server/statsposter.h"
#include "base/base64.h"
#include "base/md5.h"
#include "server/secrets.h"

// Stats get posted to a remote server via http post. The http
// posts are asynchronous, but the dns lookups are not, so it is handled
// in a thread to insulate the callers of this class.

// If the server is down or not responding, the posts are kept in memory, and
// retried. The kcPostsWaitingMax is the max # that will be kept, before old
// ones are thrown away. kcPostsPostingMax is the number that is allowed to be
// posted simultaneously, so the load is kept even.

#define kcPostsWaitingMax 1000
#define kcPostsPostingMax 10
#define kcMaxAttempts 10

namespace wi {

StatsPoster::StatsPoster(const base::SocketAddress &addr,
        const std::string& path) : base::MessageHandler(worker_), addr_(addr),
        path_(path) {
    worker_.Start(this, &StatsPoster::ThreadStart);
}

StatsPoster::~StatsPoster() {
    // Thread::Stop gets called in the Thread destructor, which does a join
    // with the actual thread, which synchronizes exiting
}

void StatsPoster::ThreadStart(void *pv) {
    // Run and process messages. RunLoop will return when this thread
    // needs to exit.
    worker_.RunLoop();

    // Cleanup that needs to happen on this thread.
    while (waiting_.size() != 0) {
        delete waiting_[0];
        waiting_.pop_front();
    }
    while (posting_.size() != 0) {
        delete posting_[0];
        posting_.pop_front();
    }
}

void StatsPoster::Submit(const GameStats& stats) {
    worker_.Post(1, this, new GameStatsData(stats));
}

void StatsPoster::OnMessage(base::Message *pmsg) {
    GameStatsData *stats_data = (GameStatsData *)pmsg->data;
    OnPostRequest(stats_data->stats);
    delete stats_data;
}

void StatsPoster::OnPostRequest(const GameStats& stats) {
    // Delete old ones, keep new ones if we have too many uncompleted
    HttpPost *post = new HttpPost(addr_, path_, MakeBody(stats));
    waiting_.push_back(post);
    if (waiting_.size() > kcPostsWaitingMax) {
        delete waiting_[0];
        waiting_.pop_front();
    }
    SchedulePosts();
}

void StatsPoster::OnPostComplete(HttpPost *post, int status_code, int error,
        const base::ByteBuffer& response) {
#if 0
    RLOG() << "STATSPOSTER: PostComplete " << (dword)post << " status_code "
            << status_code;
#endif

    // Find it in posting_
    std::deque<HttpPost *>::iterator it = std::find(posting_.begin(),
            posting_.end(), post);
    
    // It should always be in posting_.
    if (it != posting_.end()) {
        posting_.erase(it);
        post->SignalOnComplete.disconnect(this);

        // Success? If so delete it and schedule the next one. If not success,
        // push it to the front of waiting_ for later.
        if (status_code == 200) {
            delete post;
        } else {
            RLOG() << "ERROR " << status_code
                << " posting game stats. attempt:" << post->attempts()
                << " waiting:" << waiting_.size()
                << " posting:" << posting_.size();
            if (post->attempts() >= kcMaxAttempts) {
                RLOG() << "ERROR attemped gamestat post " << kcMaxAttempts
                        << " times. Deleting.";
                delete post;
            } else {
                waiting_.push_front(post);
            }
        }
    } else {
        RLOG() << "ERROR: Post " << (dword)post << " not found in posting_!";
    }

    // Schedule more posts 
    SchedulePosts();
}

void StatsPoster::SchedulePosts() {
    while (waiting_.size() != 0 && posting_.size() < kcPostsPostingMax) {
        HttpPost *post = waiting_[0];
        waiting_.pop_front();
        posting_.push_back(post);
        post->SignalOnComplete.connect(this, &StatsPoster::OnPostComplete);
        post->Submit();
#if 0
        RLOG() << "STATSPOSTER: Submiting post: " << (dword)post
                << " waiting:" << waiting_.size()
                << " posting:" << posting_.size();
#endif
    }
}

base::ByteBuffer *StatsPoster::MakeBody(const GameStats& stats) {
    std::string json = ToJson(stats);

    // Create HMAC like signature of hash(json + secret) so the receiver
    // can ensure it is valid.
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (const byte *)json.c_str(), json.size());
    MD5Update(&md5, (const byte *)kszStatSecret, strlen(kszStatSecret));
    byte hash[16];
    MD5Final(hash, &md5);
    char hash_str[33];
    strncpyz(hash_str, base::Format::ToHex(hash, sizeof(hash)),
            sizeof(hash_str));
    base::ByteBuffer *bb = new base::ByteBuffer(32 + json.size());
    bb->WriteBytes((const byte *)hash_str, 32);
    bb->WriteBytes((const byte *)json.c_str(), json.size());
    return bb;
}

// See stats/addgamestats.py for json description

std::string StatsPoster::ToJson(const GameStats& s) {
    // Use yajl to get the string encoding right

#ifdef DEBUG
    yajl_gen_config yajl_config = { 1, " " };
#else
    yajl_gen_config yajl_config = { 0, NULL };
#endif
    yajl_gen g = yajl_gen_alloc(&yajl_config);

    yajl_gen_map_open(g);
    GenNum(g, "server_id", s.server_id);
    GenNum(g, "server_start", s.server_start);
    GenNum(g, "gameid", s.gameid);
    GenNum(g, "packid_id", s.params.packid.id);
    GenString(g, "packid_hash", base::Format::ToHex(s.params.packid.hash,
            sizeof(s.params.packid.hash)));
    GenString(g, "title", s.info.title());
    GenString(g, "filename", s.info.filename());
    GenNum(g, "game_speed", s.params.tGameSpeed);
    GenNum(g, "min_players", s.info.minplayers());
    GenNum(g, "max_players", s.info.maxplayers());
    GenNum(g, "start_utc", s.start_utc);
    GenNum(g, "end_utc", s.end_utc);

    const char *str = "player_stats";
    yajl_gen_string(g, (const unsigned char *)str, strlen(str));
    yajl_gen_array_open(g);

    for (int i = 0; i < s.player_count; i++) {
        yajl_gen_map_open(g);
        GenString(g, "name", s.player_stats[i].name);
        GenNum(g, "pid", s.player_stats[i].pid);
        GenString(g, "ip", s.player_stats[i].ip);
        GenString(g, "did", s.player_stats[i].did);
        
        str = "winstats";
        yajl_gen_string(g, (const unsigned char *)str, strlen(str));
        yajl_gen_map_open(g);
        GenNum(g, "side_mask", s.player_stats[i].ws.sidm);
        GenNum(g, "side_mask_allies", s.player_stats[i].ws.sidmAllies);
        GenNum(g, "credits_acquired", s.player_stats[i].ws.cCreditsAcquired);
        GenNum(g, "credits_consumed", s.player_stats[i].ws.cCreditsConsumed);
        GenNum(g, "enemy_munts_killed",
                s.player_stats[i].ws.cEnemyMobileUnitsKilled);
        GenNum(g, "enemy_structs_killed",
                s.player_stats[i].ws.cEnemyStructuresKilled);
        GenNum(g, "munts_lost", s.player_stats[i].ws.cMobileUnitsLost);
        GenNum(g, "structs_lost", s.player_stats[i].ws.cStructuresLost);
        GenNum(g, "ff", s.player_stats[i].ws.ff);

        str = "unit_counts";
        yajl_gen_string(g, (const unsigned char *)str, strlen(str));
        yajl_gen_array_open(g);
        for (int ut = 0; ut < ARRAYSIZE(s.player_stats[i].ws.acut); ut++) {
            yajl_gen_integer(g, s.player_stats[i].ws.acut[ut]);
        }
        yajl_gen_array_close(g);

        str = "built_counts";
        yajl_gen_string(g, (const unsigned char *)str, strlen(str));
        yajl_gen_array_open(g);
        for (int ut = 0; ut < ARRAYSIZE(s.player_stats[i].ws.acutBuilt); ut++) {
            yajl_gen_integer(g, s.player_stats[i].ws.acutBuilt[ut]);
        }
        yajl_gen_array_close(g);

        yajl_gen_map_close(g);
        yajl_gen_map_close(g);
    }
    yajl_gen_array_close(g);
    yajl_gen_map_close(g);

    const char *buf;
    unsigned int len;
    yajl_gen_get_buf(g, (const unsigned char **)&buf, &len);
    std::string result(buf);
    yajl_gen_free(g);
    return result;
}

void StatsPoster::GenNum(yajl_gen g, const char *key, dword value) {
    yajl_gen_string(g, (const unsigned char *)key, strlen(key));
    const char *s = base::Format::ToString("%lu", value);
    yajl_gen_number(g, s, strlen(s));
}

void StatsPoster::GenString(yajl_gen g, const char *key, const char *value) {
    yajl_gen_string(g, (const unsigned char *)key, strlen(key));
    yajl_gen_string(g, (const unsigned char *)value, strlen(value));
}

} // namespace wi
