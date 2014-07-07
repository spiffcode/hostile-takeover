#include "yajl/wrapper/jsonbuilder.h"
#include "base/log.h"
#include <stdio.h>

class JsonBuilderTest : json::JsonBuilder::ArrayItemsCallback {
public:
    bool DoTest(bool fCallback, const char *filename) {
        json::JsonBuilder builder;
        builder.Start(fCallback ? this : NULL);

        FILE *fd = fopen(filename, "rb");
        if (fd == NULL) {
            return false;
        }
        char ach[32];
        while (true) {
            size_t cb = fread(ach, 1, sizeof(ach), fd);
            if (cb == 0) {
                if (ferror(fd)) {
                    fclose(fd);
                    return false;
                }
                if (feof(fd)) {
                    break;
                }
            }
            builder.Update(ach, cb);
        }
        json::JsonObject *obj = builder.End();
        if (obj != NULL) {
            LOG() << "Object returned from JsonBuilder::End()";
            DumpObject(obj);
            delete obj;
        }
        fclose(fd);
        return true;
    }

private:
    void OnObject(const json::JsonObject *obj) {
        LOG() << "OnObject called with object";
        DumpObject(obj);
        delete obj;
    }

    void DumpObject(const json::JsonObject *obj) {
        std::string s = obj->ToString();
        LOG() << base::Log::Format("Object Dump: %s", s.c_str());
    }
};

int main(int argc, char **argv) {
    if (argc != 3) {
        LOG() << base::Log::Format("usage: %s <0|1> <filename>", argv[0]);
        exit(1);
    }
    bool fSuccess = false;
    JsonBuilderTest test;
    if (strcmp(argv[1], "0") == 0) {
        LOG() << base::Log::Format("Test no callback on %s", argv[2]);
        fSuccess = test.DoTest(false, argv[2]);
    }
    if (strcmp(argv[1], "1") == 0) {
        LOG() << base::Log::Format("Test callback on %s\n", argv[2]);
        fSuccess = test.DoTest(true, argv[2]);
    }
    if (fSuccess) {
        LOG() << "tests successful.";
        return 0;
    } else {
        LOG() << "tests failed.";
        return 1;
    }
}
