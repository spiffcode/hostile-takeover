#ifndef __WINPACKFILE_H__
#define __WINPACKFILE_H__

#include "inc/basictypes.h"

namespace wi {

class WinPackFileReader : public PackFileReader
{
private:
    virtual PdbReader *OpenPdb(char *pszDir, char *pszFn);
    virtual bool DeletePdb(char *pszDir, char *pszFn);
};

} // namespace wi

#endif // __WINPACKFILE_H__
