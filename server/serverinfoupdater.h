#ifndef __SERVERINFOUPDATER_H__
#define __SERVERINFOUPDATER_H__

#include "server/server.h"
#include "base/socketaddress.h"
#include "base/thread.h"
#include "base/messagequeue.h"
#include "base/bytebuffer.h"
#include "base/sigslot.h"
#include "server/httppost.h"
#include "yajl/api/yajl_gen.h"
#include "yajl/src/yajl_buf.h"
#include "yajl/src/yajl_encode.h"
#include <string>

namespace wi {

struct ServerInfoResult : base::MessageData {
    ServerInfoResult(const base::ByteBuffer& result) {
        this->result.WriteBytes(result.Data(), result.Length());
    }
    base::ByteBuffer result;
};

class ServerInfoUpdater : base::MessageHandler, public base::has_slots<> {
public:
    ServerInfoUpdater(Server& server, base::SocketAddress& post_address,
            const std::string& post_path, int sort_key,
            const std::string& server_name, const std::string& server_location,
            const std::string& server_type,
            base::SocketAddress& public_address, const std::string& extra_json,
            int expires);
    ~ServerInfoUpdater();

    void Start();

    void set_drain() { drain_ = true; }
    void clear_drain() { drain_ = false; }

    base::signal2<ServerInfoUpdater *, const base::ByteBuffer&> SignalOnResponse;

private:
    void ThreadStart(void *pv);
    void OnPostComplete(HttpPost *post, int status_code, int error,
            const base::ByteBuffer& result);
    base::ByteBuffer *MakeBody();
    std::string MakeJson();
    void GenNum(yajl_gen g, const char *key, dword value);
    void GenString(yajl_gen g, const char *key, const char *value);
    void ParseExtra(const std::string& extra_json);

    // MessageHandler
    virtual void OnMessage(base::Message *pmsg);

    Server& server_;
    base::Thread worker_;
    base::SocketAddress post_address_;
    std::string post_path_;
    int sort_key_;
    std::string server_name_;
    std::string server_location_;
    std::string server_type_;
    base::SocketAddress public_address_;
    int expires_;
    typedef std::map<std::string,std::string> ExtraMap;
    ExtraMap extra_;
    bool drain_;
};

} // namespace wi

#endif // __SERVERINFOUPDATER_H__
