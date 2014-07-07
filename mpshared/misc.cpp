#include "mpshared/misc.h"
#include <string.h>

namespace wi {

// Zero terminating counted strcpy

void strncpyz(char *pszDst, const char *pszSrc, int cb)
{
    if (cb <= 0)
        return;
	cb--;
	while (cb-- != 0) {
		if ((*pszDst++ = *pszSrc++) == 0)
			return;
	}
	*pszDst = 0;
}

int stricmp(const char *psz1, const char *psz2)
{
	while (true) {
		byte b1 = (byte)*psz1++;
		if (b1 >= 'A' && b1 <= 'Z')
			b1 += 'a' - 'A';
		byte b2 = (byte)*psz2++;
		if (b2 >= 'A' && b2 <= 'Z')
			b2 += 'a' - 'A';		
		if (b1 != b2)
			return b1 - b2;
		if (b1 == 0)
			return 0;
	}
}

dword HashString(const char *s) {
    dword h = 0;
    for (int i = 0; *s != 0; s++, i++) {
        h = 5 * (i + 1) * h + *s;
    }
    return h;
}

char *AllocString(const char *psz) {
    int cb = strlen(psz) + 1;
    char *pszNew = new char[cb];
    if (pszNew == NULL) {
        return NULL;
    }
    strncpyz(pszNew, psz, cb);
    return pszNew;
}

const char *StripWhitespace(const char *sz, int *pcch) {
    int cch = strlen(sz);
    int start = 0;
    for (int i = 0; i < cch; i++, start++) {
        if (!isspace(sz[i])) {
            break;
        }
    }

    int end = cch;
    for (int i = cch - 1; i >= start; i--, end--) {
        if (!isspace(sz[i])) {
            break;
        }
    }

    *pcch = end - start;
    return &sz[start];
}

} // namespace wi
