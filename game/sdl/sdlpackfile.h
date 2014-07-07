#ifndef __SDLPACKFILE_H__
#define __SDLPACKFILE_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"

namespace wi {

class SdlPackFileReader : public PackFileReader
{
private:
    virtual PdbReader *OpenPdb(const char *pszDir, const char *pszFn);
    virtual bool DeletePdb(const char *pszDir, const char *pszFn);
};

} // namespace wi

#endif // __SDLPACKFILE_H__
