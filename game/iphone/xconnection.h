#ifndef __XCONNECTION_H__
#define __XCONNECTION_H__

#include "base/messagequeue.h"
#include "game/ht.h"
#include "game/iphone/xtransport.h"

namespace wi {

class XTransport;

class XConnection : public Connection, base::MessageHandler {
public:
    XConnection(XTransport *xtrans, dword gameid);
    ~XConnection();
    virtual dword gameid() { return gameid_; }

    // Connection overrides
	virtual bool AsyncSend(NetMessage *pnm);
	virtual void AsyncDisconnect();

    void OnNetMessage(NetMessage **ppnm);

private:
    virtual void OnMessage(base::Message *pmsg);

    XTransport *xtrans_;
    dword gameid_;
};

} // namespace wi

#endif // __XCONNECTION_H__
