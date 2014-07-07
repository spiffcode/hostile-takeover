#ifndef __BYTEBUFFER_H__
#define __BYTEBUFFER_H__

#include "inc/basictypes.h"
#include <arpa/inet.h>

namespace base {

static const int DEFAULT_SIZE = 128;

class ByteBuffer {
public:
    ByteBuffer(dword cb = DEFAULT_SIZE);
    ~ByteBuffer();

    const byte *Data() const { return bytes_ + start_; }
    int Length() const { return end_ - start_; }
    int Capacity() const { return size_ - start_; }
    void Rewind() { start_ = 0; }

    bool ReadByte(byte *pb);
    bool ReadWord(word *pw);
    bool ReadDword(dword *pdw);
    bool ReadString(char *psz, int cb);
    bool ReadBytes(byte *pb, int cb);

    void WriteByte(byte b);
    void WriteWord(word w);
    void WriteDword(dword dw);
    void WriteString(const char *psz, bool zero = true);
    void WriteBytes(const byte *pb, int cb);

    void *Strip(int *pcb = NULL);
    void Resize(int size);
    void Shift(int size);
    ByteBuffer *Clone() const;

    static word HostToNetWord(word w) {
        return htons(w);
    }
    static word NetToHostWord(word w) {
        return ntohs(w);
    }
    static dword HostToNetDword(dword dw) {
        return htonl(dw);
    }
    static dword NetToHostDword(dword dw) {
        return ntohl(dw);
    }

    ByteBuffer *pbbNext_;

private:
    byte *bytes_;
    int size_;
    int start_;
    int end_;
};

} // namespace base

#endif // __BYTEBUFFER_H__
