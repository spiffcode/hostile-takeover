#include "mpshared/packinfomanager.h"
#include "mpshared/misc.h"
#include "base/format.h"
#include "yajl/wrapper/jsonbuilder.h"

namespace wi {

PackInfoManager::PackInfoManager(const char *cachedir) {
    cachedir_ = cachedir;
}

PackInfoManager::~PackInfoManager() {
}

bool PackInfoManager::GetPackId(json::JsonMap *map, PackId *ppackid) {
    const char *id = GetString(map, "id");
    if (id == NULL) {
        return false;
    }
    const char *hash = GetString(map, "h");
    if (hash == NULL) {
        return false;
    }
    if (!base::Format::FromHex(hash, ppackid->hash, sizeof(ppackid->hash))) {
        return false;
    }
    if (!base::Format::ToDword(id, 10, &ppackid->id)) {
        return false;
    }
    return true;
}

const char *PackInfoManager::GetString(const json::JsonMap *map,
        const char *key) {
    const json::JsonObject *obj = map->GetObject(key);
    if (obj == NULL)
        return NULL;
    if (obj->type() == json::JSONTYPE_STRING)
        return ((const json::JsonString *)obj)->GetString();
    if (obj->type() == json::JSONTYPE_NUMBER)
        return ((const json::JsonNumber *)obj)->GetString();
    return NULL;
}

json::JsonMap *PackInfoManager::GetInfoMap(const PackId *ppackid) {
    FILE *file = fopen(GetInfoFilename(ppackid), "rb");
    if (file == NULL) {
        return NULL;
    }
    json::JsonMap *map = LoadInfoMap(file);
    fclose(file);
    return map;
}

json::JsonMap *PackInfoManager::LoadInfoMap(FILE *file) {
    fseek(file, 0, SEEK_SET);
    json::JsonBuilder builder;
    builder.Start(NULL);
    byte ab[256];
    while (true) {
        int cb = (int)fread(ab, 1, sizeof(ab), file);
        if (cb == 0) {
            break;
        }
        builder.Update((const char *)ab, cb);
    }
    json::JsonObject *obj = builder.End();
    if (obj == NULL || obj->type() != json::JSONTYPE_MAP) {
        delete obj;
        return false;
    }
    return (json::JsonMap *)obj;
}

const char *PackInfoManager::GetInfoFilename(const PackId *ppackid) {
    char szHash[33];
    strncpyz(szHash, base::Format::ToHex(ppackid->hash, 16), sizeof(szHash));
    return base::Format::ToString("%s/%08lx-%s", cachedir_.c_str(),
            ppackid->id, szHash);
}

} // namespace wi
