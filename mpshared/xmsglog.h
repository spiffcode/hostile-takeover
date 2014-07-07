#ifndef __XMSGLOG_H__
#define __XMSGLOG_H__

#include "inc/basictypes.h"
#include "base/bytebuffer.h"
#include "mpshared/xmsg.h"
#include <stdio.h>
#include <string>

namespace wi {

struct LogHeader {
    dword marker0;
    dword marker1;
    dword month;
    dword dayofmonth;
    dword hour;
    dword minute;
    dword second;
}; 

struct LogItem {
    dword marker;
    dword from;
    dword to;
    long64 ts;
}; 

class XMsgLog
{
public:
    XMsgLog(const std::string& filepath);
    ~XMsgLog() { Close(); }

    void Log(const base::ByteBuffer& bb, int cb, dword from, dword to);
    void Log(const base::ByteBuffer& bb, dword from, dword to, long64 ts,
            int cb = -1);
    void Flush();
    void Close();

    static bool ReadHeader(FILE *file, LogHeader *plh);
    static XMsg *GetNextXMsg(FILE *file, LogItem *pli);
    static base::ByteBuffer *GetNextByteBuffer(FILE *file, LogItem *pli);

private:
    FILE *file_;
};

} // namespace wi

#endif // __XMSGLOG_H__
