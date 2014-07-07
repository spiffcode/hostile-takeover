#include "game/iphone/xconnection.h"
#include "game/iphone/input.h"
#include "base/socket.h"

const int kidmConnect = 1;
const int kidmDisconnect = 2;

namespace wi {

XConnection::XConnection(XTransport *xtrans, dword gameid) : xtrans_(xtrans),
        gameid_(gameid) {
    thread_.Post(kidmConnect, this);
}

XConnection::~XConnection() {
    // When the game exits, the connection gets deleted. When this happens,
    // it is necessary for the transport to disconnect from the game.
    LOG();
    xtrans_->DisconnectGame(gameid_);
}

bool XConnection::AsyncSend(NetMessage *pnm) {
    return xtrans_->SendNetMessage(pnm);
}

void XConnection::OnMessage(base::Message *pmsg) {
    LOG() << pmsg->id;

    switch (pmsg->id) {
    case kidmConnect:
        // Callers to AsyncConnect expect to get called back
        // asynchronously.
        if (m_pccb != NULL) {
            m_pccb->OnConnectComplete(this);
        }
        break;

    case kidmDisconnect:
        // This will cause the game to exit, then this connection
        // will get deleted.
        if (m_pccb != NULL) {
            m_pccb->OnDisconnect(this);
        }
        break;
    }

    // Cause a message to be pumped through the game input loop to cause it
    // to wake up, since processing here was done in MessageQueue dispatch.

    thread_.Post(kidmNullEvent, NULL);
}

void XConnection::AsyncDisconnect() {
    LOG();
    thread_.Post(kidmDisconnect, this);
}

void XConnection::OnNetMessage(NetMessage **ppnm) {
#ifdef LOGGING
    LOG() << PszFromNetMessage(*ppnm);
#endif
    if (m_pccb != NULL) {
        m_pccb->OnReceive(this, ppnm);
    }
}

} // namespace wi
