// Turn this on to test the UNICODE path under XP (needed for PocketPC which is only
// unicode). Be sure to turn it off before checking; Win9x / ME doesn't have unicode.

//#define UNICODE

#include "..\ht.h"
#include <stdio.h>
#ifdef DEBUG
#define _CRTDBG_MAP_ALLOC
#endif
#include <stdlib.h>
#if defined(DEBUG) && !defined(CE)
//#include <crtdbg.h>
#endif
#include <memory.h>
#include <string.h>

class FileStream : public Stream
{
public:
	FileStream();
	~FileStream();
	bool Init(char *pszMode, char *pszFile, char *pszDelete, char *pszFinal);
	
	virtual void Close();
	virtual dword Read(void *pv, dword cb);
	virtual dword Write(void *pv, dword cb);
	virtual bool IsSuccess();

private:
	bool m_fSuccess;
	FILE *m_pf;
	char m_szFile[_MAX_PATH];
	char m_szDelete[_MAX_PATH];
	char m_szFinal[_MAX_PATH];
};

FileStream::FileStream()
{
	m_fSuccess = true;
	m_pf = NULL;
	m_szFile[0] = 0;
	m_szDelete[0] = 0;
	m_szFinal[0] = 0;
}

FileStream::~FileStream()
{
	Assert(m_pf == NULL);
}

bool FileStream::Init(char *pszMode, char *pszFile, char *pszDelete, char *pszFinal)
{
	m_pf = fopen(pszFile, pszMode);
	if (m_pf == NULL)
		return false;
	strcpy(m_szFile, pszFile);
	if (pszDelete != NULL)
		strcpy(m_szDelete, pszDelete);
	if (pszFinal != NULL)
		strcpy(m_szFinal, pszFinal);
	return true;
}

void FileStream::Close()
{
	fclose(m_pf);
	m_pf = NULL;

	// Delete and rename file if no error

	if (m_fSuccess) {
		if (m_szDelete[0] != 0) {
			TCHAR szFilename[MAX_PATH];
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0, m_szDelete, -1, szFilename, ARRAYSIZE(szFilename) - 1);
#else
			strcpy(szFilename, m_szDelete);
#endif

			DeleteFile(szFilename);
		}
		if (m_szFinal[0] != 0) {
			TCHAR szFile[MAX_PATH];
			TCHAR szFinal[MAX_PATH];
#ifdef UNICODE
			MultiByteToWideChar(CP_ACP, 0, m_szFile, -1, szFile, ARRAYSIZE(szFile) - 1);
			MultiByteToWideChar(CP_ACP, 0, m_szFinal, -1, szFinal, ARRAYSIZE(szFinal) - 1);
#else
			strcpy(szFile, m_szFile);
			strcpy(szFinal, m_szFinal);
#endif
			MoveFile(szFile, szFinal);
		}
	}
}

dword FileStream::Read(void *pv, dword cb)
{
	size_t cbT = fread(pv, 1, cb, m_pf);
	if (cb != cbT)
		m_fSuccess = false;
	return cbT;
}

dword FileStream::Write(void *pv, dword cb)
{
	size_t cbT = fwrite(pv, 1, cb, m_pf);
	if (cb != cbT)
		m_fSuccess = false;
	return cbT;
}

bool FileStream::IsSuccess()
{
	return m_fSuccess;
}

void PrependDataDirectory(char *pszIn, char *pszOut)
{
	char *pszDataDir = HostGetDataDirectory();
	strcpy(pszOut, pszDataDir);
	strcat(pszOut, "\\");
	strcat(pszOut, pszIn);
}

bool FindSaveGame(int nGame, char *psz, int cb, int *pc = NULL)
{
	if (pc != NULL)
		*pc = 0;

	char szFileSpec[_MAX_PATH];
	PrependDataDirectory("htsave*.bin", szFileSpec);

	TCHAR szT[_MAX_PATH];
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, szFileSpec, -1, szT, ARRAYSIZE(szT) - 1);
#else
	strcpy(szT, szFileSpec);
#endif
	WIN32_FIND_DATA find;
	HANDLE h = FindFirstFile(szT, &find);

	if (h == INVALID_HANDLE_VALUE)
		return false;

	char szCompare[_MAX_PATH];
	sprintf(szCompare, "htsave%d_", nGame);
	int cchCompare = strlen(szCompare);

	char szReinitializeSave[20];
	sprintf(szReinitializeSave, "htsave%d_", knGameReinitializeSave);
	int cchReinitializeSave = strlen(szReinitializeSave);

	int c = 0;
	do {
		char szFilename[MAX_PATH];
#ifdef UNICODE
		WideCharToMultiByte(CP_ACP, 0, find.cFileName, -1, szFilename, ARRAYSIZE(szFilename) - 1, NULL, NULL);
#else
		strcpy(szFilename, find.cFileName);
#endif
		if (strncmp(szCompare, szFilename, cchCompare) == 0) {
			FindClose(h);
			if (psz != NULL)
				strncpyz(psz, szFilename, cb);
			return true;
		}

		// Don't count the temporary "Reinitialize" saved game

		if (strncmp(szReinitializeSave, szFilename, cchReinitializeSave) == 0)
			continue;

		c++;
	} while (FindNextFile(h, &find) != 0);

	FindClose(h);

	if (pc != NULL)
		*pc = c;
	return false;
}

int HostGetSaveGameCount()
{
	int c;
	FindSaveGame(-1, NULL, 0, &c);
	return c;
}

bool HostGetSaveGameName(int nGame, char *psz, int cb, Date *pdate, int *pnHours24, int *pnMinutes, int *pnSeconds)
{
	// Find the game

	char szT[_MAX_PATH];
	if (!FindSaveGame(nGame, szT, sizeof(szT))) {
		strncpyz(psz, "-- Empty --", cb);
		return false;
	}

	char szPath[_MAX_PATH];
	PrependDataDirectory(szT, szPath);

	// Get the creation time in hours24, minutes
	// Note to Win32 guys: this is insane.

	TCHAR szFilename[MAX_PATH];
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, szPath, -1, szFilename, ARRAYSIZE(szFilename) - 1);
#else
	strcpy(szFilename, szPath);
#endif

	HANDLE h = CreateFile(szFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h == INVALID_HANDLE_VALUE)
		return false;

	FILETIME timeCreation;
	FILETIME timeLastAccess;
	FILETIME timeLastWrite;
	if (!GetFileTime(h, &timeCreation, &timeLastAccess, &timeLastWrite)) {
		CloseHandle(h);
		return false;
	}
	FILETIME timeLastWriteLocal;
	FileTimeToLocalFileTime(&timeLastWrite, &timeLastWriteLocal);
	SYSTEMTIME timeLastWriteLocalAndReadable;
	FileTimeToSystemTime(&timeLastWriteLocal, &timeLastWriteLocalAndReadable);
	*pnHours24 = timeLastWriteLocalAndReadable.wHour;
	*pnMinutes = timeLastWriteLocalAndReadable.wMinute;
	*pnSeconds = timeLastWriteLocalAndReadable.wSecond;
	pdate->nDay = timeLastWriteLocalAndReadable.wDay;
	pdate->nMonth = timeLastWriteLocalAndReadable.wMonth;
	pdate->nYear = timeLastWriteLocalAndReadable.wYear;
	CloseHandle(h);

	// Copy over filename, lose prefix

	char *pszName = strchr(szT, '_') + 1;
	int cbName = strlen(pszName) - 4 + 1;
	strncpyz(psz, pszName, min(cb, cbName));

	// restore '#' to ':'

	char *pchInvalid = psz;
	do {
		pchInvalid = strchr(pchInvalid, '#');
		if (pchInvalid != 0)
			*pchInvalid = ':';
	} while (pchInvalid != 0);

	return true;
}

Stream *HostNewSaveGameStream(int nGame, char *pszName)
{
	// Get the old file name - we'll delete this if successful

	char szOld[_MAX_PATH];
	char szOldFull[_MAX_PATH];
	if (!FindSaveGame(nGame, szOld, sizeof(szOld))) {
		szOldFull[0] = 0;
	} else {
		PrependDataDirectory(szOld, szOldFull);
	}

	// New file name

	char szNew[_MAX_PATH];
	sprintf(szNew, "htsave%d_%s.bin", nGame, pszName);

	// windows disallows ':' in a filename, so sub those out

	char *pchInvalid = szNew;
	do {
		pchInvalid = strchr(pchInvalid, ':');
		if (pchInvalid != 0)
			*pchInvalid = '#';
	} while (pchInvalid != 0);

	char szNewFull[_MAX_PATH];
	PrependDataDirectory(szNew, szNewFull);

	// Get stream over temp file

	FileStream *pstm = new FileStream();
	if (pstm == NULL)
		return NULL;

	// If save is successful, szOld will be deleted and httempsave.bin
	// will be renamed to szNew

	char szTempSaveFull[_MAX_PATH];
	PrependDataDirectory("httempsave.bin", szTempSaveFull);

	if (!pstm->Init("wb", szTempSaveFull, szOldFull, szNewFull)) {
		delete pstm;
		return NULL;
	}

	return (Stream *)pstm;
}

Stream *HostOpenSaveGameStream(int nGame, bool fDelete)
{
	char szT[_MAX_PATH];
	if (!FindSaveGame(nGame, szT, sizeof(szT)))
		return NULL;

	// rename to a temporary file before opening

	char szTFull[_MAX_PATH];
	PrependDataDirectory(szT, szTFull);
	char szTempNameFull[_MAX_PATH];
	PrependDataDirectory(kszTempName, szTempNameFull);

	TCHAR szSaveGame[_MAX_PATH];
	TCHAR szTempName[_MAX_PATH];

#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, szTFull, -1, szSaveGame, ARRAYSIZE(szSaveGame) - 1);
	MultiByteToWideChar(CP_ACP, 0, szTempNameFull, -1, szTempName, ARRAYSIZE(szTempName) - 1);
#else
	strcpy(szSaveGame, szTFull);
	strcpy(szTempName, szTempNameFull);
#endif
	MoveFile(szSaveGame, szTempName);

	// Get stream over temp file

	FileStream *pstm = new FileStream();
	if (pstm == NULL)
		return NULL;

	// If load is successful, and fDelete is True  szTempName will be deleted
	// if fDelete is false szTempName will be renamed to szSaveGame

	if (!pstm->Init("rb", szTempNameFull, fDelete ? szTempNameFull : NULL, fDelete ? NULL : szTFull)) {
		delete pstm;
		return NULL;
	}

	return (Stream *)pstm;
}

bool HostDeleteSaveGame(char *psz, int nGame)
{
	// if nGame is > 0, delete that, otherwise if psz is non-null delete that
	// return true if we deleted something

	char szSaveGame[_MAX_PATH];

	if (psz == NULL) {
		if (!FindSaveGame(nGame, szSaveGame, sizeof(szSaveGame)))
			return false;
	} else {
		strncpyz(szSaveGame, psz, sizeof(szSaveGame));
	}

	char szSaveGameFull[_MAX_PATH];
	PrependDataDirectory(szSaveGame, szSaveGameFull);

	// the use of ARRAYSIZE says to me that we don't know whether the string will be UNICODE or not 
	// and this is a conservative way of getting the character count.  But why don't we know?

	if (psz != NULL) {
		TCHAR szFilename[_MAX_PATH];
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, szSaveGameFull, -1, szFilename, ARRAYSIZE(szFilename) - 1);
#else
		strcpy(szFilename, szSaveGameFull);
#endif

		DeleteFile(szFilename);
		return true;
	}
	return false;
}