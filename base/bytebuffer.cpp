#include "bytebuffer.h"
#include <string.h>
#include <sys/param.h>
#include <stdlib.h>

namespace base {

ByteBuffer::ByteBuffer(dword cb) {
    start_ = 0;
    end_ = 0;
    size_ = cb;
    bytes_ = (byte *)malloc(size_);
    pbbNext_ = NULL;
}

ByteBuffer::~ByteBuffer() {
    if (bytes_ != NULL) {
        free(bytes_);
    }
}

bool ByteBuffer::ReadByte(byte *pb) {
    return ReadBytes(pb, sizeof(byte));
}

bool ByteBuffer::ReadWord(word *pw) {
    word w;
    if (!ReadBytes((byte *)&w, sizeof(word))) {
        return false;
    } else {
        *pw = NetToHostWord(w);
        return true;
    }
}

bool ByteBuffer::ReadDword(dword *pdw) {
    dword dw;
    if (!ReadBytes((byte *)&dw, 4)) {
        return false;
    } else {
        *pdw = NetToHostDword(dw);
        return true;
    }
}

bool ByteBuffer::ReadString(char *psz, int cb) {
    byte *pbStart = bytes_ + start_;
    byte *pbEnd = bytes_ + MIN(cb, end_ - start_);
    byte *pb = pbStart;
    for (; pb < pbEnd; pb++) {
        if (*pb == 0) {
            break;
        }
    }

    // Include the zero termination if found
    if (*pb == 0) {
        if (!ReadBytes((byte *)psz, pb - pbStart)) {
            return false;
        }
    } else {
        // Ensure there is room for a forced zero, and
        // only read the # of bytes that fit.
        if (cb > 0) {
            if (pb == bytes_ + cb) {
                pb--;
            }
            if (!ReadBytes((byte *)psz, pb - pbStart)) {
                return false;
            }
            psz[cb - 1] = 0;
        }
    }
    return true;
}

bool ByteBuffer::ReadBytes(byte *pb, int cb) {
    if (cb > Length()) {
        return false;
    } else {
        memcpy(pb, bytes_ + start_, cb);
        start_ += cb;
        return true;
    }
}

void ByteBuffer::WriteByte(byte b) {
    WriteBytes(&b, sizeof(byte));
}

void ByteBuffer::WriteWord(word w) {
    w = HostToNetWord(w);
    WriteBytes((byte *)&w, sizeof(word));
}

void ByteBuffer::WriteDword(dword dw) {
    dw = HostToNetDword(dw);
    WriteBytes((byte *)&dw, sizeof(dword));
}

void ByteBuffer::WriteString(const char *psz, bool zero) {
    int cch = strlen(psz);
    int cbWrite = cch + (zero ? 1 : 0);
    WriteBytes((const byte *)psz, cbWrite);
}

void ByteBuffer::WriteBytes(const byte *pb, int cb) {
    Shift(0);
    if (Length() + cb > Capacity()) {
        Resize(Length() + cb);
    }
    memcpy(bytes_ + end_, pb, cb);
    end_ += cb;
}

void ByteBuffer::Resize(int size) {
    if (size > size_)
        size = MAX(size, 3 * size_ / 2);

    int len = MIN(end_ - start_, size);
    byte* new_bytes = (byte *)malloc(size);
    memcpy(new_bytes, bytes_ + start_, len);
    free(bytes_);

    start_ = 0;
    end_ = len;
    size_ = size;
    bytes_ = new_bytes;
}

void ByteBuffer::Shift(int size) {
    // Move start_ to the beginning. Either bytes
    // have to be copied or Length() is zero and they don't need to be copied.

    dword len = Length();
    if (len == 0) {
        start_ = 0;
        end_ = 0;
    }
    if (size == 0 && start_ == 0) {
        return;
    }

    // Does two things: eats bytes from the current buffer window,
    // and moves the window to the beginning

    if (size > len) {
        return;
    }
    end_ = Length() - size;
    memmove(bytes_, bytes_ + start_ + size, end_);
    start_ = 0;
}

ByteBuffer *ByteBuffer::Clone() const {
    ByteBuffer *bbT = new ByteBuffer(Length());
    bbT->WriteBytes(Data(), Length());
    return bbT;
}

void *ByteBuffer::Strip(int *pcb) {
    Shift(0);
    void *pv = bytes_;
    if (pcb != NULL) {
        *pcb = Length();
    }

    start_ = 0;
    end_ = 0;
    size_ = DEFAULT_SIZE;
    bytes_ = (byte *)malloc(size_);
    return pv;
}
   
} // namespace base
