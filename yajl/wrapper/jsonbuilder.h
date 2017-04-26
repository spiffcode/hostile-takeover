#ifndef __JSONBUILDER_H__
#define __JSONBUILDER_H__

#include "inc/basictypes.h"
#include "yajl/api/yajl_parse.h"
#include "yajl/wrapper/jsontypes.h"

namespace json {

class JsonBuilder {
public:
    JsonBuilder();
    ~JsonBuilder();

    class ArrayItemsCallback {
    public:
        virtual void OnObject(JsonObject *obj) = 0;
    };
    bool Start(ArrayItemsCallback *callback = NULL);
    bool Update(const char *pch, int cb);
    JsonObject *End();
    void Reset();

    int OnString(const char *pch, int cb);
    int OnNumber(const char *pch, int cb);
    int OnBool(int value);
    int OnKey(const char *pch, int cb);
    int OnMapStart();
    int OnMapEnd();
    int OnArrayStart();
    int OnArrayEnd();

private:
    int CombineItem(JsonObject *obj);

    ArrayItemsCallback *callback_;
    yajl_handle handle_;
    bool alloced_;
    std::vector<JsonObject *> stack_;
};
json::JsonString *NewJsonString(const char *ach, int cb);

} // namespace json

#endif // __JSONBUILDER_H__
