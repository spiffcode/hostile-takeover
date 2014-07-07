#ifndef __BADWORDS_H__
#define __BADWORDS_H__

#include "inc/basictypes.h"
#include "base/sigslot.h"
#include "server/filewatcher.h"
#include <vector>
#include <string>
#include <map>

namespace wi {

const dword kfMatMatch = 0x80000000;
const dword kfMatStandalone = 0x40000000;
const dword kfMatWildcards = 0x20000000;
const dword kfMatIndex = 0x1fffffff;

struct Match {
    Match(int start, int end) : start(start), end(end) {}
    int start;
    int end;
};

struct MatchNode {
    MatchNode() { memset(this, 0, sizeof(*this)); }
    dword indexes[26];
};

struct MatchEntry {
    MatchEntry(int start, int end, int current_index, int char_index_last) :
            start(start), end(end), current_index(current_index),
            char_index_last(char_index_last), char_count(0),
            wildcard_count(0), wildcard_replacements(0), match(false),
            standalone(false), last_space(-1), has_newline(false) {}
    int start;
    int end;
    int current_index;
    int char_index_last;
    int char_count;
    int wildcard_count;
    int wildcard_replacements;
    bool match;
    bool standalone;
    int last_space;
    bool has_newline;
};

// Single pass bad word filter

class BadWords : public base::has_slots<> {
public:
    BadWords(const std::string& filename);
    const char *Filter(const char *input, int *cch_back = NULL);
    bool on() { return on_; }
    void toggle() { on_ ^= true; }

private:
    std::map<std::string, std::vector<std::string> > ParsePairs(
            const std::string& s);
    void AddPermutations(const std::string& word,
            const std::map<std::string, std::vector<std::string> >& pairs,
            bool standalone, bool wildcards);
    void Permute(const std::string& suffix,
            const std::vector<std::string>& parts,
            const std::vector<const std::vector<std::string> *>& values,
            int index, bool standalone, bool wildcards);
    void AddWord(const std::string& word, bool standalone, bool wildcards);
    std::vector<std::string> Split(const std::string& s, char ch);
    bool IsStandalone(const char *input, int start, int end);
    const char *BuildResult(const char *input, std::list<Match>& matches);
    void CleanupMatches(std::list<Match>& matches);
    int GetNextCharIndexes(int current_index, int *next_indexes);
    void OnFileUpdated(ThreadedFileWatcher *watcher);
    bool IsMatch(const char *input, const MatchEntry& entry);

    ThreadedFileWatcher watcher_;
    std::vector<MatchNode> nodes_;
    char buffer_[2048];
    bool on_;
};

} // namespace wi

#endif // __BADWORDS_H__
