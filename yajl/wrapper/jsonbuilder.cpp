#include <string.h>
#include "yajl/wrapper/jsonbuilder.h"
#include <string.h>

namespace json {

// yajl callbacks

int yajl_null(void *ctx) {
    // Don't accept
    return 0;
}

int yajl_boolean(void *ctx, int value) {
    // Don't accept
    return 0;
}

int yajl_number(void *ctx, const char *pch, unsigned int cb) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnString((const char *)pch, cb);
}

int yajl_string(void *ctx, const unsigned char *pch, unsigned int cb) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnString((const char *)pch, cb);
}

int yajl_map_key(void *ctx, const unsigned char *pch, unsigned int cb) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnKey((const char *)pch, cb);
}

int yajl_start_map(void *ctx) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnMapStart();
}

int yajl_end_map(void *ctx) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnMapEnd();
}

int yajl_start_array(void *ctx) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnArrayStart();
}

int yajl_end_array(void *ctx) {
    JsonBuilder *builder = (JsonBuilder *)ctx;
    return builder->OnArrayEnd();
}

static yajl_callbacks callbacks = {
    yajl_null,
    yajl_boolean,
    NULL,
    NULL,
    yajl_number,
    yajl_string,
    yajl_start_map,
    yajl_map_key,
    yajl_end_map,
    yajl_start_array,
    yajl_end_array
};

//
// JsonBuilder implementation
//

JsonBuilder::JsonBuilder() {
    callback_ = NULL;
    alloced_ = false;
}

JsonBuilder::~JsonBuilder() {
    Reset();
}

void JsonBuilder::Reset() {
    while (stack_.size() > 0) {
        JsonObject *obj = stack_[stack_.size() - 1];
        stack_.pop_back();
        delete obj;
    }
    if (alloced_) {
        yajl_free(handle_);
        alloced_ = false;
    }
}

bool JsonBuilder::Start(ArrayItemsCallback *callback) {
    Reset();
    callback_ = callback;
    yajl_parser_config cfg = { 0, 1 };
    handle_ = yajl_alloc(&callbacks, &cfg, this);
    alloced_ = true;
    return true;
}

bool JsonBuilder::Update(const char *pch, int cb) {
    yajl_status stat = yajl_parse(handle_, (const unsigned char *)pch, cb);
    if (stat != yajl_status_insufficient_data &&
            stat != yajl_status_ok) {
        return false;
    }
    return true;
}

JsonObject *JsonBuilder::End() {
    JsonObject *obj = NULL;
    if (stack_.size() == 1) {
        obj = stack_[0];
        stack_.pop_back();
    }
    Reset();
    if (callback_ == NULL) {
        return obj;
    }
    delete obj;
    return NULL;
}

json::JsonString *JsonBuilder::NewJsonString(const char *ach, int cb) {
    char szT[1024];
    char *pchT = szT;
    char *pchTerm = &szT[sizeof(szT) - 1];

    const char *apszEnt[] = {
        "&amp;", "&#039;", "&quot;", "&lt;", "&gt;" 
    };
    int acchEnt[] = {
        5, 6, 6, 4, 4
    };
    char achRep[] = {
        '&', '\'', '\"', '<', '>'
    };

    for (int i = 0; i < cb && pchT < pchTerm; i++) {
        char ch = ach[i];
        if (ch == '&') {
            bool fReplaced = false;
            for (int n = 0; n < ARRAYSIZE(apszEnt); n++) {
                if (strncmp(&ach[i], apszEnt[n], acchEnt[n]) == 0) {
                    *pchT++ = achRep[n];
                    i += acchEnt[n] - 1;
                    fReplaced = true;
                    break;
                }
            }
            if (fReplaced) {
                continue;
            }
        }

        *pchT++ = ch;
    }
    *pchT = 0;
    return new JsonString(szT);
}

int JsonBuilder::OnString(const char *pch, int cb) {
    CombineItem(NewJsonString(pch, cb));
    return 1;
}

int JsonBuilder::OnKey(const char *pch, int cb) {
    int last = (int)stack_.size() - 1;
    if (last < 0 || stack_[last]->type() != JSONTYPE_MAP) {
        return 0;
    }
    stack_.push_back(NewJsonString(pch, cb));
    return 1;
}

int JsonBuilder::OnMapStart() {
    stack_.push_back(new JsonMap());
    return 1;
}

int JsonBuilder::OnMapEnd() {
    int last = (int)stack_.size() - 1;
    if (last < 0 || stack_[last]->type() != JSONTYPE_MAP) {
        return 0;
    }
    JsonObject *obj = stack_[last];
    stack_.pop_back();
    CombineItem(obj);
    return 1;
}

int JsonBuilder::OnArrayStart() {
    stack_.push_back(new JsonArray());
    return 1;
}

int JsonBuilder::OnArrayEnd() {
    int last = (int)stack_.size() - 1;
    if (last < 0 || stack_[last]->type() != JSONTYPE_ARRAY) {
        return 0;
    }
    JsonObject *obj = stack_[last];
    stack_.pop_back();

    if (stack_.size() == 1 && stack_[0]->type() == JSONTYPE_ARRAY &&
            callback_ != NULL) {
        callback_->OnObject(obj);
    } else {
        CombineItem(obj);
    }
    return 1;
}

int JsonBuilder::CombineItem(JsonObject *obj) {
    int last = (int)stack_.size() - 1;
    if (last >= 0 && stack_[last]->type() == JSONTYPE_ARRAY) {
        JsonArray *array = (JsonArray *)stack_[last];
        array->AddObject(obj);
        return 1;
    }
    if (last >= 1 && stack_[last]->type() == JSONTYPE_STRING &&
            stack_[last - 1]->type() == JSONTYPE_MAP) {
        JsonMap *map = (JsonMap *)stack_[last - 1];
        JsonString *key = (JsonString *)stack_[last];
        map->SetObject(key->GetString(), obj);
        delete key;
        stack_.pop_back();
        return 1;
    }
    stack_.push_back(obj);
    return 1;
}

} // namespace json
