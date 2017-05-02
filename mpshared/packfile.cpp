#include "base/md5.h"
#include "mpshared/packfile.h"
#include "mpshared/misc.h"
#include "base/log.h"
#include <inc/rip.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>

namespace wi {

//#define INCL_RECORDOPENS

#if defined(INCL_RECORDOPENS)
void RecordOpen(char *pszApi, char *pszFile, dword dw)
{
	FILE *pfil = fopen("openlog.txt", "a+");
	fseek(pfil, 0, SEEK_END);
	fprintf(pfil, "0x%08lx: %s %s\n", dw, pszApi, pszFile);
	fclose(pfil);
}

void RecordClose(dword dw)
{
	FILE *pfil = fopen("openlog.txt", "r+");
	char sz[128];
	while (true) {
		long pos = ftell(pfil);
		if (fgets(sz, sizeof(sz) - 1, pfil) == NULL)
			break;
		char szCompare[32];
		sprintf(szCompare, "0x%08lx", dw);
		if (strncmp(sz, szCompare, 10) == 0) {
			fseek(pfil, pos, SEEK_SET);
			char ch = 'x';
			fwrite(&ch, 1, 1, pfil);
			fclose(pfil);
			return;
		}
	}
	fclose(pfil);
}
#endif

//
// Reads packed files
//

PackFileReader::PackFileReader()
{
	m_crnfo = 0;
	m_crnfoAlloc = 0;
	m_arnfo = NULL;
}

PackFileReader::~PackFileReader()
{
	while (m_crnfo != 0)
		Pop();
	delete[] m_arnfo;
}

File *PackFileReader::fopen(const char *pszFn, const char *pszMode)
{
	// Only support read binary mode

	if (strcmp(pszMode, "rb") != 0)
		return NULL;

	// Find this directory entry

	ReaderInfo *prnfo;
	DirEntry dir;
	if (!FindDirEntry(pszFn, &dir, &prnfo)) {
#if !defined(SDL)
        LOG() << "can't find dir entry " << pszFn;
#endif
		return NULL;
    }

	// Alloc the file stream info for this file, fill it in

	File *pfil = (File *)new byte[sizeof(File) + sizeof(word) *
            (dir.cRecs - 1)];
	if (pfil == NULL)
		return NULL;
	pfil->prnfo = prnfo;
	pfil->offRecStart = 0;
	pfil->offStream = 0;
	pfil->nRecOffStream = 0;
	pfil->cbTotal = 0;
	pfil->cRecs = dir.cRecs;
	pfil->nRecFirst = dir.nRecFirst;

	// Remember the size of each record

	word *pcb = pfil->acb;
	word nRecFirst = BigWord(dir.nRecFirst);
	for (word nRec = nRecFirst; nRec < nRecFirst + dir.cRecs; nRec++) {
		word cbT;
		prnfo->ppdbReader->GetRecordSize(nRec, &cbT);
		pfil->cbTotal += cbT;
		*pcb++ = cbT;
	}

	// Remember we have one open

	prnfo->cOpen++;

#if defined(INCL_RECORDOPENS)
	RecordOpen("fopen", pszFn, (dword)pfil);
#endif

	return pfil;
}

int PackFileReader::fclose(File *pfil)
{
#if defined(INCL_RECORDOPENS)
	RecordClose((dword)pfil);
#endif

	Assert(pfil->prnfo->cOpen > 0);
	pfil->prnfo->cOpen--;
	delete[] (byte *)pfil;
	return 0;
}

dword PackFileReader::fread(void *pv, dword cb, int c, File *pfil)
{
	dword cbT = cb * c;
	byte *pb = (byte *)pv;

	word nRecFirst = BigWord(pfil->nRecFirst);
	while (cbT != 0 && pfil->nRecOffStream < pfil->cRecs) {
		dword offRecEnd = pfil->offRecStart + pfil->acb[pfil->nRecOffStream];
		word cbLeft = (word)(offRecEnd - pfil->offStream);
		word cbReadNext = cbT < (dword)cbLeft ? (word)cbT : cbLeft;
		pfil->prnfo->ppdbReader->ReadRecord(nRecFirst + pfil->nRecOffStream, (word)(pfil->offStream - pfil->offRecStart), cbReadNext, pb);
		cbT -= cbReadNext;
		pb += cbReadNext;
		pfil->offStream += cbReadNext;
		if (pfil->offStream == offRecEnd && pfil->nRecOffStream < pfil->cRecs) {
			pfil->offRecStart = offRecEnd;
			pfil->nRecOffStream++;
		}
	}

	// If it was all read, return the count

	if (cbT == 0)
		return c;

	// It wasn't all read, return the count that was read. Note we may have a partial
	// item read which according to c-runtime docs is ok.

	dword cbRead = cb * c - cbT;
	return cbRead / cb;
}

int PackFileReader::fseek(File *pfil, int delta, int nOrigin)
{
	dword offNew;
	switch (nOrigin) {
	case SEEK_CUR:
		// Current file position + delta

		offNew = pfil->offStream + delta;
		break;

	case SEEK_END:
		// End of file + delta

		offNew = pfil->cbTotal + delta;
		break;

	case SEEK_SET:
		// Start of file + delta

		offNew = delta;
		break;

	default:
		// Error

		return 1;
	}

	// Within file bounds? Error for now even though c-runtime allows this

	if ((long)offNew < 0 || offNew > pfil->cbTotal)
		return 1;

	// Special supported case for seeking to the end

	if (offNew == pfil->cbTotal) {
		if (pfil->cbTotal == 0)
			return 0;
		while (pfil->nRecOffStream < pfil->cRecs - 1) {
			pfil->offRecStart += pfil->acb[pfil->nRecOffStream];
			pfil->nRecOffStream++;
		}
		pfil->offStream = offNew;
		return 0;
	}

	// Special case for seeking from the end backwards

	if (pfil->offStream == pfil->cbTotal) {
		if (pfil->nRecOffStream == pfil->cRecs) {
			pfil->nRecOffStream--;
			pfil->offRecStart -= pfil->acb[pfil->nRecOffStream];
		}
	}

	// Need to update File * members.

	if (offNew < pfil->offRecStart + pfil->acb[pfil->nRecOffStream]) {
		// The new pos is in the current record or behind

		if (offNew < pfil->offRecStart) {
			// The new pos is in a previous record

			while (true) {
				pfil->nRecOffStream--;
				Assert((char)pfil->nRecOffStream >= 0);
				pfil->offRecStart -= pfil->acb[pfil->nRecOffStream];
				if (offNew >= pfil->offRecStart) {
					pfil->offStream = offNew;
					break;
				}
			}
		} else {
			// The new pos is in the current record

			pfil->offStream = offNew;
		}
	} else {
		// The new pos is in a record ahead of the current record

		while (true) {
			pfil->offRecStart += pfil->acb[pfil->nRecOffStream];
			pfil->nRecOffStream++;
			Assert(pfil->nRecOffStream < pfil->cRecs);
			if (offNew < pfil->offRecStart + pfil->acb[pfil->nRecOffStream]) {
				pfil->offStream = offNew;
				break;
			}
		}
	}

	return 0;
}

dword PackFileReader::ftell(File *pfil)
{
	return pfil->offStream;
}

bool PackFileReader::EnumFiles(Enum *penm, int key, char *pszFn, int cb)
{
	if (penm->m_pvNext == (void *)kEnmFirst) {
        if (m_crnfo == 0) {
            return false;
        }
        ReaderInfo *prnfo = NULL;
        if (key == PACKENUM_FIRST) {
            prnfo = &m_arnfo[0];
        }
        if (key == PACKENUM_LAST) {
            prnfo = &m_arnfo[m_crnfo - 1];
        }
        if (prnfo == NULL) {
            return false;
        }
        penm->m_pvNext = (void *)prnfo;
        penm->m_dwUser = 0;
	}
    ReaderInfo *prnfo = (ReaderInfo *)penm->m_pvNext;
    if ((int)penm->m_dwUser < prnfo->cEntries) {
        DirEntry *pdirT = &prnfo->pdir[penm->m_dwUser];
        strncpyz(pszFn, pdirT->szFn, cb);
        penm->m_dwUser++;
        return true;
    }
	return false;
}

void *PackFileReader::MapFile(const char *pszFn, FileMap *pfmap, dword *pcb)
{
    memset(pfmap, 0, sizeof(*pfmap));

	// Find the file

	ReaderInfo *prnfo;
	DirEntry dir;
	if (!FindDirEntry(pszFn, &dir, &prnfo))
		return NULL;
	word nRecFirst = BigWord(dir.nRecFirst);

    // For now don't allow mapping of multi-record files. If this is needed
    // later it can be done.

    byte *pb = NULL;
	if (dir.cRecs > 1) {
        File *pfil = this->fopen(pszFn, "rb");
        if (pfil == NULL) {
            return NULL;
        }
        this->fseek(pfil, 0, SEEK_END);
        dword cb = ftell(pfil);
        this->fseek(pfil, 0, SEEK_SET);
        pb = new byte[cb];
        if (pb == NULL) {
            this->fclose(pfil);
            return NULL;
        }
        if (this->fread(pb, cb, 1, pfil) != 1) {
            this->fclose(pfil);
            return NULL;
        }
        pfmap->prnfo = prnfo;
        pfmap->pvCookie = 0;
        pfmap->nRec = 0;
        pfmap->pbAlloced = pb;
        if (pcb != NULL) {
            *pcb = cb;
        }
        this->fclose(pfil);
    } else {
        // See if pdbReader will map this entry
        
        word cbRec;
        void *pvCookie;
        pb = prnfo->ppdbReader->MapRecord(nRecFirst, &pvCookie, &cbRec);
        if (pb == NULL)
            return NULL;

        // All is ok.

        pfmap->prnfo = prnfo;
        pfmap->nRec = nRecFirst;
        pfmap->pvCookie = pvCookie;
        pfmap->pbAlloced = NULL;
        if (pcb != NULL)
            *pcb = (dword)cbRec;
    }
	prnfo->cOpen++;

#if defined(INCL_RECORDOPENS)
	RecordOpen("MapFile", pszFn, (dword)pfmap);
#endif

	return pb;
}

void PackFileReader::UnmapFile(FileMap *pfmap)
{
#if defined(INCL_RECORDOPENS)
	RecordClose((dword)pfmap);
#endif

	Assert(pfmap->prnfo->cOpen > 0);
	pfmap->prnfo->cOpen--;
    if (pfmap->pbAlloced != NULL) {
        delete[] pfmap->pbAlloced;
        pfmap->pbAlloced = NULL;
    } else {
        pfmap->prnfo->ppdbReader->UnmapRecord(pfmap->nRec, pfmap->pvCookie);
    }
}

bool PackFileReader::HashFile(const char *pszFn, byte *hash)
{
    dword cb;
    FileMap fmap;
    byte *pb = (byte *)MapFile(pszFn, &fmap, &cb);
    if (pb == NULL) {
        return false;
    }
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, pb, cb);
    MD5Final(hash, &ctx);
    UnmapFile(&fmap);
    return true;
}

bool PackFileReader::GetPdbName(const char *pszFn, char *pszPdb, int cbPdb,
        char *pszDir, int cbDir)
{
	ReaderInfo *prnfo;
	DirEntry dir;
	if (!FindDirEntry(pszFn, &dir, &prnfo))
		return false;
    strncpyz(pszPdb, prnfo->pszFn, cbPdb);
    if (pszDir != NULL) {
        strncpyz(pszDir, prnfo->pszDir, cbDir);
    }
    return true;
}

bool PackFileReader::FindDirEntry(const char *psz, DirEntry *pdir,
        ReaderInfo **pprnfo) {
    // Convert to lower case

	char szT[kcbFilename];

	// Lower case apis differ platform to platform, so fudge it

	strcpy(szT, psz);
	char *pszT = szT;
	while (*pszT != 0) {
		if (*pszT >= 'A' && *pszT <= 'Z')
			(*pszT) += 'a' - 'A';
		pszT++;
	}

	// The most recently pushed PdbReader has priority

	for (int n = m_crnfo - 1; n >= 0; n--) {
		ReaderInfo *prnfo = &m_arnfo[n];

		// Look through the directory entries for this file. The entries
		// are sorted in ascending order, so use a binary search.

		int nMin = 0;
		int nMax = prnfo->cEntries - 1;
		while (true) {
			if (nMax < nMin)
				break;
			int nCurrent = nMin + (nMax - nMin) / 2;
			DirEntry *pdirT = &prnfo->pdir[nCurrent];

			int n = strcmp(szT, pdirT->szFn);
			if (n == 0) {
				*pdir = *pdirT;
				*pprnfo = prnfo;
				return true;
			}
			if (n > 0) {
				nMin = nCurrent + 1;
				continue;
			}
			if (n < 0) {
				nMax = nCurrent - 1;
				continue;
			}
		}
	}
	
	return false;
}

#define kcrnfoAlloc 25

bool PackFileReader::Push(const char *pszDir, const char *pszFn,
        PdbReader *ppdbReader)
{
	// Grow as necessary

	if (m_crnfo == m_crnfoAlloc) {
		int crnfoAllocNew = m_crnfoAlloc + kcrnfoAlloc;
		ReaderInfo *prnfo = new ReaderInfo[crnfoAllocNew];
		if (prnfo == NULL)
			return false;
		if (m_arnfo != NULL) {
			memcpy(prnfo, m_arnfo, m_crnfo * sizeof(ReaderInfo));
			delete m_arnfo;
		}
		m_arnfo = prnfo;
		m_crnfoAlloc = crnfoAllocNew;
	}

	// First try to map the directory record

	word cb;
	void *pvCookie;
	DirEntry *pdir = (DirEntry *)ppdbReader->MapRecord(0, &pvCookie, &cb);

	if (pdir == NULL)
		return false;

	// That worked. Add this pdb to the list

	ReaderInfo *prnfo = &m_arnfo[m_crnfo];
	m_crnfo++;
	prnfo->ppdbReader = ppdbReader;
	prnfo->pvCookie = pvCookie;
	prnfo->pdir = pdir;
	prnfo->cEntries = cb / sizeof(DirEntry);
	prnfo->cOpen = 0;

    // If pszDir is NULL, split pszFn into directory and filename.
    // prnfo->pszFn must just point to the basename.
	if (pszDir == NULL) {
        char *pszBasename = basename((char *)pszFn);
        int cchBase = (int)strlen(pszBasename);
        int cchFn = (int)strlen(pszFn);
        prnfo->pszDir = new char[cchFn - cchBase + 1];
        strncpyz(prnfo->pszDir, pszFn, cchFn - cchBase + 1);
        int cch = (int)strlen(prnfo->pszDir);
        if (cch >= 1 && prnfo->pszDir[cch - 1] == '/') {
            prnfo->pszDir[cch - 1] = 0;
        }
        pszFn = pszBasename;
	} else {
		prnfo->pszDir = new char[strlen(pszDir) + 1];
		strcpy(prnfo->pszDir, pszDir);
	}

	prnfo->pszFn = new char[strlen(pszFn) + 1];
	strcpy(prnfo->pszFn, pszFn);

	return true;
}

bool PackFileReader::Pop()
{
	// if the sound data file is missing, we can encounter this case

	if (m_crnfo == 0)
		return false;
	RemoveReader(m_crnfo - 1);
	return true;
}

void PackFileReader::RemoveReader(int rnfo)
{
	ReaderInfo *prnfo = &m_arnfo[rnfo];
	Assert(prnfo->cOpen == 0);
	Assert(m_crnfo != 0);

	// Free the directory record and close the pdb reader

	delete[] prnfo->pszDir;
	delete[] prnfo->pszFn;
	prnfo->ppdbReader->UnmapRecord(0, prnfo->pvCookie);
	prnfo->ppdbReader->Close();
	delete prnfo->ppdbReader;

	// Move readers down

	memcpy(&m_arnfo[rnfo], &m_arnfo[rnfo + 1],
            (m_crnfo - 1 - rnfo) * ELEMENTSIZE(m_arnfo));
	m_crnfo--;
}

bool PackFileReader::Delete(const char *pszDir, const char *pszFn)
{
	// See if pszFn is pushed. If so is it in use?
	// If pushed and in use, can't delete

	for (int n = 0; n < m_crnfo; n++) {
		ReaderInfo *prnfo = &m_arnfo[n];
		if (pszDir != NULL) {
			if (stricmp(pszDir, prnfo->pszDir) != 0)
				continue;
		}
		if (stricmp(pszFn, prnfo->pszFn) != 0)
			continue;
		if (prnfo->cOpen != 0)
			return false;
		RemoveReader(n);
		break;
	}

	// Reader removed. Now the file needs to be deleted

    return DeletePdb(pszDir, pszFn);
}

bool PackFileReader::Push(const char *pszDir, const char *pszFn)
{
	PdbReader *ppdbReader = OpenPdb(pszDir, pszFn);
    if (ppdbReader == NULL) {
        return false;
    }

	if (!Push(pszDir, pszFn, ppdbReader)) {
		ppdbReader->Close();
		delete ppdbReader;
		return false;
	}

    return true;
}

} // namespace wi
