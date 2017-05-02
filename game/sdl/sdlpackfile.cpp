#include "game/sdl/sdlpackfile.h"
#include "game/mempdbreader.h"
#include "game/sdl/hosthelpers.h"
#include "inc/rip.h"
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <iostream>

namespace wi {

bool FileIsOpen(File *pfil) {
    if (pfil != NULL)
        if (pfil->prnfo != NULL)
            if (pfil->prnfo->cOpen > 0)
                return true;

    return false;
}


SdlPackFileReader::~SdlPackFileReader()
{
    while (Pop())
		;
}

PdbReader *SdlPackFileReader::OpenPdb(const char *pszDir,
        const char *pszFn) {
	// Load data from this directory

	char szT[PATH_MAX];
    if (pszDir != NULL) {
        strcpy(szT, pszDir);
        strcat(szT, "/");
        strcat(szT, pszFn);
    } else if (pszFn != NULL) {
        strcpy(szT, pszFn);
    } else {
        return false;
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

File *SdlPackFileReader::fopen(const char *pszFn, const char *pszMode) {
    if (pszFn == NULL)
        return NULL;

    // Only support read binary mode

	if (strcmp(pszMode, "rb") != 0)
		return NULL;

    // Go through the reader infos in reverse order

    for (std::vector<SdlReaderInfo *>::reverse_iterator it = m_vrnfo.rbegin(); it != m_vrnfo.rend(); it++) {

        // Is this reader a pdb?

        if ((*it)->pszPdb != NULL) {
            if (!strchr(pszFn, '/') && strlen(pszFn) <= kcbFilename) {
                File *pfil = PackFileReader::fopen(pszFn, pszMode);
                if (pfil != NULL)
                    return pfil;
            }

            // If couldn't load from pdb, continute to next reader
            continue;
        }

        // Not pdb, load from dir

        char pszFnFull[PATH_MAX];
        sprintf(pszFnFull, "%s/%s", (*it)->pszDir, pszFn);

        // Open with host I/O

        File *pfil = new File();
        pfil->pvData = HostOpenFile(pszFnFull, kfOfRead);
        if (!pfil->pvData)
            continue;

        // Set prevalent info

        HostSeekFile(pfil->pvData, 0, kfSeekEnd);
        pfil->cbTotal = HostTellFile(pfil->pvData);
        HostSeekFile(pfil->pvData, 0, kfSeekSet);
        
        return pfil;
    }

    LOG() << "can't find file " << pszFn;
    return NULL;
}

int SdlPackFileReader::fclose(File *pfil) {
    if (!FileIsOpen(pfil))
        HostCloseFile(pfil->pvData);

    if (FileIsOpen(pfil))
        PackFileReader::fclose(pfil);
        
    return 0;
}

dword SdlPackFileReader::fread(void *pv, dword cb, int c, File *pfil) {    
    if (FileIsOpen(pfil))
        return PackFileReader::fread(pv, cb, c, pfil);

    dword cbT = cb * c;
    dword cbR = c * HostReadFile(pv, c, cb, pfil->pvData);
    cbT -= cbR;
    return cbT == 0 ? c : cbR / cb;
}

int SdlPackFileReader::fseek(File *pfil, int off, int nOrigin) {
    if (FileIsOpen(pfil))
        return PackFileReader::fseek(pfil, off, nOrigin);

    return HostSeekFile(pfil->pvData, off, nOrigin) != -1 ? 0 : -1;
}

dword SdlPackFileReader::ftell(File *pfil) {
    if (FileIsOpen(pfil))
        return PackFileReader::ftell(pfil);

    return HostTellFile(pfil->pvData);
}

bool SdlPackFileReader::EnumFiles(Enum *penm, int key, char *pszFn, int cbFn) {
    // PACKENUM_LAST will enum the top pdb
    // PACKENUM_FIRST will endum the bottom dir

    if (key == PACKENUM_LAST)
        return PackFileReader::EnumFiles(penm, key, pszFn, cbFn);

    if (key != PACKENUM_FIRST)
        return false;

#if defined(__ANDROID__)
    return HostHelpers::EnumFiles(penm, key, pszFn, cbFn);
#else
    if (penm->m_pvNext == (void *)kEnmFirst) {
        DIR *dir;
        dir = opendir(BottomDir());

        penm->m_pvNext = (void *)dir;
        penm->m_dwUser = 0;
	}

    DIR *dir = (DIR *)penm->m_pvNext;
    struct dirent *ent;
    if ((ent = readdir(dir)) != NULL) {
        strncpyz(pszFn, ent->d_name, cbFn);
        penm->m_dwUser++;
        return true;
    }
    closedir(dir);
#endif

	return false;
}

void *SdlPackFileReader::MapFile(const char *pszFn, FileMap *pfmap, dword *pcb) {
    if (!strchr(pszFn, '/') && strlen(pszFn) <= kcbFilename) {
        void *pv = PackFileReader::MapFile(pszFn, pfmap, pcb);
        if (pv != NULL)
            return pv;
    }

    byte *pb = NULL;
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
    pfmap->pbAlloced = pb;
    if (pcb != NULL) {
        *pcb = cb;
    }
    this->fclose(pfil);

    return pb;
}

void SdlPackFileReader::UnmapFile(FileMap *pfmap) {
    if (pfmap != NULL)
        if (pfmap->prnfo != NULL)
            if (pfmap->prnfo->cOpen > 0)
                PackFileReader::UnmapFile(pfmap);

    if (pfmap->pbAlloced != NULL)
        delete[] pfmap->pbAlloced;
}

bool SdlPackFileReader::Push(const char *pszDir, const char *pszFn) {
    if (pszDir == NULL && pszFn == NULL)
        return false;

    SdlReaderInfo *prnfo = new SdlReaderInfo();

    prnfo->pszDir = NULL;
    if (pszDir != NULL)
        prnfo->pszDir = AllocString(pszDir);

    prnfo->pszPdb = NULL;
    if (pszFn != NULL)
        prnfo->pszPdb = AllocString(pszFn);

    // If we have pszFn then we are pushing a pdb

    if (pszFn != NULL) {
        bool fSuccess = PackFileReader::Push(pszDir, pszFn);
        if (fSuccess)
            m_vrnfo.push_back(prnfo);
        return fSuccess;
    }

    // Push the dir if it exists

#if defined(__ANDROID__)
    wi::HostHelpers::DirExists(prnfo->pszDir);
#else
    DIR *pdir;
    if ((pdir = opendir(pszDir)) == NULL) {
        return false;
    } else {
        closedir(pdir);
    }
#endif

    m_vrnfo.push_back(prnfo);
    return true;
}

bool SdlPackFileReader::Pop() {

    // Anything to pop?

    if (m_vrnfo.empty())
        return false;

    // Pop an actual pdb?

    bool fPdbRet = false;
    bool fPdb = false;
    if (m_vrnfo.back()->pszPdb != NULL) {
        fPdb = true;
        fPdbRet = PackFileReader::Pop();
    }

    // Pdb pop fail?

    if (fPdb && !fPdbRet)
        return false;

    // Either a dir, or a successfully poped pdb, pop it off the vector

    delete[] m_vrnfo.back()->pszDir;
    delete[] m_vrnfo.back()->pszPdb;
    m_vrnfo.pop_back();
    return true;
}

char *SdlPackFileReader::BottomDir() {
    for (std::vector<SdlReaderInfo *>::reverse_iterator it = m_vrnfo.rbegin();
        it != m_vrnfo.rend(); it++) {

        if ((*it)->pszDir != NULL && (*it)->pszPdb == NULL)
            return (*it)->pszDir;
    }
    return NULL;
}

char *SdlPackFileReader::BottomPdb() {
    for (std::vector<SdlReaderInfo *>::reverse_iterator it = m_vrnfo.rbegin();
        it != m_vrnfo.rend(); it++) {

        if ((*it)->pszPdb != NULL)
            return (*it)->pszPdb;
    }
    return NULL;
}

} // namespace wi
