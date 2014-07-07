#include "server/ncpackfile.h"
#include "server/ncpdbreader.h"
#include "base/log.h"
#include <string.h>
#include <sys/param.h>

namespace wi {

PdbReader *NoCachePackFileReader::OpenPdb(const char *pszDir,
        const char *pszFn) {
	// Load data from this directory

	char szT[PATH_MAX];
    if (pszDir != NULL) {
        strcpy(szT, pszDir);
        strcat(szT, "/");
        strcat(szT, pszFn);
    } else {
        pszDir = "";
        strcpy(szT, pszFn);
    }

	// Non-caching pdbReader over memory
	NoCachePdbReader *ppdbReader = new NoCachePdbReader;
	if (ppdbReader == NULL) {
        RLOG() << "NoCachePdbReader null";
		return NULL;
    }
	if (!ppdbReader->Open(szT)) {
		delete ppdbReader;
        RLOG() << base::Log::Format("NoCachePdbReader::Open(%s, %s) failed",
                pszDir, pszFn);
		return NULL;
	}

    return ppdbReader;
}

bool NoCachePackFileReader::DeletePdb(const char *pszDir, const char *pszFn) {
    // Delete isn't supported with this implementation
    return false;
}

} // namespace wi
