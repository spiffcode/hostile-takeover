#ifndef __NCPACKFILE_H__
#define __NCPACKFILE_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"
#include "mpshared/pdbreader.h"

namespace wi {

class NoCachePackFileReader : public PackFileReader
{
private:
    virtual PdbReader *OpenPdb(const char *pszDir, const char *pszFn);
    virtual bool DeletePdb(const char *pszDir, const char *pszFn);
};

} // namespace wi

#endif // __NCPACKFILE_H__
