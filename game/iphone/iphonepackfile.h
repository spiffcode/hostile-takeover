#ifndef __IPHONEPACKFILE_H__
#define __IPHONEPACKFILE_H__

#include "inc/basictypes.h"
#include "mpshared/packfile.h"

namespace wi {

class IPhonePackFileReader : public PackFileReader
{
private:
    virtual PdbReader *OpenPdb(const char *pszDir, const char *pszFn);
    virtual bool DeletePdb(const char *pszDir, const char *pszFn);
};

} // namespace wi

#endif // __IPHONEPACKFILE_H__
