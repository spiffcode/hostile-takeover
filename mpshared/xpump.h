#ifndef __XPUMP_H__
#define __XPUMP_H__

#include "mpshared/xmsg.h"
#include "mpshared/messages.h"
#include "mpshared/xmsglog.h"
#include "base/socket.h"
#include "inc/rip.h"

namespace wi {

class XMsg;

class XPumpNotify
{
public:
    virtual void OnHandshake(dword clientid, dword protocolid) { Assert(); }
    virtual void OnHandshakeResult(dword result, dword id) { Assert(); }
    virtual void OnShowMessage(const char *message, dword ipRedirect,
        bool disconnect) { Assert(); }
    virtual void OnEcho() { Assert(); }
    virtual void OnProtocolError(dword error) { Assert(); }
    virtual void OnLogin(const char *username, const char *token, const char *did) { Assert(); }
    virtual void OnLoginResult(dword loginResult) { Assert(); }
    virtual void OnSignOut() { Assert(); }
    virtual void OnSignOutResult(dword result) { Assert(); }
    virtual void OnLobbyJoin() { Assert(); }
    virtual void OnLobbyJoinResult(dword result) { Assert(); }
    virtual void OnLobbyCreateRoom(const char *name, const char *password)
            { Assert(); }
    virtual void OnLobbyCreateRoomResult(dword result, dword roomid)
            { Assert(); }
    virtual void OnLobbyCanJoinRoom(dword roomid, const char *password)
            { Assert(); }
    virtual void OnLobbyCanJoinRoomResult(dword result) { Assert(); }
    virtual void OnLobbyLurkerCount(dword count) { Assert(); }
    virtual void OnLobbyAddRoom(const char *room, dword roomid, bool priv,
            dword cPlayers, dword cGames) { Assert(); }
    virtual void OnLobbyRemoveRoom(dword roomid) { Assert(); }
    virtual void OnLobbyUpdateRoom(dword roomid, dword cPlayers, dword cGames)
            { Assert(); }
    virtual void OnLobbyLeave() { Assert(); }
    virtual void OnLobbyLeaveResult(dword result) { Assert(); }
    virtual void OnRoomJoin(dword roomid, const char *password) { Assert(); }
    virtual void OnRoomJoinResult(dword result) { Assert(); }
    virtual void OnRoomAddPlayer(const char *player) { Assert(); }
    virtual void OnRoomRemovePlayer(dword hint, const char *player)
            { Assert(); }
    virtual void OnRoomSendChat(const char *chat) { Assert(); }
    virtual void OnRoomReceiveChat(const char *player, const char *chat)
            { Assert(); }
    virtual void OnRoomAddGame(const char *player, dword gameid,
            const GameParams& params, dword minplayers, dword maxplayers,
            const char *title, dword ctotal) { Assert(); }
    virtual void OnRoomRemoveGame(dword gameid, dword ctotal) { Assert(); }
    virtual void OnRoomGameInProgress(dword gameid) { Assert(); }
    virtual void OnRoomGamePlayerNames(dword gameid, dword cnames,
            const PlayerName *anames) { Assert(); }
    virtual void OnRoomStatusComplete() { Assert(); }
    virtual void OnRoomCreateGame(const GameParams& params) { Assert(); }
    virtual void OnRoomCreateGameResult(dword gameid, dword result,
            const PackId *ppackid) { Assert(); }
    virtual void OnRoomCanJoinGame(dword gameid) { Assert(); }
    virtual void OnRoomCanJoinGameResult(dword result) { Assert(); }
    virtual void OnRoomLeave(dword hint) { Assert(); }
    virtual void OnRoomLeaveResult() { Assert(); }
    virtual void OnGameJoin(dword gameid, dword roomid) { Assert(); }
    virtual void OnGameJoinResult(dword gameid) { Assert(); }
    virtual void OnGameSendChat(const char *chat) { Assert(); }
    virtual void OnGameReceiveChat(const char *player, const char *chat)
            { Assert(); }
    virtual void OnGameNetMessage(NetMessage **ppnm) { Assert(); }
    virtual void OnGameKilled(dword gameid) { Assert(); }
    virtual void OnGameLeave() { Assert(); }
    virtual void OnGameLeaveResult(dword result) { Assert(); }
    virtual bool OnMessages() { return false; }
    virtual void OnCloseOk() { return; }
    virtual void OnError(int error) { Assert(); }
    virtual void OnClose(int error) { Assert(); }
};

class XPump : base::SocketNotify {
public:
    XPump();
    ~XPump();

    bool Attach(base::Socket *socket, XPumpNotify *notify, XMsgLog *log = NULL);
    void Close();
    bool Send(base::ByteBuffer *pbb);
    bool Flush() { return ProcessWrite(); }
    bool Dispatch();
    bool IsClosed();
    void NotifyCloseOk();
    void SetLogId(dword id) { logid_ = id; }
    static XMsg *XMsgFromBuffer(base::ByteBuffer& bb, dword cb);
    base::Socket *socket() { return sock_; }

private:
    bool Peek(dword *pcb = NULL);
    bool ProcessRead();
    bool ProcessWrite();
    void DispatchXMsg(XMsg *pmsg);

    // SocketNotify
    virtual void OnConnectEvent(base::Socket *socket);
    virtual void OnReadEvent(base::Socket *socket);
    virtual void OnWriteEvent(base::Socket *socket);
    virtual void OnCloseEvent(base::Socket *socket);

    XMsgLog *log_;
    dword logid_;
    base::Socket *sock_;
    XPumpNotify *notify_;
    base::ByteBuffer *pbbSendFirst_;
    base::ByteBuffer bbRead_;
    bool notifycloseok_;
};

} // namespace wi

#endif // __XPUMP_H__
