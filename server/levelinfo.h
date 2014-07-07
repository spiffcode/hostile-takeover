#ifndef __LEVELINFO_H__
#define __LEVELINFO_H__

#include "mpshared/mpht.h"
#include "server/ncpackfile.h"
#include "inc/basictypes.h"
#include <string.h>

namespace wi {

class LevelInfo {
public:
    LevelInfo() {
        Clear();
    }

    void Clear() {
        memset(this, 0, sizeof(*this));
    }

    bool Load(PackFileReader& pak, const char *file);

    const char *title() const { return title_; }
    const char *filename() const { return filename_; }
    int minplayers() const { return minplayers_; }
    int maxplayers() const { return maxplayers_; }
    int version() const { return version_; }
    int GetIntelligence(Side side) const {
        if (side >= ARRAYSIZE(intelligences_)) {
            return 0;
        }
        return intelligences_[side];
    }

private:
	char title_[kcbLevelTitle];
	char filename_[kcbFilename];
	int minplayers_;
    int maxplayers_;
	int version_;
	dword revision_;
    int intelligences_[5];
};

} // namespace wi

#endif // __LEVELINFO_H__
