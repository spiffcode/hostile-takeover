#include "mpshared/xmsglog.h"
#include "mpshared/xpump.h"
#include "base/tick.h"
#include "base/log.h"
#include <time.h>

namespace wi {

XMsgLog::XMsgLog(const std::string& filepath) {
    file_ = NULL;

    file_ = fopen(filepath.c_str(), "w");
    if (file_ == NULL) {
        return;
    }

    time_t t = time(NULL);
    struct tm *tm = localtime(&t); 

    LogHeader header;
    header.marker0 = 0xffffffff;
    header.marker1 = 0xffffffff;
    header.month = tm->tm_mon;
    header.dayofmonth = tm->tm_mday;
    header.hour = tm->tm_hour;
    header.minute = tm->tm_min;
    header.second = tm->tm_sec;
    
    size_t c = fwrite(&header, sizeof(header), 1, file_);
    if (c != 1) {
        Close();
    }
}

void XMsgLog::Log(const base::ByteBuffer& bb, int cb, dword from, dword to) {
    Log(bb, from, to, base::GetMillisecondCount(), cb);
}


void XMsgLog::Log(const base::ByteBuffer& bb, dword from, dword to, long64 ts,
        int cb) {
    if (file_ == NULL) {
        return;
    }
    LogItem item;
    item.marker = 0xeaeaeaea;
    item.ts = ts;
    item.from = from;
    item.to = to;

    size_t c = fwrite(&item, sizeof(item), 1, file_);
    if (c != 1) {
        Close();
        return;
    }

    if (cb == -1) {
        cb = bb.Length();
    }

    c = fwrite(bb.Data(), cb, 1, file_);
    if (c != 1) {
        Close();
        return;
    }
}

void XMsgLog::Flush() {
    if (file_ != NULL) {
        fflush(file_);
    }
}

void XMsgLog::Close() {
    if (file_ != NULL) {
        fclose(file_);
        file_ = NULL;
    }
}

bool XMsgLog::ReadHeader(FILE *file, LogHeader *plh) {
    size_t c = fread(plh, sizeof(*plh), 1, file);
    if (c != 1) {
        LOG() << "error reading logfile";
        return false;
    }

    if (plh->marker0 != 0xffffffff || plh->marker1 != 0xffffffff) {
        LOG() << "something wrong with log header";
        fclose(file);
        return false;
    }
    return true;
}

XMsg *XMsgLog::GetNextXMsg(FILE *file, LogItem *pli) {
    base::ByteBuffer *pbb = GetNextByteBuffer(file, pli);
    if (pbb != NULL) {
        XMsg *pxmsg = XPump::XMsgFromBuffer(*pbb, pbb->Length());
        delete pbb;
        return pxmsg;
    }
    return NULL;
}

base::ByteBuffer *XMsgLog::GetNextByteBuffer(FILE *file, LogItem *pli) {
    size_t c = fread(pli, sizeof(*pli), 1, file);
    if (c != 1) {
        return NULL;
    }

    dword cb;
    c = fread(&cb, sizeof(cb), 1, file);
    if (c != 1) {
        LOG() << "size read error!";
        return NULL;
    }
    cb = base::ByteBuffer::NetToHostDword(cb);
    base::ByteBuffer *pbb = new base::ByteBuffer(cb);
    if (pbb == NULL) {
        return NULL;
    }
    pbb->WriteDword(cb);
    cb -= sizeof(cb);

    while (cb != 0) {
        byte ab[4 * 1024];
        int cbRead = sizeof(ab);
        if (cb < cbRead) {
            cbRead = cb;
        }
        c = fread(ab, 1, cbRead, file);
        if (c != cbRead) {
            LOG() << "read error!";
            delete pbb;
            return NULL;
        }
        pbb->WriteBytes(ab, c);
        cb -= c;
    }

    return pbb;
}

} // namespace wi
