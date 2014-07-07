#ifndef __MESSAGES_H__
#define __MESSAGES_H__

#include "base/bytebuffer.h"
#include "mpshared/xmsg.h"
#include "mpshared/netmessage.h"
#include "mpshared/constants.h"
#include "mpshared/misc.h"
#include "base/misc.h"

namespace wi {

const dword knLobbyCreateRoomResultSuccess = 0;
const dword knLobbyCreateRoomResultFail = 1;
const dword knLobbyCreateRoomResultFull = 2;
const dword knLobbyCreateRoomResultExists = 3;

STARTLABEL(LobbyCreateRoomResults)
    LABEL(knLobbyCreateRoomResultSuccess)
    LABEL(knLobbyCreateRoomResultFail)
    LABEL(knLobbyCreateRoomResultFull)
    LABEL(knLobbyCreateRoomResultExists)
ENDLABEL(LobbyCreateRoomResults)

const dword knHandshakeResultSuccess = 0;
const dword knHandshakeResultFail = 1;
const dword knHandshakeResultServerFull = 2;

STARTLABEL(HandshakeResults)
    LABEL(knHandshakeResultSuccess)
    LABEL(knHandshakeResultFail)
    LABEL(knHandshakeResultServerFull)
ENDLABEL(HandshakeResults)

const dword knGameJoinResultSuccess = 0;
const dword knGameJoinResultFail = 1;
const dword knGameJoinResultRoomNotFound = 2;
const dword knGameJoinResultGameNotFound = 3;
const dword knGameJoinResultGameFull = 4;
const dword knGameJoinResultInProgress = 5;

STARTLABEL(GameJoinResults)
    LABEL(knGameJoinResultSuccess)
    LABEL(knGameJoinResultFail)
    LABEL(knGameJoinResultGameNotFound)
    LABEL(knGameJoinResultRoomNotFound)
    LABEL(knGameJoinResultGameFull)
    LABEL(knGameJoinResultInProgress)
ENDLABEL(GameJoinResults)

const dword knGameLeaveResultSuccess = 0;
const dword knGameLeaveResultFail = 1;
const dword knGameLeaveResultNotFound = 2;

STARTLABEL(GameLeaveResults)
    LABEL(knGameLeaveResultSuccess)
    LABEL(knGameLeaveResultFail)
    LABEL(knGameLeaveResultNotFound)
ENDLABEL(GameLeaveResults)

const dword knRoomCreateGameResultSuccess = 0;
const dword knRoomCreateGameResultFail = 1;
const dword knRoomCreateGameResultUnknownMissionPack = 2;
const dword knRoomCreateGameResultUpgradeMissionPack = 3;
const dword knRoomCreateGameResultRoomFull = 4;

STARTLABEL(CreateGameResults)
    LABEL(knRoomCreateGameResultSuccess)
    LABEL(knRoomCreateGameResultFail)
    LABEL(knRoomCreateGameResultUnknownMissionPack)
    LABEL(knRoomCreateGameResultUpgradeMissionPack)
    LABEL(knRoomCreateGameResultRoomFull)
ENDLABEL(CreateGameResults)

const dword knLoginResultSuccess = 0;
const dword knLoginResultAnonymousSuccess = 1;
const dword knLoginResultFail = 2;
const dword knLoginResultStaleToken = 3;
const dword knLoginResultAuthDown = 4;
const dword knLoginResultNoPassword = 5;

STARTLABEL(LoginResults)
    LABEL(knLoginResultSuccess)
    LABEL(knLoginResultAnonymousSuccess)
    LABEL(knLoginResultFail)
    LABEL(knLoginResultStaleToken)
    LABEL(knLoginResultAuthDown)
    LABEL(knLoginResultNoPassword)
ENDLABEL(LoginResults)

const dword knSignOutResultSuccess = 0;
const dword knSignOutResultFail = 1;

STARTLABEL(SignOutResults)
    LABEL(knSignOutResultSuccess)
    LABEL(knSignOutResultFail)
ENDLABEL(SignOutResults)

const dword knRoomJoinResultSuccess = 0;
const dword knRoomJoinResultFail = 1;
const dword knRoomJoinResultFull = 2;
const dword knRoomJoinResultNotFound = 3;
const dword knRoomJoinResultWrongPassword = 4;

STARTLABEL(RoomJoinResults)
    LABEL(knRoomJoinResultSuccess)
    LABEL(knRoomJoinResultFail)
    LABEL(knRoomJoinResultFull)
    LABEL(knRoomJoinResultNotFound)
    LABEL(knRoomJoinResultWrongPassword)
ENDLABEL(RoomJoinResults)

const dword knRoomLeaveResultSuccess = 0;
const dword knRoomLeaveResultFail = 1;

STARTLABEL(RoomLeaveResults)
    LABEL(knRoomLeaveResultSuccess)
    LABEL(knRoomLeaveResultFail)
ENDLABEL(RoomLeaveResults)

const dword knLobbyJoinResultSuccess = 0;
const dword knLobbyJoinResultFail = 1;
const dword knLobbyJoinResultNotLoggedIn = 2;
const dword knLobbyJoinResultFull = 3;

STARTLABEL(LobbyJoinResults)
    LABEL(knLobbyJoinResultSuccess)
    LABEL(knLobbyJoinResultFail)
    LABEL(knLobbyJoinResultNotLoggedIn)
    LABEL(knLobbyJoinResultFull)
ENDLABEL(LobbyJoinResults)

const dword knLobbyLeaveResultSuccess = 0;
const dword knLobbyLeaveResultFail = 1;

STARTLABEL(LobbyLeaveResults)
    LABEL(knLobbyLeaveResultSuccess)
    LABEL(knLobbyLeaveResultFail)
ENDLABEL(LobbyLeaveResults)

typedef XMsg2<XMSG_HANDSHAKE> XMsgHandshake;
typedef XMsg2<XMSG_HANDSHAKERESULT> XMsgHandshakeResult;
typedef XMsg0<XMSG_ECHO> XMsgEcho;
typedef XMsg1<XMSG_PROTOCOLERROR> XMsgProtocolError;
typedef XMsgS3<XMSG_LOGIN, kcbUsernameMax, kcbTokenMax, kcbDidMax> XMsgLogin;
typedef XMsg1<XMSG_LOGINRESULT> XMsgLoginResult;
typedef XMsg0<XMSG_SIGNOUT> XMsgSignOut;
typedef XMsg1<XMSG_SIGNOUTRESULT> XMsgSignOutResult;
typedef XMsg0<XMSG_LOBBYJOIN> XMsgLobbyJoin;
typedef XMsg1<XMSG_LOBBYJOINRESULT> XMsgLobbyJoinResult;
typedef XMsgS2<XMSG_LOBBYCREATEROOM, kcbRoomname, kcbPassword> XMsgLobbyCreateRoom;
typedef XMsg2<XMSG_LOBBYCREATEROOMRESULT> XMsgLobbyCreateRoomResult;
typedef XMsgDS<XMSG_LOBBYCANJOINROOM, kcbPassword> XMsgLobbyCanJoinRoom;
typedef XMsg1<XMSG_LOBBYCANJOINROOMRESULT> XMsgLobbyCanJoinRoomResult;
typedef XMsg1<XMSG_LOBBYLURKERCOUNT> XMsgLobbyLurkerCount;
typedef XMsg1<XMSG_LOBBYREMOVEROOM> XMsgLobbyRemoveRoom;
typedef XMsg3<XMSG_LOBBYUPDATEROOM> XMsgLobbyUpdateRoom;
typedef XMsg0<XMSG_LOBBYLEAVE> XMsgLobbyLeave;
typedef XMsg1<XMSG_LOBBYLEAVERESULT> XMsgLobbyLeaveResult;
typedef XMsgDS<XMSG_ROOMJOIN, kcbPassword> XMsgRoomJoin;
typedef XMsg1<XMSG_ROOMJOINRESULT> XMsgRoomJoinResult;
typedef XMsgS1<XMSG_ROOMADDPLAYER, kcbPlayerName> XMsgRoomAddPlayer;
typedef XMsgDS<XMSG_ROOMREMOVEPLAYER, kcbPlayerName> XMsgRoomRemovePlayer;
typedef XMsgS1<XMSG_ROOMSENDCHAT, kcbChatMax> XMsgRoomSendChat;
typedef XMsgS2<XMSG_ROOMRECEIVECHAT, kcbPlayerName, kcbChatMax> XMsgRoomReceiveChat;
typedef XMsg2<XMSG_ROOMREMOVEGAME> XMsgRoomRemoveGame;
typedef XMsg1<XMSG_ROOMGAMEINPROGRESS> XMsgRoomGameInProgress;
typedef XMsg0<XMSG_ROOMSTATUSCOMPLETE> XMsgRoomStatusComplete;
typedef XMsg1<XMSG_ROOMCANJOINGAME> XMsgRoomCanJoinGame;
typedef XMsg1<XMSG_ROOMCANJOINGAMERESULT> XMsgRoomCanJoinGameResult;
typedef XMsg1<XMSG_ROOMLEAVE> XMsgRoomLeave;
typedef XMsg1<XMSG_ROOMLEAVERESULT> XMsgRoomLeaveResult;
typedef XMsg2<XMSG_GAMEJOIN> XMsgGameJoin;
typedef XMsg1<XMSG_GAMEJOINRESULT> XMsgGameJoinResult;
typedef XMsgS1<XMSG_GAMESENDCHAT, kcbChatMax> XMsgGameSendChat;
typedef XMsgS2<XMSG_GAMERECEIVECHAT, kcbPlayerName, kcbChatMax> XMsgGameReceiveChat;
typedef XMsg1<XMSG_GAMEKILLED> XMsgGameKilled;
typedef XMsg0<XMSG_GAMELEAVE> XMsgGameLeave;
typedef XMsg1<XMSG_GAMELEAVERESULT> XMsgGameLeaveResult;

struct XMsgShowMessage : public XMsg {
    XMsgShowMessage(const char *message, dword ipRedirect, bool disconnect) :
            XMsg(XMSG_SHOWMESSAGE) {
        message_ = AllocString(message);
        ipRedirect_ = ipRedirect;
        disconnect_ = disconnect;
    }
    ~XMsgShowMessage() {
        delete message_;
    }
    const char *message_;
    dword ipRedirect_;
    bool disconnect_;
#ifdef LOGGING
    virtual std::string ToString();
#endif
    static base::ByteBuffer *ToBuffer(const char *message, dword ipRedirect,
            bool disconnect);
    static XMsgShowMessage *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_SHOWMESSAGE_FIXED = XMSGSIZE_FIXED + sizeof(dword) * 3;

struct XMsgLobbyAddRoom : public XMsg {
    XMsgLobbyAddRoom(const char *room, dword roomid, bool priv,
            dword cPlayers, dword cGames) : XMsg(XMSG_LOBBYADDROOM) {
        room_ = AllocString(room);
        roomid_ = roomid;
        priv_ = priv;
        cPlayers_ = cPlayers;
        cGames_ = cGames;
    }
    ~XMsgLobbyAddRoom() {
        delete room_;
    }
    const char *room_;
    dword roomid_;
    bool priv_;
    dword cPlayers_;
    dword cGames_;
#ifdef LOGGING
    virtual std::string ToString();
#endif
    static base::ByteBuffer *ToBuffer(const char *room, dword roomid,
            bool priv, dword cPlayers, dword cGames);
    static XMsgLobbyAddRoom *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_LOBBYADDROOM = XMSGSIZE_FIXED + kcbRoomname +
        sizeof(dword) * 4;

struct XMsgRoomAddGame : public XMsg
{
    XMsgRoomAddGame(const char *player, dword gameid, const GameParams& params,
            int minplayers, int maxplayers, const char *title, dword ctotal) :
            XMsg(XMSG_ROOMADDGAME), gameid_(gameid), params_(params),
            minplayers_(minplayers), maxplayers_(maxplayers), ctotal_(ctotal) {
        strncpyz(player_, player, sizeof(player_));
        strncpyz(title_, title, sizeof(title_));
    }

    char player_[kcbPlayerName];
    dword ctotal_;
    dword gameid_;
    dword minplayers_;
    dword maxplayers_;
    char title_[kcbLevelTitle];
    GameParams params_;

    static base::ByteBuffer *ToBuffer(const char *player, dword gameid,
            const GameParams& params, dword minplayers, dword maxplayers,
            const char *title, dword ctotal);
    static XMsgRoomAddGame *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_ROOMADDGAME = XMSGSIZE_FIXED + kcbPlayerName +
        sizeof(dword) * 2 + sizeof(int) * 2 + kcbLevelTitle +
        sizeof(GameParams);

struct XMsgRoomGamePlayerNames : public XMsg
{
    XMsgRoomGamePlayerNames(dword gameid, dword cnames,
            PlayerName *anames) : XMsg(XMSG_ROOMGAMEPLAYERNAMES),
            gameid_(gameid), cnames_(cnames), anames_(anames) { }
    virtual ~XMsgRoomGamePlayerNames() { delete anames_; }
    dword gameid_;
    dword cnames_;
    PlayerName *anames_;

    static base::ByteBuffer *ToBuffer(dword gameid, dword cnames,
            const PlayerName *anames);
    static XMsgRoomGamePlayerNames *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_ROOMGAMEPLAYERNAMES_FIXED = XMSGSIZE_FIXED +
        sizeof(dword) * 2;

struct XMsgRoomCreateGame : public XMsg {
    XMsgRoomCreateGame(GameParams *params) : XMsg(XMSG_ROOMCREATEGAME) {
        params_ = *params;
    }
    GameParams params_;
#ifdef LOGGING
    virtual std::string ToString();
#endif
    static base::ByteBuffer *ToBuffer(GameParams *params);
    static XMsgRoomCreateGame *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_ROOMCREATEGAME = XMSGSIZE_FIXED + sizeof(GameParams);

struct XMsgRoomCreateGameResult : public XMsg {
    XMsgRoomCreateGameResult(dword gameid, dword result,
            const PackId *ppackid) : XMsg(XMSG_ROOMCREATEGAMERESULT) {
        gameid_ = gameid;
        result_= result;
        packid_ = *ppackid;
    }
    dword gameid_;
    dword result_;
    PackId packid_;
    static base::ByteBuffer *ToBuffer(dword gameid, dword result,
            const PackId *ppackid);
    static XMsgRoomCreateGameResult *FromBuffer(base::ByteBuffer& bb,
            dword cb);
};
const dword XMSGSIZE_ROOMCREATEGAMERESULT = XMSGSIZE_FIXED +
        sizeof(dword) * 2 + sizeof(PackId);

struct XMsgGameNetMessage : public XMsg
{
    XMsgGameNetMessage(NetMessage *pnm) : XMsg(XMSG_GAMENETMESSAGE) {
        pnm_ = pnm;
    }
    virtual ~XMsgGameNetMessage() {
        delete pnm_;
    }
    NetMessage *pnm_;
#ifdef LOGGING
    virtual std::string ToString();
#endif
    static base::ByteBuffer *ToBuffer(NetMessage *pnm);
    static XMsgGameNetMessage *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_GAMENETMESSAGE_FIXED = XMSGSIZE_FIXED;

struct XMsgGameUpdateEmpty : public XMsg {
    //long cUpdatesBlock;
    //long cUpdatesSync;

    static base::ByteBuffer *ToBuffer(UpdateNetMessage *punm);
    static XMsgGameNetMessage *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_GAMEUPDATEEMPTY = XMSGSIZE_FIXED + sizeof(dword) * 2;

struct XMsgGameUpdateResult : public XMsg {
    //long cUpdatesBlock;
    //dword hash;
    //short cmsLatency;

    static base::ByteBuffer *ToBuffer(UpdateResultNetMessage *purnm);
    static XMsgGameNetMessage *FromBuffer(base::ByteBuffer& bb, dword cb);
};
const dword XMSGSIZE_GAMEUPDATERESULT = XMSGSIZE_FIXED + sizeof(dword) +
        sizeof(dword) + sizeof(short);

} // namespace wi

#endif // __MESSAGES_H__
