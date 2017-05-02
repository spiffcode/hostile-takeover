#ifndef __SDLPACKFILE_H__
#define __SDLPACKFILE_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"
#include <stdio.h>
#include <vector>

namespace wi {

struct SdlReaderInfo {
    char *pszDir;
    char *pszPdb;
};

class SdlPackFileReader : public PackFileReader
{
public:
	~SdlPackFileReader();

    virtual File *fopen(const char *pszFn, const char *pszMode);
	virtual int fclose(File *pfil);
	virtual dword fread(void *pv, dword cb, int c, File *pfil);
	virtual int fseek(File *pfil, int off, int nOrigin);
	virtual dword ftell(File *pfil);

    virtual bool EnumFiles(Enum *penm, int key, char *pszFn, int cbFn);
    virtual void *MapFile(const char *pszFn, FileMap *pfmap, dword *pcb = NULL);
	virtual void UnmapFile(FileMap *pfmap);

    virtual bool Push(const char *pszDir, const char *pszFn);
	virtual bool Pop();

    char *BottomDir();
    char *BottomPdb();

private:
    virtual PdbReader *OpenPdb(const char *pszDir, const char *pszFn);
    virtual bool DeletePdb(const char *pszDir, const char *pszFn);

    std::vector<SdlReaderInfo *> m_vrnfo;
};

} // namespace wi

#endif // __SDLPACKFILE_H__
