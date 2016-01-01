#include "../ht.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
//#include <memory.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

namespace wi {

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
	char m_szFile[PATH_MAX];
	char m_szDelete[PATH_MAX];
	char m_szFinal[PATH_MAX];
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
            unlink(m_szDelete);
		}
		if (m_szFinal[0] != 0) {
            rename(m_szFile, m_szFinal);
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

void PrependSavesDirectory(char *pszIn, char *pszOut)
{
	strcpy(pszOut, IPhone::GetSaveGamesDir());
	strcat(pszOut, "/");
	strcat(pszOut, pszIn);
}

bool FindSaveGame(int nGame, char *psz, int cb, int *pc = NULL)
{
	if (pc != NULL)
		*pc = 0;

    // This is the prefix of the file being looked for

	char szCompare[PATH_MAX];
	sprintf(szCompare, "htsave%d_", nGame);
	int cchCompare = strlen(szCompare);

    // This is the special save game that is only used
    // when the game exits and reloads right away

	char szReinitializeSave[20];
	sprintf(szReinitializeSave, "htsave%d_", knGameReinitializeSave);
	int cchReinitializeSave = strlen(szReinitializeSave);

    // Enum files in this directory

	char szFileSpec[PATH_MAX];
	PrependSavesDirectory("", szFileSpec);
    DIR *pdir = opendir(szFileSpec);
    if (pdir == NULL) {
        return false;
    }

    int c = 0;
    dirent *pdent;
    while ((pdent = readdir(pdir)) != NULL) {
        // Only consider save games, because if the desired save game is
        // not found, we need to count "slots".

        if (strncmp("htsave", pdent->d_name, 6) != 0) {
            continue;
        }

        // Save game found?

        if (strncmp(szCompare, pdent->d_name, cchCompare) == 0) {
			if (psz != NULL)
				strncpyz(psz, pdent->d_name, cb);
			closedir(pdir);
			return true;
		}

        // Count save games but don't count the temporary "Reinitialize"
        // saved game

		if (strncmp(szReinitializeSave, pdent->d_name,
                cchReinitializeSave) == 0) {
			continue;
        }

        // Count this save game as a slot

		c++;
    }
    closedir(pdir);

    // Didn't find the saved game, but did count the number of occupied slots

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

	char szT[PATH_MAX];
	if (!FindSaveGame(nGame, szT, sizeof(szT))) {
		strncpyz(psz, "-- Empty --", cb);
		return false;
	}

	char szPath[PATH_MAX];
	PrependSavesDirectory(szT, szPath);

	// Get the creation time in hours24, minutes

    struct stat st;
    if (stat(szPath, &st) > 0) {
        return false;
    }
    time_t tim = 0;
    struct tm *ptm = localtime(&tim);
	*pnHours24 = ptm->tm_hour;
	*pnMinutes = ptm->tm_min;
	*pnSeconds = ptm->tm_sec;
	pdate->nDay = ptm->tm_mday;
	pdate->nMonth = ptm->tm_mon;
	pdate->nYear = ptm->tm_year + 1900;

	// Copy over filename, lose prefix

	char *pszName = strchr(szT, '_') + 1;
	int cbName = strlen(pszName) - 4 + 1;
	strncpyz(psz, pszName, _min(cb, cbName));

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

	char szOld[PATH_MAX];
	char szOldFull[PATH_MAX];
	if (!FindSaveGame(nGame, szOld, sizeof(szOld))) {
		szOldFull[0] = 0;
	} else {
		PrependSavesDirectory(szOld, szOldFull);
	}

	// New file name

	char szNew[PATH_MAX];
	sprintf(szNew, "htsave%d_%s.bin", nGame, pszName);

	// windows disallows ':' in a filename, so sub those out

	char *pchInvalid = szNew;
	do {
		pchInvalid = strchr(pchInvalid, ':');
		if (pchInvalid != 0)
			*pchInvalid = '#';
	} while (pchInvalid != 0);

	char szNewFull[PATH_MAX];
	PrependSavesDirectory(szNew, szNewFull);

	// Get stream over temp file

	FileStream *pstm = new FileStream();
	if (pstm == NULL)
		return NULL;

	// If save is successful, szOld will be deleted and httempsave.bin
	// will be renamed to szNew

	char szTempSaveFull[PATH_MAX];
	PrependSavesDirectory("httempsave.bin", szTempSaveFull);

	if (!pstm->Init("wb", szTempSaveFull, szOldFull, szNewFull)) {
		delete pstm;
		return NULL;
	}

	return (Stream *)pstm;
}

Stream *HostOpenSaveGameStream(int nGame, bool fDelete)
{
	char szT[PATH_MAX];
	if (!FindSaveGame(nGame, szT, sizeof(szT)))
		return NULL;

	// rename to a temporary file before opening

	char szTFull[PATH_MAX];
	PrependSavesDirectory(szT, szTFull);
	char szTempNameFull[PATH_MAX];
	PrependSavesDirectory(kszTempName, szTempNameFull);
    rename(szTFull, szTempNameFull);

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

	char szSaveGame[PATH_MAX];

	if (psz == NULL) {
		if (!FindSaveGame(nGame, szSaveGame, sizeof(szSaveGame)))
			return false;
	} else {
		strncpyz(szSaveGame, psz, sizeof(szSaveGame));
	}

	char szSaveGameFull[PATH_MAX];
	PrependSavesDirectory(szSaveGame, szSaveGameFull);

	if (psz != NULL) {
        unlink(szSaveGameFull);
		return true;
	}
	return false;
}

} // namespace wi
