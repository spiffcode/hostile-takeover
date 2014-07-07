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
        size_t cb = fread(ach, 1, sizeof(ach), file);
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
    return index_.size();
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
                error = true;
            }
        }
    }

    IndexEntry entry;
    const json::JsonString *s;

    // id
    if (!error) {
        s = (const json::JsonString *)a->GetObject(0);
        if (!base::Format::ToDword(s->GetString(), 10, &entry.packid.id)) {
            error = true;
        }
    }

    // hash
    if (!error) {
        s = (const json::JsonString *)a->GetObject(1);
        if (s->GetLength() != 32) {
            error = true;
        }
        if (!base::Format::FromHex(s->GetString(), entry.packid.hash,
                sizeof(entry.packid.hash))) {
            error = true;
        }
    }

    // title
    if (!error) {
        s = (const json::JsonString *)a->GetObject(2);
        entry.title = s->GetString();
    }

    // min players
    if (!error) {
        s = (const json::JsonString *)a->GetObject(3);
        if (!base::Format::ToInteger(s->GetString(), 10, &entry.cPlayersMin)) {
            error = true;
        }
    }

    // max players
    if (!error) {
        s = (const json::JsonString *)a->GetObject(4);
        if (!base::Format::ToInteger(s->GetString(), 10, &entry.cPlayersMax)) {
            error = true;
        }
    }

    // mission count
    if (!error) {
        s = (const json::JsonString *)a->GetObject(5);
        if (!base::Format::ToInteger(s->GetString(), 10, &entry.cMissions)) {
            error = true;
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
