#ifndef __TOKENAUTH_H__
#define __TOKENAUTH_H__

#include "inc/basictypes.h"
#include "base/misc.h"

namespace wi {

const dword knAuthResultSuccess = 0;
const dword knAuthResultFail = 1;
const dword knAuthResultStaleToken = 2;

STARTLABEL(AuthResults)
    LABEL(knAuthResultSuccess)
    LABEL(knAuthResultFail)
    LABEL(knAuthResultStaleToken)
ENDLABEL(AuthResults)

class TokenAuth {
public:
    static dword Authenticate(const char *username, const char *token);
    static bool IsAnonymous(const char *username, const char *token);
};

} // namespace wi

#endif // __TOKENAUTH_H__
