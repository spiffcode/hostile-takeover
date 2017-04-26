#include "mpshared/indexloader.h"
#include "mpshared/mpht.h"
#include "base/format.h"
#include "yajl/wrapper/jsonbuilder.h"

namespace wi {

IndexLoader::IndexLoader() {
}

bool IndexLoader::InitFromFile(const char *indexfile) {
    FILE *file = fopen(indexfile, "r");
    if (file == NULL) {
        return false;
    }

    index_.clear();
    json::JsonBuilder builder;
    builder.Start(this);
    while (true) {
        char ach[512];
        int cb = (int)fread(ach, 1, sizeof(ach), file);
        if (cb == 0) {
            if (ferror(file)) {
                fclose(file);
                return false;
            }
            break;
        }
        if (!builder.Update(ach, cb)) {
            fclose(file);
            return false;
        }
    }
    builder.End();
    fclose(file);
    return true;
}

int IndexLoader::GetCount() {
    return (int)index_.size();
}

const IndexEntry *IndexLoader::GetEntry(int i) {
    if (i < 0 || i >= (int)index_.size()) {
        return NULL;
    }
    return &index_[i];
}

void IndexLoader::OnObject(json::JsonObject *obj) {
    // Array order is:
    // 1. pack id
    // 2. pack hash
    // 3. title
    // 4. min player count
    // 5. max player count
    // 6. mission count

    int cExpected = 6;

    // Must be array of strings.
    bool error = false;
    if (obj->type() != json::JSONTYPE_ARRAY) {
        error = true;
    }
    const json::JsonArray *a = (const json::JsonArray *)obj;
    if (!error) {
        if (a->GetCount() != cExpected) {
            error = true;
        }
    }
    if (!error) {
        for (int i = 0; i < a->GetCount(); i++) {
            if (a->GetObject(i)->type() != json::JSONTYPE_STRING) {
                //error = true;
            }
        }
    }

    IndexEntry entry;
    const json::JsonString *s;
    const json::JsonNumber *n;

    // id
    if (!error) {
        if (a->GetObject(0)->type() != json::JSONTYPE_NUMBER) {
            error = true;
        } else {
            n = (const json::JsonNumber *)a->GetObject(0);
            entry.packid.id = n->GetInteger();
        }
    }

    // hash
    if (!error) {
        if (a->GetObject(1)->type() != json::JSONTYPE_STRING) {
            error = true;
        } else {
            s = (const json::JsonString *)a->GetObject(1);
            if (s->GetLength() != 32) {
                error = true;
            }
            if (!base::Format::FromHex(s->GetString(), entry.packid.hash, sizeof(entry.packid.hash))) {
                error = true;
            }
        }
    }

    // title
    if (!error) {
        if (a->GetObject(2)->type() != json::JSONTYPE_STRING) {
            error = true;
        } else {
            s = (const json::JsonString *)a->GetObject(2);
            entry.title = s->GetString();
        }
    }

    // min players
    if (!error) {
        if (a->GetObject(3)->type() != json::JSONTYPE_NUMBER) {
            error = true;
        } else {
            n = (const json::JsonNumber *)a->GetObject(3);
            entry.cPlayersMin = n->GetInteger();
        }
    }

    // max players
    if (!error) {
        if (a->GetObject(4)->type() != json::JSONTYPE_NUMBER) {
            error = true;
        } else {
            n = (const json::JsonNumber *)a->GetObject(4);
            entry.cPlayersMax = n->GetInteger();
        }
    }

    // mission count
    if (!error) {
        if (a->GetObject(5)->type() != json::JSONTYPE_NUMBER) {
            error = true;
        } else {
            n = (const json::JsonNumber *)a->GetObject(5);
            entry.cMissions = n->GetInteger();
        }
    }
    delete obj;

    if (error) {
        OnParseError();
        return;
    }

    entry.inIndex = true;
    index_.push_back(entry);
}

void IndexLoader::OnParseError() {
}

} // namespace wi
