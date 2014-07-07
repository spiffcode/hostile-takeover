#include "game/map.h"

namespace wi {

bool Map::GetValue(const char *pszKey, char *pszValue,
        int cbValue) const {

    // Find key
    KeyMap::const_iterator it = map_.find(pszKey);
    if (it == map_.end()) {
        return false;
    }
    strncpyz(pszValue, it->second.c_str(), cbValue);
    return true;
}

bool Map::SetValue(const char *pszKey, const char *pszValue) {
    // Erase old key of this value, if it exists
    KeyMap::iterator it = map_.find(pszKey);
    if (it != map_.end()) {
        map_.erase(it);
    }

    // Insert new
    map_.insert(KeyMap::value_type(pszKey, pszValue));
    return true;
}

bool Map::EnumKeys(Enum *penm, char *pszKey, int cbKey) const {
    if (penm->m_dwUser == kEnmFirst) {
        penm->m_dwUser = 0;
    }

    KeyMap::const_iterator it = map_.begin();
    for (int i = 0; it != map_.end(); i++, it++) {
        if (penm->m_dwUser == i) {
            strncpyz(pszKey, it->first.c_str(), cbKey);
            penm->m_dwUser++;
            return true;
        }
    }
    return false;
}

} // namespace wi
