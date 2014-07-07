#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "base/thread.h"
#include "base/messagequeue.h"
#include "base/bytebuffer.h"
#include "base/tick.h"
#include <string.h>
#include <string>

namespace wi {

enum LogEntryType {
    knLogEntryTypeNone,
    knLogEntryTypeLogger,
    knLogEntryTypeChat,
    knLogEntryTypeModCommand,
    knLogEntryTypeMark,
    knLogEntryTypeSystemMsg,
    knLogEntryTypeRoomInfo,
    knLogEntryTypeGameInfo
};

#define knLogEntryVersion 1

struct LogEntry {
    LogEntry(LogEntryType type) : version_(knLogEntryVersion), type_(type),
            time_(base::GetSecondsUnixEpocUTC()), roomid_(0), gameid_(0),
            ip_(0), anon_(false), private_room_(false), mod_(false),
            admin_(false) {
        memset(did_, 0, sizeof(did_));
    }

    void Serialize(base::ByteBuffer& bb) const {
        bb.WriteByte(version_);
        bb.WriteByte(type_);
        bb.WriteByte(anon_);
        bb.WriteByte(private_room_);
        bb.WriteByte(mod_);
        bb.WriteByte(admin_);
        bb.WriteDword(time_);
        bb.WriteDword(roomid_);
        bb.WriteDword(gameid_);
        bb.WriteDword(ip_);
        bb.WriteString(did_);
        bb.WriteString(room_name_.c_str());
        bb.WriteString(game_name_.c_str());
        bb.WriteString(player_name_.c_str());
        bb.WriteString(message_.c_str());
    }

    bool Unserialize(base::ByteBuffer& bb) {
        byte bT;
        if (!bb.ReadByte(&bT)) {
            return false;
        }
        version_ = bT;

        if (!bb.ReadByte(&bT)) {
            return false;
        }
        type_ = (LogEntryType)bT;

        if (!bb.ReadByte(&bT)) {
            return false;
        }
        anon_ = (bT != 0);

        if (!bb.ReadByte(&bT)) {
            return false;
        }
        private_room_ = (bT != 0);

        if (!bb.ReadByte(&bT)) {
            return false;
        }
        mod_ = (bT != 0);

        if (!bb.ReadByte(&bT)) {
            return false;
        }
        admin_ = (bT != 0);

        if (!bb.ReadDword(&time_)) {
            return false;
        }

        if (!bb.ReadDword(&roomid_)) {
            return false;
        }

        if (!bb.ReadDword(&gameid_)) {
            return false;
        }

        if (!bb.ReadDword(&ip_)) {
            return false;
        }

        if (!bb.ReadString(did_, sizeof(did_))) {
            return false;
        }

        char szT[2048];
        if (!bb.ReadString(szT, sizeof(szT))) {
            return false;
        }
        room_name_ = szT;

        if (!bb.ReadString(szT, sizeof(szT))) {
            return false;
        }
        game_name_ = szT;

        if (!bb.ReadString(szT, sizeof(szT))) {
            return false;
        }
        player_name_ = szT;

        if (!bb.ReadString(szT, sizeof(szT))) {
            return false;
        }
        message_ = szT;

        return true;
    }
        
    dword version_;
    LogEntryType type_;
    dword time_;
    dword roomid_;
    dword gameid_;
    char did_[64];
    dword ip_;
    bool anon_;
    bool private_room_;
    bool mod_;
    bool admin_;
    std::string room_name_;
    std::string game_name_;
    std::string player_name_;
    std::string message_;
};

struct LogEntryData : base::MessageData {
    LogEntryData(const LogEntry& entry) : entry_(entry) { }
    LogEntry entry_;
};

class Endpoint;
class Logger : base::MessageHandler {
public:
    Logger(const char *pszDir, dword server_id);
    ~Logger();

    void Log(LogEntryType type, Endpoint *endpoint, const char *pszMsg);
    void LogMark(Endpoint *endpoint, const char *pszMsg) {
        Log(knLogEntryTypeMark, endpoint, pszMsg);
    }
    void LogModCommand(Endpoint *endpoint, const char *pszMsg) {
        Log(knLogEntryTypeModCommand, endpoint, pszMsg);
    }
    void LogSystemMsg(Endpoint *endpoint, const char *pszMsg) {
        if (strlen(pszMsg) != 0) {
            Log(knLogEntryTypeSystemMsg, endpoint, pszMsg);
        }
    }
    void LogRoomChat(Endpoint *endpoint, dword roomid, const char *pszRoomName, bool private_room, const char *pszMsg);
    void LogGameChat(Endpoint *endpoint, dword roomid, const char *pszRoomName, bool private_room, dword gameid, const char *pszGameName, const char *pszMsg);
    void Submit(const LogEntry& entry);

private:
    void ThreadStart(void *pv);
    bool WriteLogEntry(const LogEntry& entry);
    std::string GetLogFilename();
    bool OpenLog();
    void CloseLog();
    void RotateLog();
    void InitLogEntryFromEndpoint(Endpoint *endpoint, LogEntry &entry);

    // MessageHandler interface
    virtual void OnMessage(base::Message *pmsg);

    bool rotating_;
    std::string dir_;
    std::string prefix_;
    base::Thread worker_;
    FILE *pf_;
    std::string filenameLast_;
};

} // namespace wi

#endif // __LOGGER_H__
