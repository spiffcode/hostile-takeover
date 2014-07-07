#ifndef __BASE_MISC_H__
#define __BASE_MISC_H__

#include "inc/basictypes.h"
#include "base/log.h"
#include "base/format.h"

#if defined(LOGGING) && !defined(LABELS)
#define LABELS
#endif

namespace base {

struct Label {
    int index;
    const char *label;
};

class Labels {
public:
    Labels(const Label *alabel) {
        alabel_ = alabel;
    }
    const char *Find(int index) const {
        for (const Label *label = alabel_; label->label != NULL; label++) {
            if (label->index == index) {
                return label->label;
            }
        }
        return Format::ToString("UNKNOWN: %d", index);
    }
private:
    const Label *alabel_;
};

#ifdef LABELS
#define STARTLABEL(name) const base::Label name ## _labels[] = {
#define LABEL(x) { x, #x },
#define ENDLABEL(name) { 0, 0 } }; const base::Labels name(name ## _labels);
#else // !LABELS
#define STARTLABEL(name) const base::Label name ## _labels[] = {
#define LABEL(x)
#define ENDLABEL(name) { 0, 0 } }; const base::Labels name(name ## _labels);
#endif

class StringEncoder {
public:
    static const char *Escape(const char *s, bool quote = false);
    static const char *Replace(const char *s, const char *findstr,
            const char *replacewith);
    static const char *QueryEncode(const char *s);
};

} // namespace base

#endif // __BASE_MISC_H__
