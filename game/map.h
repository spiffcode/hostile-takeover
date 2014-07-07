#ifndef __MAP_H__
#define __MAP_H__

#include "inc/basictypes.h"
#include "mpshared/enum.h"
#include "mpshared/misc.h"
#include <map>
#include <string>

namespace wi {

class Map {
public:
    bool GetValue(const char *pszKey, char *pszValue, int cbValue) const;
    bool SetValue(const char *pszKey, const char *pszValue);
    bool EnumKeys(Enum *penm, char *pszKey, int cbKey) const;

protected:
    typedef std::map<std::string, std::string> KeyMap;
    KeyMap map_;
};

} // namespace wi

#endif // __MAP_H__
