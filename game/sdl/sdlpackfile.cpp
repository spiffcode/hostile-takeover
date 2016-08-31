#include "game/sdl/sdlpackfile.h"
#include "game/mempdbreader.h"
#include "inc/rip.h"
#include <string.h>
#include <limits.h>

namespace wi {

PdbReader *SdlPackFileReader::OpenPdb(const char *pszDir,
        const char *pszFn) {
	// Load data from this directory

	char szT[PATH_MAX];
    if (pszDir != NULL) {
        strcpy(szT, pszDir);
        strcat(szT, "/");
        strcat(szT, pszFn);
    } else {
        strcpy(szT, pszFn);
    }

	// PdbReader over memory
	MemPdbReader *ppdbReader = new MemPdbReader;
	if (ppdbReader == NULL) {
        Trace("MemPdbReader null");
		return false;
    }
	if (!ppdbReader->Open(szT)) {
		delete ppdbReader;
        Trace("MemPdbReader::Open(%s) failed", pszFn);
		return false;
	}

    return ppdbReader;
}

bool SdlPackFileReader::DeletePdb(const char *pszDir, const char *pszFn) {
// TODO(darrinm): do something cross-platform. Then replace iphonepackfile.* w/ the new version.
// - does SDL have anything? [I don't think so]
// - POSIX (mac, linux, iOS): unlink()
// - Windows: see win/winpackfile.cpp
// - Android?

	char szT[PATH_MAX];
	strcpy(szT, pszDir);
	strcat(szT, "/");
	strcat(szT, pszFn);
    unlink(szT);
    return true;
}

} // namespace wi
