#include "winpackfile.h"

PdbReader *WinPackFileReader::OpenPdb(char *szDir, char *pszFn) {
	// Load data from this directory

	char szT[_MAX_PATH];
	strcpy(szT, pszDir);
	strcat(szT, "\\");
	strcat(szT, pszFn);

	// PdbReader over memory

	MemPdbReader *ppdbReader = new MemPdbReader;
	if (ppdbReader == NULL) {
        Trace("FilePdbReader null");
		return false;
    }
	if (!ppdbReader->Open(szT)) {
		delete ppdbReader;
        Trace("FilePdbReader::Open(%s, %s) failed", pszDir, pszFn);
		return false;
	}

    return ppdbReader;
}

bool WinPackFileReader::DeletePdb(char *szDir, char *pszFn) {
	char szT[_MAX_PATH];
	strcpy(szT, pszDir);
	strcat(szT, "\\");
	strcat(szT, pszFn);

	TCHAR szFilename[_MAX_PATH];
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, szT, -1, szFilename, ARRAYSIZE(szFilename) - 1);
#else
	strcpy(szFilename, szT);
#endif
	DeleteFile(szFilename);
    return true;
}
