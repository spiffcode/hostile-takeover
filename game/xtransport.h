#ifndef __XTRANSPORT_H__
#define __XTRANSPORT_H__

#include "game/ht.h"
#include "base/socketaddress.h"
#undef SetState

#include "mpshared/xmsg.h"
#include "mpshared/xpump.h"
#include "base/socket.h"

namespace wi {

class XTransport : public Transport, XPumpNotify, base::SocketNotify
{
public:
	XTransport(const base::SocketAddress& address);
    virtual ~XTransport();
    
	static int GetTransportDescriptions(TransportDescription *atrad,
            int ctradMax);
	static dword Open(TransportDescription *ptrad, Transport **pptra);

	// Transport implementation
	virtual dword Open();
	virtual void Close();
    virtual bool IsClosed();
    virtual dword Login(const char *username, const char *token);
    virtual dword SignOut();
    virtual const char *GetAnonymousUsername();
    virtual dword JoinLobby(ILobbyCallback *plcb);
    virtual bool LeaveLobby();
    virtual dword CreateRoom(const char *roomname, const char *password,
            dword *roomid);
    virtual dword CanJoinRoom(dword roomid, const char *password);
    virtual dword JoinRoom(dword roomid, const char *password,
            IRoomCallback *prcb);
    virtual bool SendChat(const char *chat);
    virtual dword CreateGame(GameParams *params, PackId *ppackidUpgrade,
            dword *gameid);
    virtual dword CanJoinGame(dword gameid);
    virtual void LeaveRoom(dword hint);

    virtual dword JoinGame(dword gameid, dword roomid);
    virtual bool SendNetMessage(NetMessage *pnm);
    virtual void LeaveGame();

    virtual void OnEvent(Event *pevt);

    enum State {
        XTS_INVALID = -1,
        XTS_CLOSED, XTS_CONNECTING, XTS_CONNECTED,
        XTS_HANDSHAKING, XTS_HANDSHAKEERROR, XTS_HANDSHAKESERVERFULL,
        XTS_HANDSHAKESUCCESS,
        XTS_OPEN,
        XTS_LOGGINGIN, XTS_LOGINFAILED, XTS_LOGGEDIN,
        XTS_SIGNINGOUT, XTS_SIGNOUTFAILED, XTS_SIGNEDOUT,
        XTS_JOINLOBBY, XTS_JOINLOBBYFAILED, XTS_JOINLOBBYSUCCESS,
        XTS_LOBBY,
        XTS_CREATINGROOM, XTS_CREATEROOMERROR, XTS_CREATEROOMSUCCESS,
        XTS_CANJOINROOM,
        XTS_JOININGROOM, XTS_JOINROOMERROR, XTS_JOINROOMSUCCESS,
        XTS_ROOM,
        XTS_CREATINGGAME, XTS_CREATEGAMEERROR, XTS_CREATEGAMESUCCESS,
        XTS_CANJOINGAME, XTS_LEAVINGROOM, 
        XTS_JOININGGAME, XTS_JOINGAMEERROR, XTS_JOINGAMESUCCESS,
        XTS_GAME,
    };

private:
    bool WaitForStateChange(long ctWait = 30 * 100);
    void SetState(State state);
    bool CheckState(State state0, State state1 = (State)-1);
    bool Handshake();
    bool ConnectGame(dword gameid);
    void SetGameCountStatus(dword ctotal);

    // Socket notify methods
    virtual void OnConnectEvent(base::Socket *socket);
    virtual void OnReadEvent(base::Socket *socket);
    virtual void OnWriteEvent(base::Socket *socket);
    virtual void OnCloseEvent(base::Socket *socket);

     // XPump methods
    virtual void OnHandshakeResult(dword result, dword id);
    virtual void OnShowMessage(const char *message, dword ipRedirect,
            bool disconnect);
    virtual void OnEcho();
    virtual void OnLoginResult(dword result);
    virtual void OnSignOutResult(dword result);
    virtual void OnLobbyJoinResult(dword result);
    virtual void OnLobbyLeaveResult(dword result);
    virtual void OnLobbyCreateRoomResult(dword result, dword roomid);
    virtual void OnLobbyCanJoinRoomResult(dword result);
    virtual void OnLobbyLurkerCount(dword count);
    virtual void OnLobbyAddRoom(const char *room, dword idRoom, bool priv,
            dword cPlayers, dword cGames);
    virtual void OnLobbyRemoveRoom(dword idRoom);
    virtual void OnLobbyUpdateRoom(dword idRoom, dword cPlayers, dword cGames);
    virtual void OnRoomJoinResult(dword result);
    virtual void OnRoomAddPlayer(const char *player);
    virtual void OnRoomRemovePlayer(dword hint, const char *player);
    virtual void OnRoomReceiveChat(const char *player, const char *chat);
    virtual void OnRoomAddGame(const char *player, dword gameid,
            const GameParams& params, dword minplayers, dword maxplayers,
            const char *title, dword ctotal);
    virtual void OnRoomRemoveGame(dword gameid, dword ctotal);
    virtual void OnRoomGameInProgress(dword gameid);
    virtual void OnRoomGamePlayerNames(dword gameid, dword cnames,
            const PlayerName *anames);
    virtual void OnRoomStatusComplete();
    virtual void OnRoomCreateGameResult(dword gameid, dword result,
            const PackId *ppackid);
    virtual void OnRoomCanJoinGameResult(dword result);
    virtual void OnRoomLeaveResult();
    virtual void OnGameJoinResult(dword gameid);
    virtual void OnGameReceiveChat(const char *player, const char *chat);
    virtual void OnGameNetMessage(NetMessage **ppnm);
    virtual void OnGameKilled(dword gameid);
    virtual void OnGameLeaveResult(dword gameid);
    virtual bool OnMessages();
    virtual void OnError(int error);
    virtual void OnClose(int error);

    base::SocketAddress address_;
    XPump xpump_;
    State state_;
    bool waiting_;
    bool error_;
    dword resultLogin_;
    dword resultSignOut_;
    dword resultLobbyJoin_;
    dword resultRoomJoin_;
    dword resultRoomCreate_;
    dword resultGameJoin_;
    dword resultGameCreate_;
    dword roomidCreate_;
    dword gameidCreate_;
    PackId packidCreate_;
    dword id_;
};

STARTLABEL(XtsLabels)
    LABEL(XTransport::XTS_CLOSED)
    LABEL(XTransport::XTS_CONNECTING)
    LABEL(XTransport::XTS_CONNECTED)
    LABEL(XTransport::XTS_HANDSHAKING)
    LABEL(XTransport::XTS_HANDSHAKEERROR)
    LABEL(XTransport::XTS_HANDSHAKESUCCESS)
    LABEL(XTransport::XTS_OPEN)
    LABEL(XTransport::XTS_LOGGINGIN)
    LABEL(XTransport::XTS_LOGINFAILED)
    LABEL(XTransport::XTS_LOGGEDIN)
    LABEL(XTransport::XTS_SIGNINGOUT)
    LABEL(XTransport::XTS_SIGNOUTFAILED)
    LABEL(XTransport::XTS_SIGNEDOUT)
    LABEL(XTransport::XTS_JOINLOBBY)
    LABEL(XTransport::XTS_JOINLOBBYFAILED)
    LABEL(XTransport::XTS_JOINLOBBYSUCCESS)
    LABEL(XTransport::XTS_LOBBY)
    LABEL(XTransport::XTS_CREATINGROOM)
    LABEL(XTransport::XTS_CREATEROOMERROR)
    LABEL(XTransport::XTS_CREATEROOMSUCCESS)
    LABEL(XTransport::XTS_CANJOINROOM)
    LABEL(XTransport::XTS_JOININGROOM)
    LABEL(XTransport::XTS_JOINROOMERROR)
    LABEL(XTransport::XTS_JOINROOMSUCCESS)
    LABEL(XTransport::XTS_ROOM)
    LABEL(XTransport::XTS_CREATINGGAME)
    LABEL(XTransport::XTS_CREATEGAMEERROR)
    LABEL(XTransport::XTS_CREATEGAMESUCCESS)
    LABEL(XTransport::XTS_CANJOINGAME)
    LABEL(XTransport::XTS_LEAVINGROOM)
    LABEL(XTransport::XTS_JOININGGAME)
    LABEL(XTransport::XTS_JOINGAMEERROR)
    LABEL(XTransport::XTS_JOINGAMESUCCESS)
    LABEL(XTransport::XTS_GAME)
ENDLABEL(XtsLabels)

} // namespace wi

#endif // __XTRANSPORT_H__
