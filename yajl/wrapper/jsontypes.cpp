#include "yajl/wrapper/jsontypes.h"

namespace json {

//
// JsonObject
//

JsonObject::~JsonObject() {
}

#ifdef RELEASE_LOGGING
std::string JsonObject::ToString() const {
    return base::Log::Format("JsonObject <0x%08lx>", this);
}
#endif

//
// JsonString
//

JsonString::~JsonString() {
}

JsonString::JsonString(const char *pch, int cb) : JsonObject(JSONTYPE_STRING) {
    if (cb < 0) {
        s_ = std::string(pch);
    } else {
        s_ = std::string(pch, cb);
    }
}

int JsonString::GetLength() const {
    return s_.length();
}

const char *JsonString::GetString() const {
    return s_.c_str();
}

#ifdef RELEASE_LOGGING
std::string JsonString::ToString() const {
    return base::Log::Format("JsonString: %s", s_.c_str());
}
#endif

//
// JsonMap
//

JsonMap::~JsonMap() {
    while (map_.size() != 0) {
        KeyMap::iterator it = map_.begin();
        delete it->second;
        map_.erase(it);
    }
}

const JsonObject *JsonMap::GetObject(const char *key) const {
    KeyMap::const_iterator it = map_.find(key);
    if (it == map_.end()) {
        return NULL;
    }
    return it->second;
}

void JsonMap::SetObject(const char *key, JsonObject *obj) {
    // Erase old key of this value, if it exists
    KeyMap::iterator it = map_.find(key);
    if (it != map_.end()) {
        delete it->second;
        map_.erase(it);
    }

    // Insert new
    map_.insert(KeyMap::value_type(key, obj));
}

const char *JsonMap::EnumKeys(wi::Enum *penm) const {
    if (penm->m_dwUser == wi::kEnmFirst) {
        penm->m_dwUser = 0;
    }
    KeyMap::const_iterator it = map_.begin();
    for (int i = 0; it != map_.end(); i++, it++) {
        if (penm->m_dwUser == i) {
            penm->m_dwUser++;
            return it->first.c_str();
        }
    }
    return NULL;
}

#ifdef RELEASE_LOGGING
std::string JsonMap::ToString() const {
    std::string s = "JsonMap: { ";
    wi::Enum enm;
    const char *key;
    while ((key = EnumKeys(&enm)) != NULL) {
        const JsonObject *obj = GetObject(key);
        s += base::Log::Format("%s : %s, ", key, obj->ToString().c_str());
    }
    s += "}";
    return s;
}
#endif

//
// JsonArray
//

JsonArray::~JsonArray() {
    while (list_.size() != 0) {
        delete list_[list_.size() - 1];
        list_.pop_back();
    }
}

int JsonArray::GetCount() const {
    return (int)list_.size();
}

const JsonObject *JsonArray::GetObject(int i) const {
    if (i < 0 || i > (int)list_.size()) {
        return NULL;
    }
    return list_[i];
}

bool JsonArray::AddObject(const JsonObject *obj) {
    list_.push_back(obj);
    return true;
}

#ifdef RELEASE_LOGGING
std::string JsonArray::ToString() const {
    std::string s = "JsonArray: [ ";
    for (int i = 0; i < GetCount(); i++) {
        const JsonObject *obj = GetObject(i);
        s += base::Log::Format("%s, ", obj->ToString().c_str());
    }
    s += "]";
    return s;
}
#endif

} // namespace json
