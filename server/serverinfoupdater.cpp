#include "server/serverinfoupdater.h"
#include "server/secrets.h"
#include "base/base64.h"
#include "base/md5.h"
#include "base/log.h"
#include "base/tick.h"
#include "mpshared/constants.h"
#include "mpshared/misc.h"
#include "yajl/wrapper/jsonbuilder.h"
#include <map>

namespace wi {

const int MSG_SERVERINFORESULT = 1;

ServerInfoUpdater::ServerInfoUpdater(Server& server,
        base::SocketAddress& post_address, const std::string& post_path,
        int sort_key, const std::string& server_name,
        const std::string& server_location, const std::string& server_type,
        base::SocketAddress& public_address, const std::string& extra_json,
        int expires) : server_(server),
        post_address_(post_address), post_path_(post_path),
        sort_key_(sort_key), server_name_(server_name),
        server_location_(server_location), server_type_(server_type),
        public_address_(public_address), expires_(expires), drain_(false) {
    ParseExtra(extra_json);
    worker_.Start(this, &ServerInfoUpdater::ThreadStart);
}

ServerInfoUpdater::~ServerInfoUpdater() {
    // Thread::Stop gets called in the Thread destructor, which does a join
    // with the actual thread, which synchronizes exiting
}

void ServerInfoUpdater::OnPostComplete(HttpPost *post, int status_code,
        int error, const base::ByteBuffer& result) {
    if (status_code != 200) {
        RLOG() << "ERROR posting ServerInfo, status code: " << status_code;
        return;
    }

    // Pass on the result, if there is one.
    if (result.Length() != 0) {
        thread_.Post(MSG_SERVERINFORESULT, this,
                new ServerInfoResult(result));
    }
}

void ServerInfoUpdater::OnMessage(base::Message *pmsg) {
    if (pmsg->id == MSG_SERVERINFORESULT) {
        ServerInfoResult *res = (ServerInfoResult *)pmsg->data;
        SignalOnResponse(this, res->result);
        delete res;
    }
}

void ServerInfoUpdater::ThreadStart(void *pv) {
    // Create and send a new ServerInfo post on an expires_ / 2 interval,
    // to keep it fresh. expires_ in seconds, so convert to ticks (100/second).
    while (true) {
        HttpPost *post = new HttpPost(post_address_, post_path_, MakeBody());
        if (post != NULL) {
            post->SignalOnComplete.connect(this,
                    &ServerInfoUpdater::OnPostComplete);
            post->Submit();
        }
        worker_.RunLoop(expires_ * 100 / 2);
        if (post != NULL) {
            post->SignalOnComplete.disconnect(this);
            delete post;
            post = NULL;
        }
        if (worker_.IsStopping()) {
            break;
        }
    }
}

base::ByteBuffer *ServerInfoUpdater::MakeBody() {
    std::string json = MakeJson();

    // Create HMAC like signature of hash(json + secret) so the receiver
    // can ensure it is valid.
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (const byte *)json.c_str(), json.size());
    MD5Update(&md5, (const byte *)kszServerInfoSecret,
            strlen(kszServerInfoSecret));
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

// See stats/serverinfo.py for json description. The poster generates
// one of the info maps.

std::string ServerInfoUpdater::MakeJson() {
#ifdef DEBUG
    yajl_gen_config yajl_config = { 1, " " };
#else
    yajl_gen_config yajl_config = { 0, NULL };
#endif
    yajl_gen g = yajl_gen_alloc(&yajl_config);

    yajl_gen_map_open(g);
    GenNum(g, "sort_key", sort_key_);
    GenString(g, "name", server_name_.c_str());
    GenString(g, "location", server_location_.c_str());
    GenString(g, "address", public_address_.ToString());
    GenNum(g, "protocol", kdwProtocolCurrent);
    if (drain_) {
        GenString(g, "status", "drain");
    } else {
        GenString(g, "status", "ok");
    }
    GenNum(g, "expires_utc", base::GetSecondsUnixEpocUTC() + expires_);
    GenNum(g, "start_utc", server_.start_time());
    GenNum(g, "player_count", server_.endpoint_count_thread_safe());
    GenString(g, "type", server_type_.c_str());

    // Add in the extras
    ExtraMap::iterator it = extra_.begin();
    for (; it != extra_.end(); it++) {
        GenString(g, it->first.c_str(), it->second.c_str());
    }

    yajl_gen_map_close(g);

    const char *buf;
    unsigned int len;
    yajl_gen_get_buf(g, (const unsigned char **)&buf, &len);
    std::string result(buf);
    yajl_gen_free(g);
    return result;
}

void ServerInfoUpdater::GenNum(yajl_gen g, const char *key, dword value) {
    yajl_gen_string(g, (const unsigned char *)key, strlen(key));
    const char *s = base::Format::ToString("%lu", value);
    yajl_gen_number(g, s, strlen(s));
}

void ServerInfoUpdater::GenString(yajl_gen g, const char *key,
        const char *value) {
    yajl_gen_string(g, (const unsigned char *)key, strlen(key));
    yajl_gen_string(g, (const unsigned char *)value, strlen(value));
}

void ServerInfoUpdater::ParseExtra(const std::string& extra_json) {
    if (extra_json.size() == 0) {
        return;
    }

    // Parse json
    json::JsonBuilder builder;
    builder.Start();
    builder.Update(extra_json.c_str(), extra_json.size());
    json::JsonObject *obj = builder.End();

    // Ensure a map results
    if (obj->type() != json::JSONTYPE_MAP) {
        delete obj;
        return;
    }
    json::JsonMap *map = (json::JsonMap *)obj;

    // Pull out the strings and add them to the extra map.
    Enum enm;
    const char *key;
    while ((key = map->EnumKeys(&enm)) != NULL) {
        const json::JsonObject *value_obj = map->GetObject(key);
        if (value_obj->type() != json::JSONTYPE_STRING && obj->type() != json::JSONTYPE_NUMBER) {
            continue;
        }
        const char *value =
        value_obj->type() == json::JSONTYPE_NUMBER ?
            ((const json::JsonNumber *)value_obj)->GetString() :
            ((const json::JsonString *)value_obj)->GetString();
        extra_.insert(ExtraMap::value_type(std::string(key),
                std::string(value)));
    }
    delete map;
}

void ServerInfoUpdater::SetInfo(const std::string key, const std::string value) {
    if (key == "sort") {
        int n = 0;
        base::Format::ToInteger(value.c_str(), 10, &n);
        sort_key_ = n;
    } else if (key == "status") {
        if (value == "drain") {
            drain_ = true;
        }
        if (value == "undrain" || value == "ok") {
            drain_ = false;
        }
    } else if (key == "name") {
        server_name_ = value;
    } else if (key == "location") {
        server_location_ = value;
    } else if (key == "type") {
        server_type_ = value;
    } else {
        if (value.empty()) {
            extra_.erase(key);
        } else {
            extra_[key] = value;
        }
    }
}

} // namespace wi
