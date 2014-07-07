#include "server/logger.h"
#include "server/endpoint.h"
#include "server/server.h"
#include <sys/stat.h>
#include <errno.h>

namespace wi {

#define MSG_LOGENTRY 1

Logger::Logger(const char  *pszDir, dword server_id) :
        base::MessageHandler(worker_), dir_(pszDir), pf_(NULL),
        rotating_(false) {

    char szPrefix[64];
    sprintf(szPrefix, "server-%lu-%lu", base::GetSecondsUnixEpocUTC(), server_id);
    prefix_ = szPrefix;
    worker_.Start(this, &Logger::ThreadStart);
}

Logger::~Logger() {
    // Thread::Stop gets called in the Thread destructor, which does a join
    // with the actual thread, which synchronizes exiting
}

void Logger::ThreadStart(void *pv) {
    // Start logging
    OpenLog();

    // Run and process messages. RunLoop will return when this thread exits
    // When worker_ destructs, queued base::MessageData objects get destroyed
    worker_.RunLoop();

    // Close files
    CloseLog();
}

std::string Logger::GetLogFilename() {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char szFilename[PATH_MAX];
    sprintf(szFilename, "%s/%s_%04d-%02d-%02d.log", dir_.c_str(),
            prefix_.c_str(), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return std::string(szFilename);
}

bool Logger::OpenLog() {
    // Create log dir if it doesn't exist
    struct stat st;
    if (stat(dir_.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            // Dir doesn't exist; Assume one level directory to keep this easy
            if (mkdir(dir_.c_str(), 0755) != 0) {
                return false;
            }
        } else {
            return false;
        }
    } else {
        // Ensure it is a directory
        if (!(st.st_mode & S_IFDIR)) {
            return false;
        }
    }

    // Close it if it is open... shouldn't be the case but be safe
    if (pf_ != NULL) {
        CloseLog();
    }

    // Attempt open for writing. Allow append to the end of an existing file
    std::string filename = GetLogFilename();
    pf_ = fopen(filename.c_str(), "ab");
    if (pf_ == NULL) {
        return false;
    }
    filenameLast_ = filename;

    // Called on worker thread so handle directly
    LogEntry entry(knLogEntryTypeLogger);
    entry.message_ = "OPEN LOG";
    WriteLogEntry(entry);
    return true;
}

void Logger::CloseLog() {
    // Called on worker thread so handle directly
    LogEntry entry(knLogEntryTypeLogger);
    entry.message_ = "CLOSE LOG";
    WriteLogEntry(entry);

    // Close log
    if (pf_ != NULL) {
        fclose(pf_);
        pf_ = NULL;
    }
}

void Logger::RotateLog() {
    if (!rotating_) {
        std::string filename = GetLogFilename();
        if (filenameLast_.compare(filename) != 0) {
            rotating_ = true;
            CloseLog();
            OpenLog();
            rotating_ = false;
        }
    }
}

void Logger::Submit(const LogEntry& entry) {
    // Called on any thread, post to worker thread
    worker_.Post(MSG_LOGENTRY, this, new LogEntryData(entry));
}

void Logger::InitLogEntryFromEndpoint(Endpoint *endpoint, LogEntry &entry) {
    entry.gameid_ = endpoint->gameid();
    strncpyz(entry.did_, endpoint->did(), sizeof(entry.did_));
    entry.ip_ = endpoint->xpump().socket()->GetRemoteAddress().ip();
    entry.anon_ = endpoint->anonymous();
    entry.game_name_ = endpoint->GetGameName();
    entry.player_name_ = endpoint->name();
    entry.mod_ = endpoint->IsModerator();
    entry.admin_ = endpoint->IsAdmin();

    Room *room = endpoint->server().lobby().FindRoom(endpoint->roomid());
    if (room != NULL) {
        entry.roomid_ = endpoint->roomid();
        entry.room_name_ = room->name();
        entry.private_room_ = (room->password()[0] != 0);
    }
}

void Logger::Log(LogEntryType type, Endpoint *endpoint, const char *pszMsg) {
    LogEntry entry(type);
    InitLogEntryFromEndpoint(endpoint, entry);
    entry.message_ = pszMsg;
    Submit(entry);
}

void Logger::LogRoomChat(Endpoint *endpoint, dword roomid, const char *pszRoomName, bool private_room, const char *pszMsg) {
    if (strlen(pszMsg) != 0) {
        LogEntry entry(knLogEntryTypeChat);
        if (endpoint != NULL) {
            InitLogEntryFromEndpoint(endpoint, entry);
        }
        entry.roomid_ = roomid;
        entry.room_name_ = pszRoomName;
        entry.private_room_ = private_room;
        entry.message_ = pszMsg;
        Submit(entry);
    }
}

void Logger::LogGameChat(Endpoint *endpoint, dword roomid, const char *pszRoomName, bool private_room, dword gameid, const char *pszGameName, const char *pszMsg) {
    if (strlen(pszMsg) != 0) {
        // TODO: Put roomid / room name in here
        LogEntry entry(knLogEntryTypeChat);
        if (endpoint != NULL) {
            InitLogEntryFromEndpoint(endpoint, entry);
        }
        entry.roomid_ = roomid;
        entry.room_name_ = pszRoomName;
        entry.private_room_ = private_room;
        entry.gameid_ = gameid;
        entry.game_name_ = pszGameName;
        entry.message_ = pszMsg;
        Submit(entry);
    }
}

void Logger::OnMessage(base::Message *pmsg) {
    // Worker thread message handler
    if (pmsg->id == MSG_LOGENTRY) {
        LogEntryData *data = (LogEntryData *)pmsg->data;
        WriteLogEntry(data->entry_);
        delete data;
    }
}

bool Logger::WriteLogEntry(const LogEntry& entry) {
    // Executes on worker thread

    // Check for log rotation
    RotateLog();

    // If there is no open log file, fail
    if (pf_ == NULL) {
        return false;
    }

    // Serialize the LogEntry
    base::ByteBuffer bb;
    entry.Serialize(bb);

    // Write to the log file and flush immediately 
    base::ByteBuffer bbT;
    bbT.WriteDword(bb.Length());
    bbT.WriteBytes(bb.Data(), bb.Length());
    if (fwrite(bbT.Data(), bbT.Length(), 1, pf_) != 1) {
        return false;
    }
    fflush(pf_);
    return true;
}

} // namespace wi
