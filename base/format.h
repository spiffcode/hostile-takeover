#ifndef __FORMAT_H__
#define __FORMAT_H__

#include "inc/basictypes.h"

namespace base {

class Format {
public:
    static const char *ToString(const char *fmt, ...);
    static const char *ToHex(const byte *pb, int cb);
    static bool FromHex(const char *psz, byte *pb, int cb);
    static bool ToDword(const char *psz, int base, dword *pdw);
    static bool ToInteger(const char *psz, int base, int *pint);

private:
    static bool GetHexDigit(char ch, byte *pb);
    static bool ValidateString(const char *psz, int base);
};

} // namespace base

#endif // __FORMAT_H__
