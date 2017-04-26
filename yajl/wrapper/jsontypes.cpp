#include "yajl/wrapper/jsontypes.h"
#include "base/format.h"
#include "mpshared/enum.h"
#include "mpshared/misc.h"
#include <math.h>
#include <stdlib.h>

namespace json {

//
// JsonObject
//

JsonObject::~JsonObject() {
}

#ifdef RELEASE_LOGGING
std::string JsonObject::ToString() const {
    return base::Log::Format("JsonObject <0x%p>", this);
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
    return (int)s_.length();
}

const char *JsonString::GetString() const {
    return s_.c_str();
}

const char *JsonString::ToJson() const {
    return base::Format::ToString("\"%s\"", s_.c_str());
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

int JsonMap::GetInteger(const char *key) const {
    json::JsonObject *o = (json::JsonObject *)GetObject(key);
    if (o == NULL)
        return -1;
    if (o->type() != json::JSONTYPE_NUMBER)
        return -1;
    json::JsonNumber *n = (json::JsonNumber *)o;
    if (n == NULL) {
        return -1;
    }
    return n->GetInteger();
}

double JsonMap::GetFloat(const char *key) const {
    json::JsonObject *o = (json::JsonObject *)GetObject(key);
    if (o == NULL)
        return -1;
    if (o->type() != json::JSONTYPE_NUMBER)
        return -1;
    json::JsonNumber *n = (json::JsonNumber *)o;
    if (n == NULL) {
        return -1;
    }
    return n->GetFloat();
}

const char *JsonMap::GetString(const char *key) const {
    json::JsonObject *o = (json::JsonObject *)GetObject(key);
    if (o == NULL)
        return "";
    if (o->type() != json::JSONTYPE_STRING)
        return "";
    json::JsonString *s = (json::JsonString *)o;
    if (s == NULL) {
        return "";
    }
    return s->GetString();
}

bool JsonMap::GetBool(const char *key) const {
    json::JsonObject *o = (json::JsonObject *)GetObject(key);
    if (o == NULL)
        return false;
    if (o->type() != json::JSONTYPE_BOOL)
        return false;
    json::JsonBool *b = (json::JsonBool *)o;
    if (b == NULL) {
        return false;
    }
    return b->GetBool();
}

const char *JsonMap::ToJson() const {
    std::string j = "{ ";
    wi::Enum enm;
    const char *key;
    while ((key = EnumKeys(&enm)) != NULL) {

        JsonObject *obj = (JsonObject *)GetObject(key);
        if (obj == NULL)
            continue;

        std::string s = "";
        switch (obj->type()) {
            case JSONTYPE_MAP:
                s = ((JsonMap *)obj)->ToJson();
                break;
            case JSONTYPE_ARRAY:
                s = ((JsonArray *)obj)->ToJson();
                break;
            case JSONTYPE_STRING:
                s = ((JsonString *)obj)->ToJson();
                break;
            case JSONTYPE_NUMBER:
                if (((JsonNumber *)obj)->IsInteger())
                    s = base::Format::ToString("%i", ((JsonNumber *)obj)->GetInteger());
                else
                    s = base::Format::ToString("%f", ((JsonNumber *)obj)->GetFloat());
                break;
            case JSONTYPE_BOOL:
                s = ((JsonBool *)obj)->GetString();
                break;
            default:
                break;
        }

        j += base::Format::ToString("\"%s\": %s, ", key, s.c_str());

    }

    // Remove the last comma and add the closing bracket

    j = j.substr(0, j.size()-2);
    j += " }";

    return j.c_str();
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

const char *JsonArray::ToJson() const {
    std::string j = "[ ";
    for (int i = 0; i < GetCount(); i++) {
        const JsonObject *obj = GetObject(i);
        if (obj == NULL)
            continue;

        std::string s = "";
        switch (obj->type()) {
            case JSONTYPE_MAP:
                s = ((JsonMap *)obj)->ToJson();
                break;
            case JSONTYPE_ARRAY:
                s = ((JsonArray *)obj)->ToJson();
                break;
            case JSONTYPE_STRING:
                s = ((JsonString *)obj)->ToJson();
                break;
            case JSONTYPE_NUMBER:
                if (((JsonNumber *)obj)->IsInteger())
                    s = base::Format::ToString("%i", ((JsonNumber *)obj)->GetInteger());
                else
                    s = base::Format::ToString("%f", ((JsonNumber *)obj)->GetFloat());
                break;
            case JSONTYPE_BOOL:
                s = ((JsonBool *)obj)->GetString();
                break;
            default:
                break;
        }

        j += base::Format::ToString("%s, ", s.c_str());

    }

    // Remove the last comma and add the closing bracket
    
    j = j.substr(0, j.size()-2);
    j += " ]";

    return j.c_str();
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

//
// JsonNumber
//

JsonNumber::~JsonNumber() {
    delete[] psz_;
}

JsonNumber::JsonNumber(const char *pch, int cb) : JsonObject(JSONTYPE_NUMBER) {
    psz_ = new char[cb + 1];
    wi::strncpyz(psz_, pch, cb + 1);
}

JsonNumber::JsonNumber(int n) : JsonObject(JSONTYPE_NUMBER) {

    // Integer division to calulate number of digits

    int cb = 0;
    if (n == 0)
        cb = 1;
    else
        for (int c = n; c != 0; c = c / 10, cb++);

    psz_ = new char[cb + 1];
    sprintf(psz_, "%d", n);
}

JsonNumber::JsonNumber(float n) : JsonObject(JSONTYPE_NUMBER) {
    psz_ = new char[12];
    sprintf(psz_, "%f", n);
}

bool JsonNumber::IsInteger() const {
    if (GetInteger() == GetFloat())
        return true;
    return false;
}

int JsonNumber::GetInteger() const {
    return atoi(psz_);
}

double JsonNumber::GetFloat() const {
    return atof(psz_);
}

const char *JsonNumber::GetString() const {
    std::string str = base::Format::ToString("%s", psz_);
    char *psz = new char[str.length()];
    strcpy(psz, str.c_str());
    return psz;
}

#ifdef RELEASE_LOGGING
std::string JsonNumber::ToString() const {
    return base::Log::Format("JsonNumber: %s", psz_);
}
#endif

//
// JsonBool
//

JsonBool::~JsonBool() {
}

JsonBool::JsonBool(bool f) : JsonObject(JSONTYPE_BOOL) {
    f_ = f;
}

bool JsonBool::GetBool() const {
    return f_;
}

const char *JsonBool::GetString() const {
    return base::Format::ToString(f_ == true ? "true" : "false");
}

#ifdef RELEASE_LOGGING
std::string JsonBool::ToString() const {
    return base::Log::Format("JsonBool: %s", f_ == true ? "true" : "false");
}
#endif

} // namespace json
