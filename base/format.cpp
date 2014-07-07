#include "base/format.h"
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string>

namespace base {

static std::string s_str;

const char *Format::ToString(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char sz[512];
    vsnprintf(sz, sizeof(sz), fmt, va);
    va_end(va);
    s_str = sz;
    const char *psz = s_str.c_str();
    return psz;
}

const char *Format::ToHex(const byte *pb, int cb) {
    const char *map = { "0123456789abcdef" };
    char sz[512];
    if (cb >= (sizeof(sz) - 1) / 2) {
        return "0";
    }
    char *pch = sz;
    while (cb-- != 0) {
        *pch++ = map[(((*pb) >> 4) & 0xf)];
        *pch++ = map[((*pb) & 0xf)];
        pb++;
    }
    *pch = 0;
    s_str = sz;
    return s_str.c_str();
}

bool Format::GetHexDigit(char ch, byte *pb) {
    if (ch >= '0' && ch <= '9') {
        *pb = ch - '0';
        return true;
    } else if (ch >= 'A' && ch <= 'F') {
        *pb = ch - 'A' + 10;
        return true;
    } else if (ch >= 'a' && ch <= 'f') {
        *pb = ch - 'a' + 10;
        return true;
    }
    return false;
}

bool Format::FromHex(const char *psz, byte *pb, int cb) {
    const char *pch = psz;
    while (*pch != 0 && cb != 0) {
        byte b;
        if (!GetHexDigit(*pch++, &b)) {
            return false;
        }
        *pb = b << 4;
        if (!GetHexDigit(*pch++, &b)) {
            return false;
        }
        *pb++ |= b;
        cb--;
    }
    return true;
}

bool Format::ToDword(const char *psz, int base, dword *pdw) {
    if (!ValidateString(psz, base)) {
        return false;
    }

    *pdw = 0;
    long long ll = strtoll(psz, (char **)NULL, base);
    *pdw = (dword)ll;
    return true;
}

bool Format::ToInteger(const char *psz, int base, int *pint) {
    if (!ValidateString(psz, base)) {
        return false;
    }

    *pint = 0;
    long l = strtol(psz, (char **)NULL, base);
    *pint = (int)l;
    return true;
}

bool Format::ValidateString(const char *psz, int base) {
    // Checking errno after calling strtol or strtoll is an unreliable way to
    // detect error! That is why this method exists.

    if (base == 10) {
        for (const char *pch = psz; *pch != 0; pch++ ) {
            if (*pch < '0' || *pch > '9') {
                if (pch != psz || *pch != '-') {
                    return false;
                }
            }
        }
        return true;
    }

    if (base == 16) {
        for (const char *pch = psz; *pch != 0; pch++ ) {
            if (!((*pch >= '0' && *pch <= '9') ||
                    (*pch >= 'a' && *pch <= 'f') ||
                    (*pch >= 'A' && *pch <= 'F'))) {
                return false;
            }
        }
        return true;
    }

    return false;
}

} // namespace base
