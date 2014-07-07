#include "base/log.h"
#include "base/thread.h"
#include "game/xtransport.h"
#include "game/serviceurls.h"
#include "game/chooseserverform.h"
#include "mpshared/xpump.h"

namespace wi {
    
//---------------------------------------------------------------------------
// XTransport implementation

XTransport::XTransport(const base::SocketAddress& address) : Transport(),
        address_(address), state_(XTS_CLOSED),
        waiting_(false),
        error_(false), id_(0), roomidCreate_(0), gameidCreate_(0),
        resultLogin_(knLoginResultFail),
        resultSignOut_(knSignOutResultSuccess),
        resultLobbyJoin_(knLobbyJoinResultFail),
        resultRoomJoin_(knRoomJoinResultFail),
        resultRoomCreate_(knLobbyCreateRoomResultFail),
        resultGameJoin_(knGameJoinResultFail),
        resultGameCreate_(knRoomCreateGameResultFail) {
}

XTransport::~XTransport() {
}

int XTransport::GetTransportDescriptions(TransportDescription *atrad,
        int ctradMax) {
    // This transport is opened directly, currently
#if 0
    TransportDescription *ptrad = atrad;
    ptrad->trat = ktratX;
    strcpy(ptrad->szName, "Connection");
    ptrad->pfnOpen = XTransport::Open;
    ptrad->dwTransportSpecific = 0;
    return 1;
#else
    return 0;
#endif
}

dword XTransport::Open(TransportDescription *ptrad, Transport **pptra) {
    // This transport is opened directly, currently
#if 0
    Transport *ptra = new XTransport(ptrad->dwTransportSpecific);
    if (ptra == NULL) {
        return knTransportOpenResultFail;
    }
    dword result = ptra->Open();
    if (result == knTransportOpenResultSuccess) {
        *pptra = ptra;
        return result;
    }
    delete ptra;
    return result;
#else
    return knTransportOpenResultFail;
#endif
}

void XTransport::SetState(State state) {
    LOG() << "From: " << XtsLabels.Find(state_)
            << " To: " << XtsLabels.Find(state);
    state_ = state;

    // If waiting for a state change, wake up
    if (waiting_) {
        base::Thread::current().Post(base::kidmNullEvent, NULL);
    }
}

bool XTransport::CheckState(State state0, State state1) {
    Assert(state0 == state_ || state1 == state_);
    if (state0 != state_ && state1 != state_) {
        LOG() << "Error! Current: " << XtsLabels.Find(state_);
        if (state1 == XTS_INVALID) {
            LOG() << " Expected: " << XtsLabels.Find(state0);
        } else {
            LOG() << " Expected: " << XtsLabels.Find(state0)
                    << " or " << XtsLabels.Find(state1);
        }
        return false;
    }
    return true;
}

dword XTransport::Open() {
    // The caller of Transport::Open expects a synchronous Open,
    // yet the connect and handshake need to stay responsive to UI
    // needs (like exit application, and timeout).
    error_ = false;
    base::Socket *sock = base::Thread::current().ss().CreateSocket(SOCK_STREAM,
            this);
    if (sock == NULL) {
        LOG() << "CreateSocket() failed";
        return knTransportOpenResultNoNetwork;
    }

    // Synchronous Connect to the server! There is "connecting..." UI
    // showing during this.
    if (sock->Connect(address_) < 0) {
        if (error_ || !sock->IsBlocking()) {
            LOG() << "Connect() failed";
            delete sock;
            return knTransportOpenResultCantConnect;
        }
    }
    SetState(XTS_CONNECTING);

    // Wait for the connect to occur with a modal loop that dispatches
    // nothing. If there is no connect after the timeout, assume error.

    LOG() << "Waiting to connect to " << address_.ToString();

    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting for connect";
    }
    if (state_ != XTS_CONNECTED) {
        LOG() << "Could not connect, closing socket";
        delete sock;
        SetState(XTS_CLOSED);
        return knTransportOpenResultNotResponding;
    }

    // From here on, the socket is owned by xpump.
    xpump_.Attach(sock, this);

    // Now send a handshake message, and await a reply before
    // returning
    if (!Handshake() || error_) {
        LOG() << "Handshake error";
        xpump_.Close();
        SetState(XTS_CLOSED);
        return knTransportOpenResultCantConnect;
    }

    // Await a proper reply. 
    LOG() << "Waiting for handshake reply";
    if (!WaitForStateChange()) {
        LOG() << "Timed out Waiting for handshake reply";
        xpump_.Close();
        SetState(XTS_CLOSED);
        return knTransportOpenResultNotResponding;
    }

    if (state_ == XTS_HANDSHAKEERROR) {
        LOG() << "Received handshake error";
        xpump_.Close();
        SetState(XTS_CLOSED);
        return knTransportOpenResultProtocolMismatch;
    }

    if (state_ == XTS_HANDSHAKESERVERFULL) {
        LOG() << "Received handshake server full.";
        xpump_.Close();
        SetState(XTS_CLOSED);
        return knTransportOpenResultServerFull;
    }

    // Now in opened state
    SetState(XTS_OPEN);
    LOG() << "XTransport successfully opened!";
    return knTransportOpenResultSuccess;
}

void XTransport::Close() {
    SetState(XTS_CLOSED);
    xpump_.Close();
}

bool XTransport::IsClosed() {
    return state_ == XTS_CLOSED;
}

bool XTransport::WaitForStateChange(long ctWait) {
    waiting_ = true;
    gtimm.Enable(false);
    long t = HostGetTickCount();
    int state = state_;
    while (state == state_) {
        if (error_) {
            return false;
        }
        if (HostGetTickCount() - t >= ctWait) {
            gtimm.Enable(true);
            waiting_ = false;
            return false;
        }
        Event evt;
        if (gevm.GetEvent(&evt, 50, false)) {
            if (gevm.IsAppStopping()) {
                gtimm.Enable(true);
                waiting_ = false;
                return false;
            }
        }
        OnEvent(&evt);
    }
    gtimm.Enable(true);
    waiting_ = false;
    return true;
}

bool XTransport::Handshake() {
    if (!CheckState(XTS_CONNECTED)) {
        return false;
    }
    xpump_.Send(XMsgHandshake::ToBuffer(kdwClientID, kdwProtocolCurrent));
    SetState(XTS_HANDSHAKING);
    return true;
}

void XTransport::OnHandshakeResult(dword result, dword id) {
    LOG() << HandshakeResults.Find(result);
    if (!CheckState(XTS_HANDSHAKING)) {
        return;
    }

    // These should be results not states, but not changing now...
    switch (result) {
    case knHandshakeResultSuccess:
        id_ = id;
        SetState(XTS_HANDSHAKESUCCESS);
        return;

    case knHandshakeResultServerFull:
        SetState(XTS_HANDSHAKESERVERFULL);
        return;

    default:
        SetState(XTS_HANDSHAKEERROR);
        return;
    }
}

void XTransport::OnShowMessage(const char *message, dword ipRedirect,
        bool disconnect) {
    if (error_ || xpump_.IsClosed()) {
        return;
    }
    // The server is asking to show a message. This is allowed during
    // any state.
    // TODO: show message
    // TODO: handle ipRedirect, disconnect
    if (m_ptcb != NULL) {
        if (strlen(message) != 0) {
            m_ptcb->OnShowMessage(message);
        }
    }
}

void XTransport::OnEcho() {
    xpump_.Send(XMsgEcho::ToBuffer());
}

dword XTransport::Login(const char *username, const char *token) {
    LOG() << "Logging in...";

    if (error_ || xpump_.IsClosed()) {
        return knLoginResultFail;
    }
    if (!CheckState(XTS_OPEN)) {
        return knLoginResultFail;
    }

    // Login and wait synchronously for reply.
    SetState(XTS_LOGGINGIN);
    xpump_.Send(XMsgLogin::ToBuffer(username, token, gszDeviceId));

    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting for Login";
        resultLogin_ = knLoginResultFail;
    }

    if (state_ != XTS_LOGGEDIN) {
        LOG() << LoginResults.Find(resultLogin_);
        SetState(XTS_OPEN);
        return resultLogin_;
    }

    // Login successful. Keep state XTS_LOGGEDIN.
    return resultLogin_;
}

void XTransport::OnLoginResult(dword resultLogin) {
    LOG() << LoginResults.Find(resultLogin);
    if (!CheckState(XTS_LOGGINGIN)) {
        resultLogin_ = knLoginResultFail;
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultLogin_ = knLoginResultFail;
        return;
    }
    resultLogin_ = resultLogin;

    if (resultLogin_ != knLoginResultSuccess &&
            resultLogin_ != knLoginResultAnonymousSuccess) {
        SetState(XTS_LOGINFAILED);
        return;
    }
    SetState(XTS_LOGGEDIN);
}

dword XTransport::SignOut() {
    LOG() << "Signing Out...";
    if (error_ || xpump_.IsClosed()) {
        return knSignOutResultFail;
    }

    // Only know how to do this from XTS_LOGGEDIN state, currently.
    if (!CheckState(XTS_LOGGEDIN)) {
        return knSignOutResultFail;
    }

    // Sign out and wait synchronously for reply.
    SetState(XTS_SIGNINGOUT);
    xpump_.Send(XMsgSignOut::ToBuffer());

    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting for sign out";
        resultSignOut_ = knSignOutResultFail;
    }

    if (state_ != XTS_SIGNEDOUT) {
        LOG() << SignOutResults.Find(resultSignOut_);
        SetState(XTS_LOGGEDIN);
        return resultSignOut_;
    }

    // Sign Out successful.
    SetState(XTS_OPEN);
    return resultSignOut_;
}

void XTransport::OnSignOutResult(dword result) {
    LOG() << SignOutResults.Find(result);
    if (!CheckState(XTS_SIGNINGOUT)) {
        resultSignOut_ = knSignOutResultFail;
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultSignOut_ = knSignOutResultFail;
        return;
    }
    resultSignOut_ = result;

    if (resultSignOut_ != knSignOutResultSuccess) {
        SetState(XTS_SIGNOUTFAILED);
        return;
    }
    SetState(XTS_SIGNEDOUT);
}

const char *XTransport::GetAnonymousUsername() {
    return base::Format::ToString("anon%d", id_);
}

dword XTransport::JoinLobby(ILobbyCallback *plcb) {
    if (error_ || xpump_.IsClosed()) {
        return knLobbyJoinResultFail;
    }
    if (!CheckState(XTS_LOGGEDIN)) {
        return knLobbyJoinResultFail;
    }

    xpump_.Send(XMsgLobbyJoin::ToBuffer());
    SetState(XTS_JOINLOBBY);

    LOG() << "Waiting for LobbyJoinResult";
    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting for LobbyJoinResult";
    }
    if (state_ != XTS_JOINLOBBYSUCCESS) {
        LOG() << "Could not enter lobby reason: "
                << LobbyJoinResults.Find(resultLobbyJoin_);
        SetState(XTS_LOGGEDIN);
        return resultLobbyJoin_;
    }
    SetState(XTS_LOBBY);
    m_plcb = plcb;
    return resultLobbyJoin_;
}

void XTransport::OnLobbyJoinResult(dword result) {
    LOG() << LobbyJoinResults.Find(result);
    if (error_ || xpump_.IsClosed()) {
        return;
    }
    if (!CheckState(XTS_JOINLOBBY)) {
        return;
    }
    resultLobbyJoin_ = result;
    if (result == knLobbyJoinResultSuccess) {
        SetState(XTS_JOINLOBBYSUCCESS);
        return;
    }
    SetState(XTS_JOINLOBBYFAILED);
}

void XTransport::OnLobbyAddRoom(const char *room, dword roomid, bool priv,
        dword cPlayers, dword cGames) {
    if (m_plcb != NULL) {
        m_plcb->OnAddRoom(room, roomid, priv, cPlayers, cGames);
    }
}

void XTransport::OnLobbyRemoveRoom(dword roomid) {
    if (m_plcb != NULL) {
        m_plcb->OnRemoveRoom(roomid);
    }
}

void XTransport::OnLobbyUpdateRoom(dword roomid, dword cPlayers, dword cGames) {
    if (m_plcb != NULL) {
        m_plcb->OnUpdateRoom(roomid, cPlayers, cGames);
    }
}

bool XTransport::LeaveLobby() {    
    m_plcb = NULL;
    if (error_ || xpump_.IsClosed()) {
        return false;
    }
    if (!CheckState(XTS_LOBBY)) {
        return false;
    }
    xpump_.Send(XMsgLobbyLeave::ToBuffer());
    SetState(XTS_LOGGEDIN);
    return true;
}

void XTransport::OnLobbyLeaveResult(dword result) {
}

dword XTransport::CreateRoom(const char *roomname, const char *password,
        dword *roomid) {
    LOG() << base::Format::ToString("CreateRoom: %s %s", roomname, password);

    if (error_ || xpump_.IsClosed()) {
        return knLobbyCreateRoomResultFail;
    }
    if (!CheckState(XTS_LOBBY)) {
        return knLobbyCreateRoomResultFail;
    }

    SetState(XTS_CREATINGROOM);
    xpump_.Send(XMsgLobbyCreateRoom::ToBuffer(roomname, password));

    // Wait for reply

    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting room create";
        resultRoomCreate_ = knLobbyCreateRoomResultFail;
    }

    if (state_ != XTS_CREATEROOMSUCCESS) {
        LOG() << LobbyCreateRoomResults.Find(resultRoomCreate_);
        SetState(XTS_LOBBY);
        return resultRoomCreate_;
    }

    *roomid = roomidCreate_;
    SetState(XTS_LOBBY);
    return resultRoomCreate_;
}

void XTransport::OnLobbyCreateRoomResult(dword result, dword roomid) {
    LOG() << LobbyCreateRoomResults.Find(result);
    if (!CheckState(XTS_CREATINGROOM)) {
        resultRoomCreate_ = knLobbyCreateRoomResultFail;
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultRoomCreate_ = knLobbyCreateRoomResultFail;
        return;
    }
    resultRoomCreate_ = result;
    roomidCreate_ = roomid;
    if (resultRoomCreate_ != knLobbyCreateRoomResultSuccess) {
        SetState(XTS_CREATEROOMERROR);
        return;
    }
    SetState(XTS_CREATEROOMSUCCESS);
}

dword XTransport::CanJoinRoom(dword roomid, const char *password) {
    if (error_ || xpump_.IsClosed()) {
        return knRoomJoinResultFail;
    }
    if (!CheckState(XTS_LOBBY)) {
        return knRoomJoinResultFail;
    }

    SetState(XTS_CANJOINROOM);
    xpump_.Send(XMsgLobbyCanJoinRoom::ToBuffer(roomid, password));

    // Wait for the reply

    if (!WaitForStateChange()) {
        LOG() << "Timed out can wait joining room";
        resultRoomJoin_ = knRoomJoinResultFail;
    }
    SetState(XTS_LOBBY);
    return resultRoomJoin_;
}

void XTransport::OnLobbyCanJoinRoomResult(dword result) {
    if (!CheckState(XTS_CANJOINROOM)) {
        return;
    }
    resultRoomJoin_ = result;
    SetState(XTS_LOBBY);
}

void XTransport::OnLobbyLurkerCount(dword count) {
    if (m_plcb != NULL) {
        m_plcb->OnLurkerCount(count);
    }
}

dword XTransport::JoinRoom(dword roomid, const char *password,
        IRoomCallback *prcb) {
    // Synchronously join a room. It may fail.

    if (error_ || xpump_.IsClosed()) {
        return knRoomJoinResultFail;
    }
    if (!CheckState(XTS_LOGGEDIN)) {
        return knRoomJoinResultFail;
    }

    SetState(XTS_JOININGROOM);
    xpump_.Send(XMsgRoomJoin::ToBuffer(roomid, password));

    // Wait for the reply

    if (!WaitForStateChange()) {
        LOG() << "Timed out waiting room join";
        resultRoomJoin_ = knRoomJoinResultFail;
    }

    if (state_ != XTS_JOINROOMSUCCESS) {
        LOG() << RoomJoinResults.Find(resultRoomJoin_);
        SetState(XTS_LOGGEDIN);
        return resultRoomJoin_;
    }

    // Entering a new room; clear the games being tracked

    m_prcb = prcb;
    SetState(XTS_ROOM);
    return resultRoomJoin_;
}

void XTransport::OnRoomJoinResult(dword result) {
    LOG() << RoomJoinResults.Find(result);
    if (!CheckState(XTS_JOININGROOM)) {
        resultRoomJoin_ = knRoomJoinResultFail;
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultRoomJoin_ = knRoomJoinResultFail;
        return;
    }
    resultRoomJoin_ = result;
    if (resultRoomJoin_ != knRoomJoinResultSuccess) {
        SetState(XTS_JOINROOMERROR);
        return;
    }
    SetState(XTS_JOINROOMSUCCESS);
}

void XTransport::OnRoomAddPlayer(const char *player) {
    if (m_prcb != NULL) {
        m_prcb->OnAddPlayer(player);
    }
}

void XTransport::OnRoomRemovePlayer(dword hint, const char *player) {
    if (m_prcb != NULL) {
        m_prcb->OnRemovePlayer(hint, player);
    }
}

bool XTransport::SendChat(const char *chat) {
    if (error_ || xpump_.IsClosed()) {
        return false;
    }
    if (state_ == XTS_ROOM) {
        xpump_.Send(XMsgRoomSendChat::ToBuffer(chat));
        return true;
    }
    if (state_ == XTS_GAME) {
        xpump_.Send(XMsgGameSendChat::ToBuffer(chat));
        return true;
    }
    Assert();
    return false;
} 

void XTransport::OnRoomReceiveChat(const char *player, const char *chat) {
    if (m_prcb != NULL) {
        m_prcb->OnReceiveChat(player, chat);
    }
}

void XTransport::OnRoomAddGame(const char *player, dword gameid,
        const GameParams& params, dword minplayers, dword maxplayers,
        const char *title, dword ctotal) {
    if (m_prcb != NULL) {
        m_prcb->OnAddGame(player, gameid, params, minplayers, maxplayers,
                title, ctotal);
    }
}

void XTransport::OnRoomRemoveGame(dword gameid, dword ctotal) {
    if (m_prcb != NULL) {
        m_prcb->OnRemoveGame(gameid, ctotal);
    }
}

void XTransport::OnRoomGameInProgress(dword gameid) {
    if (m_prcb != NULL) {
        m_prcb->OnGameInProgress(gameid);
    }
}

void XTransport::OnRoomGamePlayerNames(dword gameid, dword cnames,
        const PlayerName *anames) {
    if (m_prcb != NULL) {
        m_prcb->OnGamePlayerNames(gameid, cnames, anames);
    }
}

void XTransport::OnRoomStatusComplete() {
    if (m_prcb != NULL) {
        m_prcb->OnStatusComplete();
    }
}

dword XTransport::CanJoinGame(dword gameid) {
    if (error_ || xpump_.IsClosed()) {
        return knRoomJoinResultFail;
    }
    if (!CheckState(XTS_ROOM)) {
        return knGameJoinResultFail;
    }

    SetState(XTS_CANJOINGAME);
    xpump_.Send(XMsgRoomCanJoinGame::ToBuffer(gameid));

    // Wait for the reply

    if (!WaitForStateChange()) {
        LOG() << "Timed out can wait joining game";
        resultGameJoin_ = knGameJoinResultFail;
    }
    SetState(XTS_ROOM);
    return resultRoomJoin_;
}

void XTransport::OnRoomCanJoinGameResult(dword result) {
    if (!CheckState(XTS_CANJOINGAME)) {
        return;
    }
    resultGameJoin_ = result;
    SetState(XTS_ROOM);
}

void XTransport::SetGameCountStatus(dword ctotal) {
    if (m_ptcb != NULL) {
        char szT[64];
        if (ctotal == 1) {
            sprintf(szT, "%d game.", (int)ctotal);
        } else {
            sprintf(szT, "%d games.", (int)ctotal);
        }
        m_ptcb->OnStatusUpdate(szT);
    }
}

dword XTransport::CreateGame(GameParams *params, PackId *ppackidUpgrade,
        dword *gameid) {
    if (error_ || xpump_.IsClosed()) {
        LOG() << "Error or closed";
        return knRoomCreateGameResultFail;
    }
    if (!CheckState(XTS_ROOM)) {
        return knRoomCreateGameResultFail;
    }
    xpump_.Send(XMsgRoomCreateGame::ToBuffer(params));

    // Wait synchronously for the result, since the UI needs it.
    SetState(XTS_CREATINGGAME);
    if (!WaitForStateChange()) {
        LOG() << "timed out waiting for XMsgRoomCreateGameResult";
        SetState(XTS_ROOM);
        return knRoomCreateGameResultFail;
    }

    if (!CheckState(XTS_CREATEGAMEERROR, XTS_CREATEGAMESUCCESS)) {
        // Don't know what state it is, so don't state transition
        LOG() << "expected CREATEGAMESUCCESS or ERROR state";
        return knRoomCreateGameResultFail;
    }

    if (state_ == XTS_CREATEGAMEERROR) {
        *ppackidUpgrade = packidCreate_;
        SetState(XTS_ROOM);
        return resultGameCreate_;
    }

    SetState(XTS_ROOM);
    *gameid = gameidCreate_;
    return resultGameCreate_;
}

void XTransport::OnRoomCreateGameResult(dword gameid, dword result,
        const PackId *ppackid) {
    LOG() << CreateGameResults.Find(result);
    if (!CheckState(XTS_CREATINGGAME)) {
        resultGameCreate_ = knRoomCreateGameResultFail;
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultGameCreate_ = knRoomCreateGameResultFail;
        return;
    }

    // Remember these for later
    resultGameCreate_ = result;
    packidCreate_ = *ppackid;
    gameidCreate_ = gameid;

    if (result != knRoomCreateGameResultSuccess) {
        SetState(XTS_CREATEGAMEERROR);
        return;
    }
    SetState(XTS_CREATEGAMESUCCESS);
}

void XTransport::LeaveRoom(dword hint) {
    m_prcb = NULL;
    if (error_ || xpump_.IsClosed()) {
        return;
    }
    if (!CheckState(XTS_ROOM)) {
        return;
    }
    SetState(XTS_LEAVINGROOM);
    xpump_.Send(XMsgRoomLeave::ToBuffer(hint));
    WaitForStateChange();
    SetState(XTS_LOGGEDIN);
}

void XTransport::OnRoomLeaveResult() {
    if (error_ || xpump_.IsClosed()) {
        return;
    }
    if (!CheckState(XTS_LEAVINGROOM)) {
        return;
    }
    SetState(XTS_LOGGEDIN);
}

dword XTransport::JoinGame(dword gameid, dword roomid) {
    if (error_ || xpump_.IsClosed()) {
        LOG() << "Error or closed";
        return knGameJoinResultFail;
    }

    // Must be in XTS_LOGGEDIN state, not in room, lobby, or game.
    if (!CheckState(XTS_LOGGEDIN)) {
        return knGameJoinResultFail;
    }

    // Wait synchronously for the result
    SetState(XTS_JOININGGAME);
    xpump_.Send(XMsgGameJoin::ToBuffer(gameid, roomid));
    if (!WaitForStateChange()) {
        LOG() << "timed out waiting for XMsgGameJoinResult";
        SetState(XTS_LOGGEDIN);
        return knGameJoinResultFail;
    }

    if (!CheckState(XTS_JOINGAMEERROR, XTS_JOINGAMESUCCESS)) {
        // Don't know what state it is, so don't state transition
        LOG() << "expected JOINGAMESUCCESS or ERROR state";
        return knGameJoinResultFail;
    }

    if (state_ == XTS_JOINGAMEERROR) {
        SetState(XTS_LOGGEDIN);
        return resultGameJoin_;
    }

    SetState(XTS_GAME);
    return resultGameJoin_;
}

void XTransport::OnGameJoinResult(dword result) {
    LOG() << GameJoinResults.Find(result);
    if (!CheckState(XTS_JOININGGAME)) {
        return;
    }
    if (error_ || xpump_.IsClosed()) {
        resultGameJoin_ = knGameJoinResultFail;
        return;
    }

    resultGameJoin_ = result;

    if (result != knGameJoinResultSuccess) {
        LOG() << "Error connecting to game.";
        SetState(XTS_JOINGAMEERROR);
        return;
    }

    SetState(XTS_JOINGAMESUCCESS);
}

void XTransport::OnGameReceiveChat(const char *player, const char *chat) {
    if (m_pgcb != NULL) {
        m_pgcb->OnReceiveChat(player, chat);
    }
}

bool XTransport::SendNetMessage(NetMessage *pnm) {
    if (error_ || xpump_.IsClosed()) {
        return false;
    }
    if (!CheckState(XTS_GAME)) {
        return false;
    }
    return xpump_.Send(XMsgGameNetMessage::ToBuffer(pnm));
}

void XTransport::OnGameNetMessage(NetMessage **ppnm) {
    if (!CheckState(XTS_GAME)) {
        return;
    }
    if (m_pgcb != NULL) {
        m_pgcb->OnNetMessage(ppnm);
    }
}

void XTransport::OnGameKilled(dword gameid) {
    if (!CheckState(XTS_GAME)) {
        return;
    }

    // The server removed this client from the game, so this client's
    // state is no longer GAME.
    SetState(XTS_LOGGEDIN);

    if (m_pgcb != NULL) {
        m_pgcb->OnGameDisconnect();
    }
}

void XTransport::LeaveGame() {
    if (error_ || xpump_.IsClosed()) {
        LOG() << "Error or closed";
        return;
    }

    // If the game was killed by the server, the state gets reverted to
    // XTS_LOGGEDIN. In this case, the client may call LeaveGame anyway,
    // so ignore this case without an assert.
    if (state_ == XTS_LOGGEDIN) {
        return;
    }

    // The client should bin the game
    if (!CheckState(XTS_GAME)) {
        LOG() << "Not in game state!";
        return;
    }

    // Disconnect this game synchronously. This is so that a game
    // in progress doesn't continue to send updates when the client
    // thinks it is disconnected.

    xpump_.Send(XMsgGameLeave::ToBuffer());
    if (!WaitForStateChange()) {
        LOG() << "timed out waiting for disconnect game result";
    }

    // Go to XTS_LOGGEDIN, whether it was successful or not
    SetState(XTS_LOGGEDIN);
}

void XTransport::OnGameLeaveResult(dword result) {
    LOG() << GameLeaveResults.Find(result);
    SetState(XTS_LOGGEDIN);
}

void XTransport::OnEvent(Event *pevt) {
    if (xpump_.Dispatch()) {
        OnMessages();
    }
}

bool XTransport::OnMessages() {
    // Pump the message loop for each xmsg. Important since xmsg's change
    // state, and message loops evaluate state.
    base::Thread::current().Post(base::kidmTransportEvent, NULL);
    return true;
}

void XTransport::OnError(int error) {
    LOG() << base::Socket::GetErrorString(error);
    error_ = true;
    OnClose(0);
}

void XTransport::OnClose(int error) {
    LOG() << error;
    if (m_ptcb != NULL) {
        m_ptcb->OnConnectionClose();
    }
    SetState(XTS_CLOSED);

    // Cause a message to be pumped through the game input loop to cause it
    // to wake up, since processing here was done in MessageQueue dispatch.

    base::Thread::current().Post(base::kidmNullEvent, NULL);
}

void XTransport::OnConnectEvent(base::Socket *socket) {
    SetState(XTS_CONNECTED);
}

void XTransport::OnReadEvent(base::Socket *socket) {
}

void XTransport::OnWriteEvent(base::Socket *socket) {
}

void XTransport::OnCloseEvent(base::Socket *socket) {
}

} // namespace wi
