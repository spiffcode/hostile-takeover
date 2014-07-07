#ifndef __MISC_H__
#define __MISC_H__

#include "inc/basictypes.h"
#include <ctype.h>

namespace wi {

#if defined(__CPU_68K)
#define BigWord(x) (x)
#define BigDword(x) (x)
#else
#define BigWord(x) ((((x)&0xFF)<<8) | (((x)&0xFF00)>>8))
#define BigDword(x) ((((x)&0xFF)<<24) | (((x)&0xFF00)<<8) | (((x)&0xFF0000)>>8) | (((x)&0xFF000000)>>24))
#endif

extern void strncpyz(char *pszDst, const char *pszSrc, int cb);
extern int stricmp(const char *psz1, const char *psz2);
extern dword HashString(const char *s);
extern char *AllocString(const char *psz);
extern const char *StripWhitespace(const char *sz, int *pcch);

} // namespace wi

#endif // __MISC_H__
