#ifndef __INDEXLOADER_H__
#define __INDEXLOADER_H__

#include "inc/basictypes.h"
#include "mpshared/mpht.h"
#include "yajl/wrapper/jsonbuilder.h"

namespace wi {

struct IndexEntry {
    PackId packid;
    std::string title;
    int cPlayersMin;
    int cPlayersMax;
    int cMissions;
    bool inIndex;
};

class IndexLoader : protected json::JsonBuilder::ArrayItemsCallback { // idxl
public:
    IndexLoader();
    virtual ~IndexLoader() {}

    bool InitFromFile(const char *indexfile);
    int GetCount();
    const IndexEntry *GetEntry(int i);

protected:
    virtual void OnParseError();

    // ArrayItemsCallback
    virtual void OnObject(json::JsonObject *obj);

    typedef std::vector<IndexEntry> Index;
    Index index_;
};

} // namespace wi

#endif // __INDEXLOADER_H__
