#include "mpshared/xpump.h"
#include "mpshared/messages.h"
#include "base/socketaddress.h"
#include "base/socket.h"
#include "base/log.h"
#include "inc/rip.h"

namespace wi {
    
//---------------------------------------------------------------------------
// XPump implementation

XPump::XPump() : sock_(NULL), notify_(NULL), pbbSendFirst_(NULL), log_(NULL),
    notifycloseok_(false) {
}

XPump::~XPump() {
    // Free send buffers
    base::ByteBuffer *pbbT = pbbSendFirst_;
    while (pbbT != NULL) {
        base::ByteBuffer *pbbNext = pbbT->pbbNext_;
        delete pbbT;
        pbbT = pbbNext;
    }
    
    // Close socket
    Close();
}

bool XPump::Attach(base::Socket *sock, XPumpNotify *notify, XMsgLog *log) {
    LOG() << "XPump attached to " << sock->GetRemoteAddress().ToString();

    // XPump will receive socket notifications. This puts the socket into
    // async mode.

    log_ = log;
    logid_ = 0;
    notify_ = notify;
    sock_ = sock;
    sock_->SetNotify(this);

    return true;
}

void XPump::Close() {
    if (sock_ != NULL) {
        sock_->Close();
        delete sock_;
        sock_ = NULL;
    }
}

void XPump::NotifyCloseOk() {
    // Notify when all data has been sent. This is needed for asynchronous
    // closing.
    notifycloseok_ = true;
    if (pbbSendFirst_ == NULL) {
        notify_->OnCloseOk();
    }
}

bool XPump::Send(base::ByteBuffer *pbb) {
    if (pbb == NULL)
        return false;

#ifdef LOGGING
    base::ByteBuffer *pbbT = pbb->Clone();
    XMsg *pmsg = XMsgFromBuffer(*pbbT, pbbT->Length());
    LOG() << base::Log::Format("0x%08lx", notify_) << ": " << pmsg->ToString()
            << ", " << pbb->Length() << " bytes";
    delete pmsg;
    delete pbbT;
#endif

    if (log_ != NULL) {
        log_->Log(*pbb, -1, logid_, (dword)(long64)notify_);
    }

    base::ByteBuffer **ppbbSend = &pbbSendFirst_;
    while ((*ppbbSend) != NULL) {
        ppbbSend = &(*ppbbSend)->pbbNext_;
    }
    *ppbbSend = pbb;
    pbb->pbbNext_ = NULL;
    return ProcessWrite();
}

bool XPump::ProcessWrite() {
    // Start sending bytes from queue

    while (pbbSendFirst_ != NULL) {
        base::ByteBuffer *pbb = pbbSendFirst_;
        int cb = sock_->Send(pbb->Data(), pbb->Length());
        if (cb < 0) {
            if (sock_->IsBlocking()) {
                return true;
            }
            notify_->OnError(sock_->GetError());
            return false;
        }
        //LOG() << "Sent " << cb << " bytes out of " << pbb->Length();
        if (cb != pbb->Length()) {
            pbb->Shift(cb);
            break;
        }
        pbbSendFirst_ = pbb->pbbNext_;
        delete pbb;
    }
    if (notifycloseok_ && pbbSendFirst_ == NULL) {
        notify_->OnCloseOk();
    }
    return true;
}

bool XPump::ProcessRead() {
    // Read data out of the socket
    while (true) {
        static byte s_abT[4096];
        int cb = sock_->Recv(s_abT, sizeof(s_abT));
        if (cb < 0) {
            if (sock_->IsBlocking()) {
                return true;
            }
            notify_->OnError(sock_->GetError());
            return false;
        }
        //LOG() << "Read " << cb << " bytes.";
        bbRead_.WriteBytes(s_abT, cb);
    }
}

bool XPump::Peek(dword *pcb) {
    dword cbT;
    if (pcb == NULL) {
        pcb = &cbT;
    }
    return XMsg::ExtractSize(bbRead_, pcb) && bbRead_.Length() >= *pcb;
}

bool XPump::Dispatch() {
    dword cbMsg;
    if (!Peek(&cbMsg)) {
        return false;
    }

    if (log_ != NULL) {
        log_->Log(bbRead_, (int)cbMsg, (dword)(long64)notify_, logid_);
    }

    // Instantiate and dispatch this message.
    XMsg *pmsg = XMsgFromBuffer(bbRead_, cbMsg);
    if (pmsg == NULL) {
        notify_->OnError(-1);
        return false;
    }

#ifdef LOGGING
    LOG() << base::Log::Format("0x%08lx", notify_) << ": "
            << pmsg->ToString() << ", " << cbMsg << " bytes.";
#endif

    DispatchXMsg(pmsg);
    delete pmsg;
    return Peek();
}

XMsg *XPump::XMsgFromBuffer(base::ByteBuffer& bb, dword cb) {
    dword cbT;
    if (!XMsg::ExtractSize(bb, &cbT)) {
        return NULL;
    }
    Assert(cb == cbT);
    if (cb != cbT) {
        return NULL;
    }
    dword id;
    if (!XMsg::ExtractId(bb, &id)) {
        return NULL;
    }
    
    switch (id) {
    case XMSG_HANDSHAKE:
        return XMsgHandshake::FromBuffer(bb, cb);

    case XMSG_HANDSHAKERESULT:
        return XMsgHandshakeResult::FromBuffer(bb, cb);

    case XMSG_SHOWMESSAGE:
        return XMsgShowMessage::FromBuffer(bb, cb);

    case XMSG_ECHO:
        return XMsgEcho::FromBuffer(bb, cb);

    case XMSG_PROTOCOLERROR:
        return XMsgProtocolError::FromBuffer(bb, cb);

    case XMSG_LOGIN:
        return XMsgLogin::FromBuffer(bb, cb);

    case XMSG_LOGINRESULT:
        return XMsgLoginResult::FromBuffer(bb, cb);

    case XMSG_SIGNOUT:
        return XMsgSignOut::FromBuffer(bb, cb);

    case XMSG_SIGNOUTRESULT:
        return XMsgSignOutResult::FromBuffer(bb, cb);

    case XMSG_LOBBYJOIN:
        return XMsgLobbyJoin::FromBuffer(bb, cb);

    case XMSG_LOBBYJOINRESULT:
        return XMsgLobbyJoinResult::FromBuffer(bb, cb);

    case XMSG_LOBBYCREATEROOM:
        return XMsgLobbyCreateRoom::FromBuffer(bb, cb);

    case XMSG_LOBBYCREATEROOMRESULT:
        return XMsgLobbyCreateRoomResult::FromBuffer(bb, cb);

    case XMSG_LOBBYCANJOINROOM:
        return XMsgLobbyCanJoinRoom::FromBuffer(bb, cb);

    case XMSG_LOBBYCANJOINROOMRESULT:
        return XMsgLobbyCanJoinRoomResult::FromBuffer(bb, cb);

    case XMSG_LOBBYLURKERCOUNT:
        return XMsgLobbyLurkerCount::FromBuffer(bb, cb);

    case XMSG_LOBBYADDROOM:
        return XMsgLobbyAddRoom::FromBuffer(bb, cb);

    case XMSG_LOBBYREMOVEROOM:
        return XMsgLobbyRemoveRoom::FromBuffer(bb, cb);

    case XMSG_LOBBYUPDATEROOM:
        return XMsgLobbyUpdateRoom::FromBuffer(bb, cb);

    case XMSG_LOBBYLEAVE:
        return XMsgLobbyLeave::FromBuffer(bb, cb);

    case XMSG_LOBBYLEAVERESULT:
        return XMsgLobbyLeaveResult::FromBuffer(bb, cb);

    case XMSG_ROOMJOIN:
        return XMsgRoomJoin::FromBuffer(bb, cb);

    case XMSG_ROOMJOINRESULT:
        return XMsgRoomJoinResult::FromBuffer(bb, cb);

    case XMSG_ROOMADDPLAYER:
        return XMsgRoomAddPlayer::FromBuffer(bb, cb);

    case XMSG_ROOMREMOVEPLAYER:
        return XMsgRoomRemovePlayer::FromBuffer(bb, cb);

    case XMSG_ROOMSENDCHAT:
        return XMsgRoomSendChat::FromBuffer(bb, cb);

    case XMSG_ROOMRECEIVECHAT:
        return XMsgRoomReceiveChat::FromBuffer(bb, cb);

    case XMSG_ROOMADDGAME:
        return XMsgRoomAddGame::FromBuffer(bb, cb);

    case XMSG_ROOMREMOVEGAME:
        return XMsgRoomRemoveGame::FromBuffer(bb, cb);

    case XMSG_ROOMGAMEINPROGRESS:
        return XMsgRoomGameInProgress::FromBuffer(bb, cb);

    case XMSG_ROOMGAMEPLAYERNAMES:
        return XMsgRoomGamePlayerNames::FromBuffer(bb, cb);
  
    case XMSG_ROOMSTATUSCOMPLETE:
        return XMsgRoomStatusComplete::FromBuffer(bb, cb);
        
    case XMSG_ROOMCREATEGAME:
        return XMsgRoomCreateGame::FromBuffer(bb, cb);

    case XMSG_ROOMCREATEGAMERESULT:
        return XMsgRoomCreateGameResult::FromBuffer(bb, cb);

    case XMSG_ROOMCANJOINGAME:
        return XMsgRoomCanJoinGame::FromBuffer(bb, cb);

    case XMSG_ROOMCANJOINGAMERESULT:
        return XMsgRoomCanJoinGameResult::FromBuffer(bb, cb);

    case XMSG_ROOMLEAVE:
        return XMsgRoomLeave::FromBuffer(bb, cb);

    case XMSG_ROOMLEAVERESULT:
        return XMsgRoomLeaveResult::FromBuffer(bb, cb);

    case XMSG_GAMEJOIN:
        return XMsgGameJoin::FromBuffer(bb, cb);

    case XMSG_GAMEJOINRESULT:
        return XMsgGameJoinResult::FromBuffer(bb, cb);

    case XMSG_GAMESENDCHAT:
        return XMsgGameSendChat::FromBuffer(bb, cb);

    case XMSG_GAMERECEIVECHAT:
        return XMsgGameReceiveChat::FromBuffer(bb, cb);

    case XMSG_GAMENETMESSAGE:
        return XMsgGameNetMessage::FromBuffer(bb, cb);

    case XMSG_GAMEUPDATEEMPTY:
        return XMsgGameUpdateEmpty::FromBuffer(bb, cb);

    case XMSG_GAMEUPDATERESULT:
        return XMsgGameUpdateResult::FromBuffer(bb, cb);

    case XMSG_GAMEKILLED:
        return XMsgGameKilled::FromBuffer(bb, cb);

    case XMSG_GAMELEAVE:
        return XMsgGameLeave::FromBuffer(bb, cb);

    case XMSG_GAMELEAVERESULT:
        return XMsgGameLeaveResult::FromBuffer(bb, cb);

    default:
        return NULL;
    }
}

void XPump::DispatchXMsg(XMsg *pmsg) {
    switch (pmsg->id_) {
    case XMSG_HANDSHAKE:
        {
            XMsgHandshake *pmsgT = (XMsgHandshake *)pmsg;
            // clientid, protocolid
            notify_->OnHandshake(pmsgT->dw0_, pmsgT->dw1_);
        }
        break;

    case XMSG_HANDSHAKERESULT:
        {
            XMsgHandshakeResult *pmsgT = (XMsgHandshakeResult *)pmsg;
            // result, id_
            notify_->OnHandshakeResult(pmsgT->dw0_, pmsgT->dw1_);
        }
        break;

    case XMSG_SHOWMESSAGE:
        {
            XMsgShowMessage *pmsgT = (XMsgShowMessage *)pmsg;
            notify_->OnShowMessage(pmsgT->message_, pmsgT->ipRedirect_,
                    pmsgT->disconnect_);
        }
        break;

    case XMSG_ECHO:
        notify_->OnEcho();
        break;

    case XMSG_PROTOCOLERROR:
        {
            XMsgProtocolError *pmsgT = (XMsgProtocolError *)pmsg;
            // error
            notify_->OnProtocolError(pmsgT->dw0_);
        }
        break;

    case XMSG_LOGIN:
        {
            XMsgLogin *pmsgT = (XMsgLogin *)pmsg;
            // username, token, did
            notify_->OnLogin(pmsgT->s0_, pmsgT->s1_, pmsgT->s2_);
        }
        break;

    case XMSG_LOGINRESULT:
        {
            XMsgLoginResult *pmsgT = (XMsgLoginResult *)pmsg;
            // result
            notify_->OnLoginResult(pmsgT->dw0_);
        }
        break;

    case XMSG_SIGNOUT:
        {
            XMsgSignOut *pmsgT = (XMsgSignOut *)pmsg;
            notify_->OnSignOut();
        }
        break;

    case XMSG_SIGNOUTRESULT:
        {
            XMsgSignOutResult *pmsgT = (XMsgSignOutResult *)pmsg;
            // result
            notify_->OnSignOutResult(pmsgT->dw0_);
        }
        break;

    case XMSG_LOBBYJOIN:
        {
            XMsgLobbyJoin *pmsgT = (XMsgLobbyJoin *)pmsg;
            notify_->OnLobbyJoin();
        }
        break;

    case XMSG_LOBBYJOINRESULT:
        {
            XMsgLobbyJoinResult *pmsgT = (XMsgLobbyJoinResult *)pmsg;
            // result
            notify_->OnLobbyJoinResult(pmsgT->dw0_);
        }
        break;

    case XMSG_LOBBYCREATEROOM:
        {
            XMsgLobbyCreateRoom *pmsgT = (XMsgLobbyCreateRoom *)pmsg;
            // username, password
            notify_->OnLobbyCreateRoom(pmsgT->s0_, pmsgT->s1_);
        }
        break;

    case XMSG_LOBBYCREATEROOMRESULT:
        {
            XMsgLobbyCreateRoomResult *pmsgT =
                    (XMsgLobbyCreateRoomResult *)pmsg;
            // result, roomid
            notify_->OnLobbyCreateRoomResult(pmsgT->dw0_, pmsgT->dw1_);
        }
        break;

    case XMSG_LOBBYCANJOINROOM:
        {
            XMsgLobbyCanJoinRoom *pmsgT = (XMsgLobbyCanJoinRoom *)pmsg;
            // roomid, password
            notify_->OnLobbyCanJoinRoom(pmsgT->dw0_, pmsgT->s0_);
        }
        break;

    case XMSG_LOBBYCANJOINROOMRESULT:
        {
            XMsgLobbyCanJoinRoomResult *pmsgT =
                    (XMsgLobbyCanJoinRoomResult *)pmsg;
            // result
            notify_->OnLobbyCanJoinRoomResult(pmsgT->dw0_);
        }
        break;

    case XMSG_LOBBYLURKERCOUNT:
        {
            XMsgLobbyLurkerCount *pmsgT = (XMsgLobbyLurkerCount *)pmsg;
            // result
            notify_->OnLobbyLurkerCount(pmsgT->dw0_);
        }
        break;

    case XMSG_LOBBYADDROOM:
        {
            XMsgLobbyAddRoom *pmsgT = (XMsgLobbyAddRoom *)pmsg;
            notify_->OnLobbyAddRoom(pmsgT->room_, pmsgT->roomid_, pmsgT->priv_,
                    pmsgT->cPlayers_, pmsgT->cGames_);
        }
        break;

    case XMSG_LOBBYREMOVEROOM:
        {
            XMsgLobbyRemoveRoom *pmsgT = (XMsgLobbyRemoveRoom *)pmsg;
            // roomid
            notify_->OnLobbyRemoveRoom(pmsgT->dw0_);
        }
        break;

    case XMSG_LOBBYUPDATEROOM:
        {
            XMsgLobbyUpdateRoom *pmsgT = (XMsgLobbyUpdateRoom *)pmsg;
            // roomid, cplayers, cgames
            notify_->OnLobbyUpdateRoom(pmsgT->dw0_, pmsgT->dw1_, pmsgT->dw2_);
        }
        break;

    case XMSG_LOBBYLEAVE:
        {
            notify_->OnLobbyLeave();
        }
        break;

    case XMSG_LOBBYLEAVERESULT:
        {
            XMsgLobbyLeaveResult *pmsgT = (XMsgLobbyLeaveResult *)pmsg;
            // result
            notify_->OnLobbyLeaveResult(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMJOIN:
        {
            XMsgRoomJoin *pmsgT = (XMsgRoomJoin *)pmsg;
            // roomid, password
            notify_->OnRoomJoin(pmsgT->dw0_, pmsgT->s0_);
        }
        break;

    case XMSG_ROOMJOINRESULT:
        {
            XMsgRoomJoinResult *pmsgT = (XMsgRoomJoinResult *)pmsg;
            // result
            notify_->OnRoomJoinResult(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMADDPLAYER:
        {
            XMsgRoomAddPlayer *pmsgT = (XMsgRoomAddPlayer *)pmsg;
            // player name
            notify_->OnRoomAddPlayer(pmsgT->s0_);
        }
        break;

    case XMSG_ROOMREMOVEPLAYER:
        {
            XMsgRoomRemovePlayer *pmsgT = (XMsgRoomRemovePlayer *)pmsg;
            // hint, player name
            notify_->OnRoomRemovePlayer(pmsgT->dw0_, pmsgT->s0_);
        }
        break;

    case XMSG_ROOMSENDCHAT:
        {
            XMsgRoomSendChat *pmsgT = (XMsgRoomSendChat *)pmsg;
            // chat
            notify_->OnRoomSendChat(pmsgT->s0_);
        }
        break;

    case XMSG_ROOMRECEIVECHAT:
        {
            XMsgRoomReceiveChat *pmsgT = (XMsgRoomReceiveChat *)pmsg;
            // player, chat
            notify_->OnRoomReceiveChat(pmsgT->s0_, pmsgT->s1_);
        }
        break;
            
    case XMSG_ROOMADDGAME:
        {
            XMsgRoomAddGame *pmsgT = (XMsgRoomAddGame *)pmsg;
            notify_->OnRoomAddGame(pmsgT->player_, pmsgT->gameid_,
                    pmsgT->params_, pmsgT->minplayers_, pmsgT->maxplayers_,
                    pmsgT->title_, pmsgT->ctotal_);
        }
        break;

    case XMSG_ROOMREMOVEGAME:
        {
            XMsgRoomRemoveGame *pmsgT = (XMsgRoomRemoveGame *)pmsg;
            // gameid, ctotal
            notify_->OnRoomRemoveGame(pmsgT->dw0_, pmsgT->dw1_);
        }
        break;

    case XMSG_ROOMGAMEINPROGRESS:
        {
            XMsgRoomGameInProgress *pmsgT = (XMsgRoomGameInProgress *)pmsg;
            // gameid
            notify_->OnRoomGameInProgress(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMGAMEPLAYERNAMES:
        {
            XMsgRoomGamePlayerNames *pmsgT = (XMsgRoomGamePlayerNames *)pmsg;
            notify_->OnRoomGamePlayerNames(pmsgT->gameid_, pmsgT->cnames_,
                    pmsgT->anames_);
        }
        break;

    case XMSG_ROOMSTATUSCOMPLETE:
        notify_->OnRoomStatusComplete();
        break;

    case XMSG_ROOMCREATEGAME:
        {
            XMsgRoomCreateGame *pmsgT = (XMsgRoomCreateGame *)pmsg;
            notify_->OnRoomCreateGame(pmsgT->params_);
        }
        break;

    case XMSG_ROOMCREATEGAMERESULT:
        {
            XMsgRoomCreateGameResult *pmsgT =
                    (XMsgRoomCreateGameResult *)pmsg;
            notify_->OnRoomCreateGameResult(pmsgT->gameid_, pmsgT->result_,
                    &pmsgT->packid_);
        }
        break;

    case XMSG_ROOMCANJOINGAME:
        {
            XMsgRoomCanJoinGame *pmsgT = (XMsgRoomCanJoinGame *)pmsg;
            // gameid
            notify_->OnRoomCanJoinGame(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMCANJOINGAMERESULT:
        {
            XMsgRoomCanJoinGameResult *pmsgT =
                    (XMsgRoomCanJoinGameResult *)pmsg;
            // result
            notify_->OnRoomCanJoinGameResult(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMLEAVE:
        {
            XMsgRoomLeave *pmsgT = (XMsgRoomLeave *)pmsg;
            // hint
            notify_->OnRoomLeave(pmsgT->dw0_);
        }
        break;

    case XMSG_ROOMLEAVERESULT:
        notify_->OnRoomLeaveResult();
        break;

    case XMSG_GAMEJOIN:
        {
            XMsgGameJoin *pmsgT = (XMsgGameJoin *)pmsg;
            // gameid, roomid
            notify_->OnGameJoin(pmsgT->dw0_, pmsgT->dw1_);
        }
        break;

    case XMSG_GAMEJOINRESULT:
        {
            XMsgGameJoinResult *pmsgT = (XMsgGameJoinResult *)pmsg;
            // result
            notify_->OnGameJoinResult(pmsgT->dw0_);
        }
        break;

    case XMSG_GAMESENDCHAT:
        {
            XMsgGameSendChat *pmsgT = (XMsgGameSendChat *)pmsg;
            // chat
            notify_->OnGameSendChat(pmsgT->s0_);
        }
        break;

    case XMSG_GAMERECEIVECHAT:
        {
            XMsgGameReceiveChat *pmsgT = (XMsgGameReceiveChat *)pmsg;
            // player, chat
            notify_->OnGameReceiveChat(pmsgT->s0_, pmsgT->s1_);
        }
        break;

    case XMSG_GAMENETMESSAGE:
        {
            XMsgGameNetMessage *pmsgT = (XMsgGameNetMessage *)pmsg;
            notify_->OnGameNetMessage(&pmsgT->pnm_);
        }
        break;

    case XMSG_GAMEKILLED:
        {
            XMsgGameKilled *pmsgT = (XMsgGameKilled *)pmsg;
            // gameid
            notify_->OnGameKilled(pmsgT->dw0_);
        }
        break;
   
    case XMSG_GAMELEAVE:
        notify_->OnGameLeave();
        break;

    case XMSG_GAMELEAVERESULT:
        {
            XMsgGameLeaveResult *pmsgT = (XMsgGameLeaveResult *)pmsg;
            // result
            notify_->OnGameLeaveResult(pmsgT->dw0_);
        }
        break;

    default:
        notify_->OnError(-1);
        Assert(false);
        break;
    }
}

void XPump::OnConnectEvent(base::Socket *sock) {
    // This will never get called.
    Assert(false);
}

void XPump::OnReadEvent(base::Socket *sock) {
    // Read the socket
    Assert(sock == sock_);
    ProcessRead();

    // If there are messages, notify that there are messages. This allows the
    // notifyee to do per-message procesing.

    if (Peek()) {
        if (!notify_->OnMessages()) {
            // OnMessages returning false means execute the default,
            // which is to dispatch them all.
            while (Dispatch()) {
                ;
            }
        }
    }
}

void XPump::OnWriteEvent(base::Socket *sock) {
    Assert(sock == sock_);
    ProcessWrite();
}

void XPump::OnCloseEvent(base::Socket *sock) {
    Assert(sock == sock_);
    LOG() << "Socket closed from " << sock->GetRemoteAddress().ToString();
    sock_->SetNotify(NULL);
    notify_->OnClose(sock_->GetError());
}

bool XPump::IsClosed() {
    if (sock_ == NULL) {
        return true;
    }
    return sock_->GetState() == base::Socket::CS_CLOSED;
}
    
} // namespace wi
