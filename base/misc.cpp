#include "base/misc.h"
#include <ctype.h>
#include <string>

namespace base {

const char *StringEncoder::Replace(const char *s, const char *findstr,
        const char *replacewith) {
    int index = 0;
    std::string out(s);
    while (true) {
        size_t found = out.find(findstr, index);
        if (found == std::string::npos) {
            break;
        }
        out.replace(found, strlen(findstr), replacewith);
        index += found + strlen(replacewith);
    }
    return base::Format::ToString("%s", out.c_str());
}

// Algorithmically identical to python cgi.escape
const char *StringEncoder::Escape(const char *s, bool quote) {
    // html content escaping
    std::string out = Replace(s, "&", "&amp;");
    out = Replace(out.c_str(), "<", "&lt;");
    out = Replace(out.c_str(), ">", "&gt;");
    if (quote) {
        out = Replace(out.c_str(), "\"", "&quot;");
    }
    return base::Format::ToString("%s", out.c_str());
}

// For encoding query keys and values
const char *StringEncoder::QueryEncode(const char *s) {
    char *buf = new char[strlen(s) * 3 + 1];
    char *out = buf;
    const char *in = s;
    while (*in) {
        if (isalnum(*in) || *in == '-' || *in == '_' || *in == '.' ||
                *in == '~') {
            *out++ = *in++;
        } else if (*in == ' ') {
            *out++ = '+';
            in++;
        } else {
            char *hex = "0123456789abcdef";
            *out++ = '%';
            *out++ = hex[(*in >> 4) & 0xf];
            *out++ = hex[(*in) & 0xf];
            in++;
        }
    }
    *out = 0;
    const char *result = base::Format::ToString("%s", buf);
    delete buf;
    return result;
}

} // namespace base
