#include "mpshared/messages.h"
#include "mpshared/netmessage.h"
#include "base/socketaddress.h"
#include <limits.h>

namespace wi {

#ifdef LOGGING
std::string XMsgShowMessage::ToString() {
    std::ostringstream ss;
    ss << XMsgLabels.Find(id_) << " message: " << message_ <<
            " address: " << base::SocketAddress(ipRedirect_, 0).ToString() <<
            " disconnect: " << (disconnect_ ? "true" : "false");
    return ss.str();
}
#endif

base::ByteBuffer *XMsgShowMessage::ToBuffer(const char *message,
        dword ipRedirect, bool disconnect) {
    int cbMessage = strlen(message) + 1;
    char messageT[kcbShowMessageMax];
    if (cbMessage > sizeof(messageT)) {
        strncpyz(messageT, message, sizeof(messageT));
        message = messageT;
        cbMessage = sizeof(messageT);
    }
    dword cb = XMSGSIZE_SHOWMESSAGE_FIXED + cbMessage;
    base::ByteBuffer *bb = new base::ByteBuffer(cb);
    bb->WriteWord(cb);
    bb->WriteByte(XMSG_SHOWMESSAGE);
    bb->WriteDword(ipRedirect);
    bb->WriteDword(disconnect ? 1 : 0);
    bb->WriteDword(cbMessage);
    bb->WriteBytes((const byte *)message, cbMessage);
    if (bb->Length() != cb) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgShowMessage *XMsgShowMessage::FromBuffer(base::ByteBuffer& bb,
        dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w || w < XMSGSIZE_SHOWMESSAGE_FIXED) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_SHOWMESSAGE) {
        return NULL;
    }
    dword ipRedirect = 0;
    bb.ReadDword(&ipRedirect);
    dword disconnect = 0;
    bb.ReadDword(&disconnect);
    dword cbMessage = 0;
    bb.ReadDword(&cbMessage);
    if (cbMessage > kcbShowMessageMax) {
        Assert();
        return NULL;
    }
    char message[kcbShowMessageMax];
    bb.ReadBytes((byte *)message, cbMessage);
    message[cbMessage - 1] = 0;
    if (cbSav - bb.Length() != w) {
        Assert();
        return NULL;
    }

    return new XMsgShowMessage(message, ipRedirect, disconnect != 0);
}

#ifdef LOGGING
std::string XMsgRoomCreateGame::ToString() {
    std::ostringstream ss;
    ss << XMsgLabels.Find(id_) << " GameParams: " <<
            GameParamsToString(params_);
    return ss.str();
}
#endif

base::ByteBuffer *XMsgRoomCreateGame::ToBuffer(GameParams *params) {
    base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_ROOMCREATEGAME);
    bb->WriteWord(XMSGSIZE_ROOMCREATEGAME);
    bb->WriteByte(XMSG_ROOMCREATEGAME);
    GameParams paramsT = *params;
    SwapGameParams(&paramsT);
    bb->WriteBytes((const byte *)&paramsT, sizeof(paramsT));
    Assert(bb->Length() == XMSGSIZE_ROOMCREATEGAME);
    if (bb->Length() != XMSGSIZE_ROOMCREATEGAME) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgRoomCreateGame *XMsgRoomCreateGame::FromBuffer(base::ByteBuffer& bb,
        dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_ROOMCREATEGAME) {
        return NULL;
    }
    GameParams params;
    bb.ReadBytes((byte *)&params, sizeof(GameParams));
    SwapGameParams(&params);
    if (!ValidateGameParams(params)) {
        return NULL;
    }
    Assert(cbSav - bb.Length() == XMSGSIZE_ROOMCREATEGAME);
    if (cbSav - bb.Length() != XMSGSIZE_ROOMCREATEGAME) {
        return NULL;
    }

    return new XMsgRoomCreateGame(&params);
}

base::ByteBuffer *XMsgRoomCreateGameResult::ToBuffer(dword gameid,
        dword result, const PackId *ppackid) {
    base::ByteBuffer *bb = new base::ByteBuffer(
            XMSGSIZE_ROOMCREATEGAMERESULT);
    bb->WriteWord(XMSGSIZE_ROOMCREATEGAMERESULT);
    bb->WriteByte(XMSG_ROOMCREATEGAMERESULT);
    bb->WriteDword(gameid);
    bb->WriteDword(result);
    PackId packidT;
    if (ppackid == NULL) {
        memset(&packidT, 0, sizeof(packidT));
        ppackid = &packidT;
    }
    bb->WriteDword(ppackid->id);
    bb->WriteBytes(ppackid->hash, sizeof(ppackid->hash));
    Assert(bb->Length() == XMSGSIZE_ROOMCREATEGAMERESULT);
    if (bb->Length() != XMSGSIZE_ROOMCREATEGAMERESULT) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgRoomCreateGameResult *XMsgRoomCreateGameResult::FromBuffer(
        base::ByteBuffer& bb, dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_ROOMCREATEGAMERESULT) {
        return NULL;
    }
    dword gameid;
    bb.ReadDword(&gameid);
    dword result;
    bb.ReadDword(&result);
    PackId packid;
    bb.ReadDword(&packid.id);
    bb.ReadBytes(packid.hash, sizeof(packid.hash));

    Assert(cbSav - bb.Length() == XMSGSIZE_ROOMCREATEGAMERESULT);
    if (cbSav - bb.Length() != XMSGSIZE_ROOMCREATEGAMERESULT) {
        return NULL;
    }

    return new XMsgRoomCreateGameResult(gameid, result, &packid);
}

#ifdef LOGGING
std::string XMsgGameNetMessage::ToString() {
    std::ostringstream ss;
    ss << XMsgLabels.Find(id_) << ", " << PszFromNetMessage(pnm_);
    return ss.str();
}
#endif

base::ByteBuffer *XMsgGameNetMessage::ToBuffer(NetMessage *pnm) {
    // Special case very common messages, to make them as small as possible
    if (pnm->nmid == knmidScUpdate) {
        UpdateNetMessage *punm = (UpdateNetMessage *)pnm;
        if (punm->cmsgCommands == 0) {
            return XMsgGameUpdateEmpty::ToBuffer(punm);
        }
    }
    if (pnm->nmid == knmidCsUpdateResult) {
        return XMsgGameUpdateResult::ToBuffer((UpdateResultNetMessage *)pnm);
    }

    // Slim the generic NetMessage to essential elements.

    word cb = XMSGSIZE_GAMENETMESSAGE_FIXED; // fixed overhead of this xmsg
    cb += 1; // nmid
    word cbInside = pnm->cb - sizeof(NetMessage);
    cb += cbInside;
    byte *pbInside = (byte *)pnm + sizeof(NetMessage);

    base::ByteBuffer *bb = new base::ByteBuffer(cb);
    bb->WriteWord(cb);
    bb->WriteByte(XMSG_GAMENETMESSAGE);
    Assert(bb->Length() == XMSGSIZE_GAMENETMESSAGE_FIXED);
    if (bb->Length() != XMSGSIZE_GAMENETMESSAGE_FIXED) {
        delete bb;
        return NULL;
    }

    bb->WriteByte(pnm->nmid);
    NetMessageByteOrderSwap(pnm, true);
    bb->WriteBytes(pbInside, cbInside);
    NetMessageByteOrderSwap(pnm, false);

    Assert(bb->Length() == cb);
    if (bb->Length() != cb) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgGameNetMessage *XMsgGameNetMessage::FromBuffer(base::ByteBuffer& bb,
        dword cb) {
    dword cbSav = bb.Length();
    word cbMsg;
    if (!bb.ReadWord(&cbMsg)) {
        return NULL;
    }
    if (cb != (dword)cbMsg) {
        Assert();
        return NULL;
    }
    if (cbMsg - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_GAMENETMESSAGE) {
        return NULL;
    }
    byte nmid = 0;
    bb.ReadByte(&nmid);

    word cbInside = cbMsg - XMSGSIZE_GAMENETMESSAGE_FIXED - 1;
    word cbNmsg = cbInside + sizeof(NetMessage);
    
    byte *pb = new byte[cbNmsg];
    memset(pb, 0, cbNmsg);
    NetMessage *pnm = (NetMessage *)pb;
    pnm->cb = base::ByteBuffer::HostToNetWord(cbNmsg);
    pnm->nmid = base::ByteBuffer::HostToNetWord(nmid);
    bb.ReadBytes(pb + sizeof(NetMessage), cbInside);
    NetMessageByteOrderSwap(pnm, false);

    Assert(cbSav - bb.Length() == cbMsg);
    if (cbSav - bb.Length() != cbMsg) {
        delete pb;
        return NULL;
    }

    // TODO: Validate size and content of NetMessage

    return new XMsgGameNetMessage(pnm);
}

base::ByteBuffer *XMsgGameUpdateEmpty::ToBuffer(UpdateNetMessage *punm) {
    base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_GAMEUPDATEEMPTY);
    bb->WriteWord(XMSGSIZE_GAMEUPDATEEMPTY);
    bb->WriteByte(XMSG_GAMEUPDATEEMPTY);
    bb->WriteDword(punm->cUpdatesBlock);
    bb->WriteDword(punm->cUpdatesSync);
    if (bb->Length() != XMSGSIZE_GAMEUPDATEEMPTY) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgGameNetMessage *XMsgGameUpdateEmpty::FromBuffer(base::ByteBuffer& bb,
        dword cb) {
    dword cbSav = bb.Length();
    word cbMsg;
    if (!bb.ReadWord(&cbMsg)) {
        return NULL;
    }
    if (cb != (dword)cbMsg || cbMsg != XMSGSIZE_GAMEUPDATEEMPTY) {
        Assert();
        return NULL;
    }
    if (cbMsg - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_GAMEUPDATEEMPTY) {
        return NULL;
    }
    dword cUpdatesBlock = 0;
    bb.ReadDword(&cUpdatesBlock);
    dword cUpdatesSync = 0;
    bb.ReadDword(&cUpdatesSync);
    Assert(cbSav - bb.Length() == XMSGSIZE_GAMEUPDATEEMPTY);
    if (cbSav - bb.Length() != XMSGSIZE_GAMEUPDATEEMPTY) {
        return NULL;
    }
   
    int cbUnm = sizeof(UpdateNetMessage) - sizeof(Message); 
    UpdateNetMessage *punm = (UpdateNetMessage *)new byte[cbUnm];
    punm->nmid = knmidScUpdate;
    punm->cb = cbUnm;
    punm->cUpdatesBlock = cUpdatesBlock;
    punm->cUpdatesSync = cUpdatesSync;
    punm->cmsgCommands = 0;
    return new XMsgGameNetMessage(punm);
}

base::ByteBuffer *XMsgGameUpdateResult::ToBuffer(
        UpdateResultNetMessage *purnm) {
    base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_GAMEUPDATERESULT);
    bb->WriteWord(XMSGSIZE_GAMEUPDATERESULT);
    bb->WriteByte(XMSG_GAMEUPDATERESULT);
    bb->WriteDword(purnm->ur.cUpdatesBlock);
    bb->WriteDword(purnm->ur.hash);
    if (purnm->ur.cmsLatency > SHRT_MAX) {
        bb->WriteWord(SHRT_MAX);
    } else if (purnm->ur.cmsLatency < SHRT_MIN) {
        bb->WriteWord((word)SHRT_MIN);
    } else {
        bb->WriteWord((short)purnm->ur.cmsLatency);
    }
    if (bb->Length() != XMSGSIZE_GAMEUPDATERESULT) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgGameNetMessage *XMsgGameUpdateResult::FromBuffer(base::ByteBuffer& bb,
        dword cb) {
    dword cbSav = bb.Length();
    word cbMsg;
    if (!bb.ReadWord(&cbMsg)) {
        return NULL;
    }
    if (cb != (dword)cbMsg || cbMsg != XMSGSIZE_GAMEUPDATERESULT) {
        Assert();
        return NULL;
    }
    if (cbMsg - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_GAMEUPDATERESULT) {
        return NULL;
    }
    dword cUpdatesBlock = 0;
    bb.ReadDword(&cUpdatesBlock);
    dword hash = 0;
    bb.ReadDword(&hash);
    short cmsLatency = 0;
    bb.ReadWord((word *)&cmsLatency);
    
    Assert(cbSav - bb.Length() == XMSGSIZE_GAMEUPDATERESULT);
    if (cbSav - bb.Length() != XMSGSIZE_GAMEUPDATERESULT) {
        return NULL;
    }
  
    UpdateResultNetMessage *purnm = new UpdateResultNetMessage;
    purnm->ur.cUpdatesBlock = cUpdatesBlock;
    purnm->ur.hash = hash;
    purnm->ur.cmsLatency = cmsLatency;
    return new XMsgGameNetMessage(purnm);
}

base::ByteBuffer *XMsgRoomAddGame::ToBuffer(const char *player, dword gameid,
        const GameParams& params, dword minplayers, dword maxplayers,
        const char *title, dword ctotal) {
    base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_ROOMADDGAME);
    bb->WriteWord(XMSGSIZE_ROOMADDGAME);
    bb->WriteByte(XMSG_ROOMADDGAME);
    bb->WriteDword(ctotal);
    bb->WriteDword(gameid);
    bb->WriteDword(minplayers);
    bb->WriteDword(maxplayers);
    char playerT[kcbPlayerName];
    memset(playerT, 0, sizeof(playerT));
    strncpyz(playerT, player, sizeof(playerT));
    bb->WriteBytes((const byte *)playerT, sizeof(playerT));
    char titleT[kcbLevelTitle];
    memset(titleT, 0, sizeof(titleT));
    strncpyz(titleT, title, sizeof(titleT));
    bb->WriteBytes((const byte *)titleT, sizeof(titleT));
    GameParams paramsT = params;
    SwapGameParams(&paramsT);
    bb->WriteBytes((const byte *)&paramsT, sizeof(paramsT));
    if (bb->Length() != XMSGSIZE_ROOMADDGAME) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgRoomAddGame *XMsgRoomAddGame::FromBuffer(base::ByteBuffer& bb, dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w || w != XMSGSIZE_ROOMADDGAME) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_ROOMADDGAME) {
        return NULL;
    }
    dword ctotal;
    bb.ReadDword(&ctotal);
    dword gameid;
    bb.ReadDword(&gameid);
    dword minplayers;
    bb.ReadDword(&minplayers);
    dword maxplayers;
    bb.ReadDword(&maxplayers);
    char player[kcbPlayerName];
    bb.ReadBytes((byte *)player, sizeof(player));
    player[sizeof(player) - 1] = 0;
    char title[kcbLevelTitle];
    bb.ReadBytes((byte *)title, sizeof(title));
    title[sizeof(title) - 1] = 0;
    GameParams params;
    bb.ReadBytes((byte *)&params, sizeof(params));
    SwapGameParams(&params);
    if (!ValidateGameParams(params)) {
        return NULL;
    }
    Assert(cbSav - bb.Length() == XMSGSIZE_ROOMADDGAME);
    if (cbSav - bb.Length() != XMSGSIZE_ROOMADDGAME) {
        return NULL;
    }
    return new XMsgRoomAddGame(player, gameid, params, minplayers, maxplayers,
            title, ctotal);
}

base::ByteBuffer *XMsgRoomGamePlayerNames::ToBuffer(dword gameid, dword cnames,
        const PlayerName *anames) {
    dword cb = XMSGSIZE_ROOMGAMEPLAYERNAMES_FIXED + cnames * sizeof(PlayerName);
    base::ByteBuffer *bb = new base::ByteBuffer(cb);
    bb->WriteWord(cb);
    bb->WriteByte(XMSG_ROOMGAMEPLAYERNAMES);
    bb->WriteDword(gameid);
    bb->WriteDword(cnames);
    bb->WriteBytes((const byte *)anames, cnames * sizeof(PlayerName));
    if (bb->Length() != cb) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgRoomGamePlayerNames *XMsgRoomGamePlayerNames::FromBuffer(
        base::ByteBuffer& bb, dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_ROOMGAMEPLAYERNAMES) {
        return NULL;
    }
    dword gameid;
    bb.ReadDword(&gameid);
    dword cnames;
    bb.ReadDword(&cnames);
    PlayerName *anames = new PlayerName[cnames];
    bb.ReadBytes((byte *)anames, cnames * sizeof(PlayerName));

    if (cbSav - bb.Length() != w) {
        Assert();
        delete anames;
        return NULL;
    }

    return new XMsgRoomGamePlayerNames(gameid, cnames, anames);
}

#ifdef LOGGING
std::string XMsgLobbyAddRoom::ToString() {
    std::ostringstream ss;
    ss << XMsgLabels.Find(id_) << " room: " << room_ << " roomid: " <<
            roomid_ << " priv: " << priv_ << " cPlayers: " << cPlayers_ <<
            " cGames: " << cGames_;
    return ss.str();
}
#endif

base::ByteBuffer *XMsgLobbyAddRoom::ToBuffer(const char *room, dword roomid,
        bool priv, dword cPlayers, dword cGames) {
    dword cb = XMSGSIZE_LOBBYADDROOM;
    base::ByteBuffer *bb = new base::ByteBuffer(cb);
    bb->WriteWord(cb);
    bb->WriteByte(XMSG_LOBBYADDROOM);
    char roomT[kcbRoomname];
    memset(roomT, 0, sizeof(roomT));
    strncpyz(roomT, room, sizeof(roomT));
    bb->WriteBytes((const byte *)roomT, sizeof(roomT));
    bb->WriteDword(roomid);
    bb->WriteDword(priv ? 1 : 0);
    bb->WriteDword(cPlayers);
    bb->WriteDword(cGames);
    if (bb->Length() != cb) {
        delete bb;
        return NULL;
    }
    return bb;
}

XMsgLobbyAddRoom *XMsgLobbyAddRoom::FromBuffer(base::ByteBuffer& bb, dword cb) {
    dword cbSav = bb.Length();
    word w;
    if (!bb.ReadWord(&w)) {
        return NULL;
    }
    if (cb != (dword)w) {
        Assert();
        return NULL;
    }
    if (w - sizeof(word) > bb.Length()) {
        return NULL;
    }
    byte id = 0;
    bb.ReadByte(&id);
    if (id != XMSG_LOBBYADDROOM) {
        return NULL;
    }
    char room[kcbRoomname];
    bb.ReadBytes((byte *)room, sizeof(room));
    dword roomid;
    bb.ReadDword(&roomid);
    dword v;
    bb.ReadDword(&v);
    bool priv = (v != 0);
    dword cPlayers;
    bb.ReadDword(&cPlayers);
    dword cGames;
    bb.ReadDword(&cGames);

    if (cbSav - bb.Length() != w) {
        Assert();
        return NULL;
    }

    return new XMsgLobbyAddRoom(room, roomid, priv, cPlayers, cGames);
}

} // namespace wi
