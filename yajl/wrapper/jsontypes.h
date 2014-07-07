#ifndef __JSONTYPES_H__
#define __JSONTYPES_H__

#include "inc/basictypes.h"
#include "mpshared/enum.h"
#include "base/log.h"
#include <string>
#include <map>
#include <vector>

namespace json {

enum JsonType {
    JSONTYPE_OBJECT, JSONTYPE_MAP, JSONTYPE_ARRAY, JSONTYPE_STRING
};

class JsonObject {
public:
    JsonObject(JsonType type) : type_(type) {}
    virtual ~JsonObject();
    int type() const { return type_; }

#ifdef RELEASE_LOGGING
    virtual std::string ToString() const;
#endif

protected:
    JsonType type_;
};

class JsonString : public JsonObject {
public:
    JsonString(const char *pch, int cb = -1);
    virtual ~JsonString();

    int GetLength() const;
    const char *GetString() const;

#ifdef RELEASE_LOGGING
    virtual std::string ToString() const;
#endif

private:
    std::string s_;
};

class JsonMap : public JsonObject {
public:
    JsonMap() : JsonObject(JSONTYPE_MAP) {}
    virtual ~JsonMap();

    const JsonObject *GetObject(const char *key) const;
    void SetObject(const char *key, JsonObject *obj);
    const char *EnumKeys(wi::Enum *penm) const;

#ifdef RELEASE_LOGGING
    virtual std::string ToString() const;
#endif

private:
    typedef std::map<std::string, const JsonObject *> KeyMap;
    KeyMap map_;
};

class JsonArray : public JsonObject {
public:
    JsonArray() : JsonObject(JSONTYPE_ARRAY) {}
    virtual ~JsonArray();

    int GetCount() const;
    const JsonObject *GetObject(int i) const;
    bool AddObject(const JsonObject *obj);

#ifdef RELEASE_LOGGING
    virtual std::string ToString() const;
#endif

private:
    typedef std::vector<const JsonObject *> ObjList;
    ObjList list_;
};

} // namespace json

#endif // __JSONTYPES_H__
