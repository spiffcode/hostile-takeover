#include "game/iphone/iphonepackfile.h"
#include "game/mempdbreader.h"
#include "inc/rip.h"
#include <string.h>
#include <sys/syslimits.h>

namespace wi {

PdbReader *IPhonePackFileReader::OpenPdb(const char *pszDir,
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

bool IPhonePackFileReader::DeletePdb(const char *pszDir, const char *pszFn) {
	char szT[PATH_MAX];
	strcpy(szT, pszDir);
	strcat(szT, "/");
	strcat(szT, pszFn);
    unlink(szT);
    return true;
}

} // namespace wi
