#ifndef __XMSG_H__
#define __XMSG_H__

#include "inc/basictypes.h"
#include "inc/rip.h"
#include "base/bytebuffer.h"
#include "base/misc.h"
#include "base/log.h"
#include "mpshared/constants.h"
#include "mpshared/misc.h"
#include <string.h>

namespace wi {

// byte count, id
const dword XMSGSIZE_SIZE = sizeof(word);
const dword XMSGSIZE_ID = sizeof(byte);
const dword XMSGSIZE_FIXED = XMSGSIZE_SIZE + XMSGSIZE_ID;

struct XMsg
{
    XMsg(byte id) {
        id_ = id;
    }
    virtual ~XMsg() {}
    byte id_;

    static bool ExtractSize(base::ByteBuffer& bb, dword *pcb) {
        if (bb.Length() < XMSGSIZE_SIZE) {
            return false;
        }
        if (pcb != NULL) {
            word cb;
            memcpy(&cb, bb.Data(), sizeof(cb));
            *pcb = (dword)base::ByteBuffer::NetToHostWord(cb);
        }
        return true;
    }

    static bool ExtractId(base::ByteBuffer& bb, dword *pid) {
        if (bb.Length() < XMSGSIZE_SIZE + XMSGSIZE_ID) {
            return false;
        }
        byte idT;
        memcpy(&idT, bb.Data() + XMSGSIZE_SIZE, sizeof(idT));
        *pid = (dword)idT;
        return true;
    }

    static base::ByteBuffer *ToBuffer(dword id, dword cb) {
        base::ByteBuffer *bb = new base::ByteBuffer(cb);
        bb->WriteWord((word)cb);
        bb->WriteByte((byte)id);
        Assert(bb->Length() == cb);
        if (bb->Length() != cb) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsg *FromBuffer(dword id, base::ByteBuffer& bb, dword cb) {
        if (bb.Length() < XMSGSIZE_FIXED) {
            return NULL;
        }
        dword cbSav = bb.Length();
        word w = 0;
        bb.ReadWord(&w);
        if (cb != (dword)w) {
            Assert();
            return NULL;
        }
        byte b;
        bb.ReadByte(&b);
        if (id != (dword)b) {
            return NULL;
        }
        Assert(cbSav - bb.Length() == XMSGSIZE_FIXED);
        if (cbSav - bb.Length() != XMSGSIZE_FIXED) {
            return NULL;
        }
        return new XMsg(id);
    }

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_);
        return ss.str();
    }
#endif
};

const dword XMSGSIZE_ZERODWORD = XMSGSIZE_FIXED + 0 * sizeof(dword);

template <dword ID>
struct XMsg0 : public XMsg
{
    XMsg0() : XMsg(ID) {
    }
    static base::ByteBuffer *ToBuffer() {
        return XMsg::ToBuffer(ID, XMSGSIZE_ZERODWORD);
    }
    static XMsg0<ID> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        return (XMsg0<ID> *)XMsg::FromBuffer(ID, bb, cb);
    }
};

const dword XMSGSIZE_ONEDWORD = XMSGSIZE_FIXED + 1 * sizeof(dword);

template <dword ID>
struct XMsg1 : public XMsg
{
    XMsg1(dword dw0) : XMsg(ID) {
        dw0_ = dw0;
    }
    dword dw0_;

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", dw0: " << dw0_;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(dword dw0) {
        base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_ONEDWORD);
        bb->WriteWord(XMSGSIZE_ONEDWORD);
        bb->WriteByte((byte)ID);
        bb->WriteDword(dw0);
        Assert(bb->Length() == XMSGSIZE_ONEDWORD);
        if (bb->Length() != XMSGSIZE_ONEDWORD) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsg1<ID> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        Assert(cb == XMSGSIZE_ONEDWORD);
        if (cb != XMSGSIZE_ONEDWORD || bb.Length() < XMSGSIZE_ONEDWORD) {
            return NULL;
        }
        word cbSav = bb.Length();

        word w = 0;
        bb.ReadWord(&w);
        if (cb != (dword)w) {
            Assert();
            return NULL;
        }
        byte b = 0;
        bb.ReadByte(&b);
        if (ID != (dword)b) {
            return NULL;
        }
        dword dw;
        if (!bb.ReadDword(&dw)) {
            return NULL;
        }
        Assert(cbSav - bb.Length() == XMSGSIZE_ONEDWORD);
        if (cbSav - bb.Length() != XMSGSIZE_ONEDWORD) {
            return NULL;
        }
        return new XMsg1<ID>(dw);
    }
};

const dword XMSGSIZE_TWODWORD = XMSGSIZE_FIXED + 2 * sizeof(dword);

template <dword ID>
struct XMsg2 : public XMsg
{
    XMsg2(dword dw0, dword dw1) : XMsg(ID) {
        dw0_ = dw0;
        dw1_ = dw1;
    }
    dword dw0_;
    dword dw1_;

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", dw0: " << dw0_ << ", dw1: " << dw1_
                << std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(dword dw0, dword dw1) {
        base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_TWODWORD);
        bb->WriteWord(XMSGSIZE_TWODWORD);
        bb->WriteByte((byte)ID);
        bb->WriteDword(dw0);
        bb->WriteDword(dw1);
        Assert(bb->Length() == XMSGSIZE_TWODWORD);
        if (bb->Length() != XMSGSIZE_TWODWORD) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsg2<ID> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        Assert(cb == XMSGSIZE_TWODWORD);
        if (cb != XMSGSIZE_TWODWORD || bb.Length() < XMSGSIZE_TWODWORD) {
            return NULL;
        }
        word cbSav = bb.Length();

        word w = 0;
        bb.ReadWord(&w);
        if (cb != (dword)w) {
            Assert();
            return NULL;
        }
        byte b = 0;
        bb.ReadByte(&b);
        if (ID != (dword)b) {
            return NULL;
        }
        dword dw0;
        if (!bb.ReadDword(&dw0)) {
            return NULL;
        }
        dword dw1;
        if (!bb.ReadDword(&dw1)) {
            return NULL;
        }
        Assert(cbSav - bb.Length() == XMSGSIZE_TWODWORD);
        if (cbSav - bb.Length() != XMSGSIZE_TWODWORD) {
            return NULL;
        }
        return new XMsg2<ID>(dw0, dw1);
    }
};

const dword XMSGSIZE_THREEDWORD = XMSGSIZE_FIXED + 3 * sizeof(dword);

template <dword ID>
struct XMsg3 : public XMsg
{
    XMsg3(dword dw0, dword dw1, dword dw2) : XMsg(ID) {
        dw0_ = dw0;
        dw1_ = dw1;
        dw2_ = dw2;
    }
    dword dw0_;
    dword dw1_;
    dword dw2_;

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", dw0: " << dw0_ << ", dw1: " << dw1_
                << ", dw2: " << dw2_ << std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(dword dw0, dword dw1, dword dw2) {
        base::ByteBuffer *bb = new base::ByteBuffer(XMSGSIZE_THREEDWORD);
        bb->WriteWord(XMSGSIZE_THREEDWORD);
        bb->WriteByte((byte)ID);
        bb->WriteDword(dw0);
        bb->WriteDword(dw1);
        bb->WriteDword(dw2);
        Assert(bb->Length() == XMSGSIZE_THREEDWORD);
        if (bb->Length() != XMSGSIZE_THREEDWORD) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsg3<ID> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        Assert(cb == XMSGSIZE_THREEDWORD);
        if (cb != XMSGSIZE_THREEDWORD || bb.Length() < XMSGSIZE_THREEDWORD) {
            return NULL;
        }
        word cbSav = bb.Length();

        word w = 0;
        bb.ReadWord(&w);
        if (cb != (dword)w) {
            Assert();
            return NULL;
        }
        byte b = 0;
        bb.ReadByte(&b);
        if (ID != (dword)b) {
            return NULL;
        }
        dword dw0;
        if (!bb.ReadDword(&dw0)) {
            return NULL;
        }
        dword dw1;
        if (!bb.ReadDword(&dw1)) {
            return NULL;
        }
        dword dw2;
        if (!bb.ReadDword(&dw2)) {
            return NULL;
        }
        Assert(cbSav - bb.Length() == XMSGSIZE_THREEDWORD);
        if (cbSav - bb.Length() != XMSGSIZE_THREEDWORD) {
            return NULL;
        }
        return new XMsg3<ID>(dw0, dw1, dw2);
    }
};

const dword XMSGSIZE_STRING1_FIXED = XMSGSIZE_FIXED + sizeof(word);

template <dword ID, int SIZE0>
struct XMsgS1 : public XMsg
{
    XMsgS1(const char *s0) : XMsg(ID) {
        strncpyz(s0_, s0, sizeof(s0_));
    }
    char s0_[SIZE0];

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", s0: " << s0_ << std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(const char *s0) {
        word cbS0 = strlen(s0) + 1;
        char s0T[SIZE0];
        if (cbS0 > sizeof(s0T)) {
            strncpyz(s0T, s0, sizeof(s0T));
            s0 = s0T;
            cbS0 = sizeof(s0T);
        }
        dword cb = XMSGSIZE_STRING1_FIXED + cbS0;
        base::ByteBuffer *bb = new base::ByteBuffer(cb);
        bb->WriteWord(cb);
        bb->WriteByte(ID);
        bb->WriteWord(cbS0);
        bb->WriteBytes((const byte *)s0, cbS0);
        if (bb->Length() != cb) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsgS1<ID, SIZE0> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        dword cbSav = bb.Length();
        word w;
        if (!bb.ReadWord(&w)) {
            return NULL;
        }
        if (cb != (dword)w || w < XMSGSIZE_STRING1_FIXED ||
                w > XMSGSIZE_STRING1_FIXED + SIZE0) {
            Assert();
            return NULL;
        }
        if (w - sizeof(word) > bb.Length()) {
            return NULL;
        }
        byte id = 0;
        bb.ReadByte(&id);
        if (id != ID) {
            return NULL;
        }
        word cbS0 = 0;
        bb.ReadWord(&cbS0);
        if (cbS0 > SIZE0) {
            Assert();
            return NULL;
        }
        char s0[SIZE0];
        bb.ReadBytes((byte *)s0, cbS0);
        s0[cbS0 - 1] = 0;
        if (cbSav - bb.Length() != w) {
            Assert();
            return NULL;
        }

        return new XMsgS1<ID, SIZE0>(s0);
    }
};

const dword XMSGSIZE_STRING2_FIXED = XMSGSIZE_FIXED + sizeof(word) * 2;

template <dword ID, int SIZE0, int SIZE1>
struct XMsgS2 : public XMsg
{
    XMsgS2(const char *s0, const char *s1) : XMsg(ID) {
        strncpyz(s0_, s0, sizeof(s0_));
        strncpyz(s1_, s1, sizeof(s1_));
    }
    char s0_[SIZE0];
    char s1_[SIZE1];

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", s0: " << s0_ << ", s1: " << s1_ <<
                std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(const char *s0, const char *s1) {
        word cbS0 = strlen(s0) + 1;
        char s0T[SIZE0];
        if (cbS0 > sizeof(s0T)) {
            strncpyz(s0T, s0, sizeof(s0T));
            s0 = s0T;
            cbS0 = sizeof(s0T);
        }
        word cbS1 = strlen(s1) + 1;
        char s1T[SIZE1];
        if (cbS1 > sizeof(s1T)) {
            strncpyz(s1T, s1, sizeof(s1T));
            s1 = s1T;
            cbS1 = sizeof(s1T);
        }
        dword cb = XMSGSIZE_STRING2_FIXED + cbS0 + cbS1;
        base::ByteBuffer *bb = new base::ByteBuffer(cb);
        bb->WriteWord(cb);
        bb->WriteByte(ID);
        bb->WriteWord(cbS0);
        bb->WriteBytes((const byte *)s0, cbS0);
        bb->WriteWord(cbS1);
        bb->WriteBytes((const byte *)s1, cbS1);
        if (bb->Length() != cb) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsgS2<ID, SIZE0, SIZE1> *FromBuffer(base::ByteBuffer& bb,
            dword cb) {
        dword cbSav = bb.Length();
        word w;
        if (!bb.ReadWord(&w)) {
            return NULL;
        }
        if (cb != (dword)w || w < XMSGSIZE_STRING2_FIXED ||
                w > XMSGSIZE_STRING2_FIXED + SIZE0 + SIZE1) {
            Assert();
            return NULL;
        }
        if (w - sizeof(word) > bb.Length()) {
            return NULL;
        }
        byte id = 0;
        bb.ReadByte(&id);
        if (id != ID) {
            return NULL;
        }

        word cbS0 = 0;
        bb.ReadWord(&cbS0);
        if (cbS0 > SIZE0) {
            Assert();
            return NULL;
        }
        char s0[SIZE0];
        bb.ReadBytes((byte *)s0, cbS0);
        s0[cbS0 - 1] = 0;
        word cbS1 = 0;
        bb.ReadWord(&cbS1);
        if (cbS1 > SIZE1) {
            Assert();
            return NULL;
        }
        char s1[SIZE1];
        bb.ReadBytes((byte *)s1, cbS1);
        s1[cbS1 - 1] = 0;
        if (cbSav - bb.Length() != w) {
            Assert();
            return NULL;
        }

        return new XMsgS2<ID, SIZE0, SIZE1>(s0, s1);
    }
};

const dword XMSGSIZE_STRING3_FIXED = XMSGSIZE_FIXED + sizeof(word) * 3;

template <dword ID, int SIZE0, int SIZE1, int SIZE2>
struct XMsgS3 : public XMsg
{
    XMsgS3(const char *s0, const char *s1, const char *s2) : XMsg(ID) {
        strncpyz(s0_, s0, sizeof(s0_));
        strncpyz(s1_, s1, sizeof(s1_));
        strncpyz(s2_, s2, sizeof(s2_));
    }
    char s0_[SIZE0];
    char s1_[SIZE1];
    char s2_[SIZE2];

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", s0: " << s0_ << ", s1: " << s1_ <<
                ", s2: " << s2_ << std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(const char *s0, const char *s1,
            const char *s2) {
        word cbS0 = strlen(s0) + 1;
        char s0T[SIZE0];
        if (cbS0 > sizeof(s0T)) {
            strncpyz(s0T, s0, sizeof(s0T));
            s0 = s0T;
            cbS0 = sizeof(s0T);
        }
        word cbS1 = strlen(s1) + 1;
        char s1T[SIZE1];
        if (cbS1 > sizeof(s1T)) {
            strncpyz(s1T, s1, sizeof(s1T));
            s1 = s1T;
            cbS1 = sizeof(s1T);
        }
        word cbS2 = strlen(s2) + 1;
        char s2T[SIZE2];
        if (cbS2 > sizeof(s2T)) {
            strncpyz(s2T, s2, sizeof(s2T));
            s2 = s2T;
            cbS2 = sizeof(s2T);
        }
        dword cb = XMSGSIZE_STRING3_FIXED + cbS0 + cbS1 + cbS2;
        base::ByteBuffer *bb = new base::ByteBuffer(cb);
        bb->WriteWord(cb);
        bb->WriteByte(ID);
        bb->WriteWord(cbS0);
        bb->WriteBytes((const byte *)s0, cbS0);
        bb->WriteWord(cbS1);
        bb->WriteBytes((const byte *)s1, cbS1);
        bb->WriteWord(cbS2);
        bb->WriteBytes((const byte *)s2, cbS2);
        if (bb->Length() != cb) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsgS3<ID, SIZE0, SIZE1, SIZE2> *FromBuffer(base::ByteBuffer& bb,
            dword cb) {
        dword cbSav = bb.Length();
        word w;
        if (!bb.ReadWord(&w)) {
            return NULL;
        }
        if (cb != (dword)w || w < XMSGSIZE_STRING3_FIXED ||
                w > XMSGSIZE_STRING3_FIXED + SIZE0 + SIZE1 + SIZE2) {
            Assert();
            return NULL;
        }
        if (w - sizeof(word) > bb.Length()) {
            return NULL;
        }
        byte id = 0;
        bb.ReadByte(&id);
        if (id != ID) {
            return NULL;
        }

        word cbS0 = 0;
        bb.ReadWord(&cbS0);
        if (cbS0 > SIZE0) {
            Assert();
            return NULL;
        }
        char s0[SIZE0];
        bb.ReadBytes((byte *)s0, cbS0);
        s0[cbS0 - 1] = 0;

        word cbS1 = 0;
        bb.ReadWord(&cbS1);
        if (cbS1 > SIZE1) {
            Assert();
            return NULL;
        }
        char s1[SIZE1];
        bb.ReadBytes((byte *)s1, cbS1);
        s1[cbS1 - 1] = 0;

        word cbS2 = 0;
        bb.ReadWord(&cbS2);
        if (cbS2 > SIZE2) {
            Assert();
            return NULL;
        }
        char s2[SIZE2];
        bb.ReadBytes((byte *)s2, cbS2);
        s2[cbS2 - 1] = 0;

        if (cbSav - bb.Length() != w) {
            Assert();
            return NULL;
        }

        return new XMsgS3<ID, SIZE0, SIZE1, SIZE2>(s0, s1, s2);
    }
};

const dword XMSGSIZE_DWORDSTRING_FIXED = XMSGSIZE_FIXED + sizeof(dword) +
        sizeof(word);

template <dword ID, int SIZE0>
struct XMsgDS : public XMsg
{
    XMsgDS(dword dw0, const char *s0) : XMsg(ID) {
        dw0_ = dw0;
        strncpyz(s0_, s0, sizeof(s0_));
    }
    dword dw0_;
    char s0_[SIZE0];

#ifdef LOGGING
    virtual std::string ToString() {
        std::ostringstream ss;
        ss << XMsgLabels.Find(id_) << ", dw0: " << dw0_ << ", s0: " << s0_ <<
                std::endl;
        return ss.str();
    }
#endif

    static base::ByteBuffer *ToBuffer(dword dw0, const char *s0) {
        word cbS0 = strlen(s0) + 1;
        char s0T[SIZE0];
        if (cbS0 > sizeof(s0T)) {
            strncpyz(s0T, s0, sizeof(s0T));
            s0 = s0T;
            cbS0 = sizeof(s0T);
        }
        dword cb = XMSGSIZE_DWORDSTRING_FIXED + cbS0;
        base::ByteBuffer *bb = new base::ByteBuffer(cb);
        bb->WriteWord(cb);
        bb->WriteByte(ID);
        bb->WriteDword(dw0);
        bb->WriteWord(cbS0);
        bb->WriteBytes((const byte *)s0, cbS0);
        if (bb->Length() != cb) {
            delete bb;
            return NULL;
        }
        return bb;
    }

    static XMsgDS<ID, SIZE0> *FromBuffer(base::ByteBuffer& bb, dword cb) {
        dword cbSav = bb.Length();
        word w;
        if (!bb.ReadWord(&w)) {
            return NULL;
        }
        if (cb != (dword)w || w < XMSGSIZE_DWORDSTRING_FIXED ||
                w > XMSGSIZE_DWORDSTRING_FIXED + SIZE0) {
            Assert();
            return NULL;
        }
        if (w - sizeof(word) > bb.Length()) {
            return NULL;
        }
        byte id = 0;
        bb.ReadByte(&id);
        if (id != ID) {
            return NULL;
        }
        dword dw0;
        bb.ReadDword(&dw0);
        word cbS0 = 0;
        bb.ReadWord(&cbS0);
        if (cbS0 > SIZE0) {
            Assert();
            return NULL;
        }
        char s0[SIZE0];
        bb.ReadBytes((byte *)s0, cbS0);
        s0[cbS0 - 1] = 0;
        if (cbSav - bb.Length() != w) {
            Assert();
            return NULL;
        }
        return new XMsgDS<ID, SIZE0>(dw0, s0);
    }
};

} // namespace wi

#endif // __XMSG_H__
