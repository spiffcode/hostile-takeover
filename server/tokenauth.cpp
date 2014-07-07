#include "server/tokenauth.h"
#include "server/secrets.h"
#include "mpshared/misc.h"
#include "mpshared/mpht.h"
#include "base/base64.h"
#include "base/format.h"
#include "base/md5.h"
#include <string>

namespace wi {

dword TokenAuth::Authenticate(const char *username, const char *token) {
    if (strlen(token) > kcbTokenMax) {
        return knAuthResultFail;
    }

    // base64 decode it
    char output[kcbTokenMax * 2];
    int cb = base::base64decode((byte *)token, strlen(token), (byte *)output,
            sizeof(output));
    if (cb == -1) {
        return knAuthResultFail;
    }
    output[cb] = 0;

// example
// [{"c":30782,"u":"c2NvdHRsdQ==","t":1239744982},"cad14dfc03ad28caa83d9bd298f91e31"]

    // Pull out the pieces we need. First, read out the bytes between the {},
    // inclusive
    std::string t(output);
    int start = t.find('{');
    if (start < 0) {
        return knAuthResultFail;
    }
    int end = t.find('}');
    if (end < 0) {
        return knAuthResultFail;
    }
    std::string s(t, start, end - start + 1);

    // Pull out the hash
    end = t.rfind('"');
    if (end < 0) {
        return knAuthResultFail;
    }
    start = t.rfind('"', end - 1);
    if (start < 0) {
        return knAuthResultFail;
    }
    std::string h(t, start + 1, end - (start + 1));

    // Compare the passed username with the token username
    // Need to base64 decode the name first.
    start = t.find("\"u\":\"");
    if (start < 0) {
        return knAuthResultFail;
    }
    end = t.find("\",", start);
    if (end < 0) {
        return knAuthResultFail;
    }
    std::string username64(t, start + 5, end - (start + 5));
    // base64 decode it
    cb = base::base64decode((byte *)username64.c_str(), username64.size(),
            (byte *)output, sizeof(output));
    if (cb == -1) {
        return knAuthResultFail;
    }
    output[cb] = 0;
    if (strcmp(username, output) != 0) {
        return knAuthResultFail;
    }

    // Hash the first string with token auth secret appended
    MD5_CTX md5;
    MD5Init(&md5);
    std::string sT(s + kszTokenAuthSecret);
    MD5Update(&md5, (const byte *)sT.c_str(), sT.size());
    byte hash[16];
    MD5Final(hash, &md5);

    // Compare to the passed in hash. This validates the token.
    if (strcmp(h.c_str(), base::Format::ToHex(hash, sizeof(hash))) != 0) {
        return knAuthResultFail;
    }

    // Compare the timestamp with the current time.
    start = t.find("\"t\":");
    if (start < 0) {
        return knAuthResultFail;
    }
    int startN = start + 4;
    int endN = startN;
    while (t[endN] >= '0' && t[endN] <= '9') {
        endN++;
    }
    std::string ts(t, startN, endN - startN);
    dword tToken;
    if (!base::Format::ToDword(ts.c_str(), 10, &tToken)) {
        return knAuthResultFail;
    }

    // Compare to current time - seconds since epoch. Current time must be
    // less than or equal. If this fails, return stale so the client
    // knows to get a new token.
    time_t tCurrent = time(NULL);
    if (tCurrent > tToken) {
        return knAuthResultStaleToken;
    }

    // It's good!
    return knAuthResultSuccess;
}

bool TokenAuth::IsAnonymous(const char *username, const char *token) {
    return *token == 0;
}

} // namespace wi
