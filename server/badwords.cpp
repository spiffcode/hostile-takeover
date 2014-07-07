#include "server/badwords.h"
#include "mpshared/misc.h"

namespace boost {
template <class T>
T next(T x) { return ++x; }
template <class T>
T prev(T x) { return --x; }  
} // namespace boost

namespace wi {

// Number of wildcard chars in an entry, for example s**t has 2.
const int kcWildcardsEntryMax = 2;

BadWords::BadWords(const std::string& filename) : watcher_(filename),
        on_(true) {
    watcher_.SignalOnFileUpdated.connect(this, &BadWords::OnFileUpdated);
}

std::map<std::string, std::vector<std::string> > BadWords::ParsePairs(
        const std::string& s) {
    // Example: @pairs/f:ph/o:a,e,i,u/o:y 
    std::vector<std::string> parts = Split(s, '/');
    std::map<std::string, std::vector<std::string> > pairs;
    if (parts.size() == 0) {
        return pairs;
    }
    std::vector<std::string>::const_iterator it = parts.begin();
    for (; it != parts.end(); it++) {
        std::vector<std::string> key_values = Split(*it, ':');
        if (key_values.size() != 2) {
            continue;
        }
        // Get the values for this key. Include the key itself, since
        // the value list will be enumerated for word permutation.
        std::vector<std::string> values = Split(key_values[1], ',');
        values.push_back(key_values[0]);

        if (values.size() == 0) {
            continue;
        }
        std::map<std::string, std::vector<std::string> >::iterator it =
                pairs.find(key_values[0]);
        if (it == pairs.end()) {
            pairs.insert(std::map<std::string, std::vector<std::string> >
                    ::value_type(key_values[0], values));
        } else {
            it->second.insert(it->second.end(), values.begin(), values.end());
        }
    }
    return pairs;
}

void BadWords::OnFileUpdated(ThreadedFileWatcher *watcher) {
    RLOG() << watcher_.filename() << " has changed!";
    FILE *f = fopen(watcher_.filename().c_str(), "r");
    if (f == NULL) {
        return;
    }

    std::map<std::string, std::vector<std::string> > pairs;
    nodes_.clear();
    nodes_.push_back(MatchNode());
    bool standalone = false;
    bool wildcards = true;

    char sz[256];
    while (fgets(sz, sizeof(sz), f) != NULL) {
        int cch;
        const char *start = StripWhitespace(sz, &cch);
        if (cch == 0) {
            continue;
        }
        std::string s(start, cch);
        if (s[0] == '#') {
            continue;
        }
        if (s.find("@pairs") == 0) {
            pairs = ParsePairs(s);
            continue;
        }
        if (s.find("@standalone") == 0) {
            std::vector<std::string> parts = Split(s.c_str(), '/');
            if (parts.size() == 2) {
                if (parts[1].compare("true") == 0) {
                    standalone = true;
                }
                if (parts[1].compare("false") == 0) {
                    standalone = false;
                }
            }
            continue;
        }
        if (s.find("@wildcards") == 0) {
            std::vector<std::string> parts = Split(s.c_str(), '/');
            if (parts.size() == 2) {
                if (parts[1].compare("true") == 0) {
                    wildcards = true;
                }
                if (parts[1].compare("false") == 0) {
                    wildcards = false;
                }
            }
            continue;
        }
        AddPermutations(s, pairs, standalone, wildcards);
    }
    fclose(f);
}

void BadWords::AddPermutations(const std::string& word,
            const std::map<std::string, std::vector<std::string> >& pairs,
            bool standalone, bool wildcards) {
    // Split the word into pieces. Remember the order of the split values.
    std::vector<std::string> parts;
    std::vector<const std::vector<std::string> *> values;
    const std::vector<std::string> *splitter_values;
    const char *next = word.c_str();
    while (true) {
        const char *last = next;
        next = NULL;
        const char *splitter = NULL;
        std::map<std::string, std::vector<std::string> >::const_iterator it =
                pairs.begin();
        for (; it != pairs.end(); it++) {
            const char *pos = strstr(last, it->first.c_str());
            if (pos != NULL && (next == NULL || pos < next)) {
                next = pos;
                splitter = it->first.c_str();
                splitter_values = &it->second;
            }
        }
        if (next == NULL) {
            parts.push_back(last);
            break;
        }
        parts.push_back(std::string(last, next - last));
        next += strlen(splitter);
        values.push_back(splitter_values);
    }

    // Perform a recusion based permutation of each value at each split point.
    // Cycle through the split points right to left, so the left side
    // is the inner loop. That way, right sides can re-use portions of
    // the node graph that are the same.
    if (values.size() != 0) {
        Permute(parts[parts.size() - 1], parts, values, values.size() - 1,
                standalone, wildcards);
    } else {
        AddWord(parts[0], standalone, wildcards);
    }
}

void BadWords::Permute(const std::string& suffix,
        const std::vector<std::string>& parts,
        const std::vector<const std::vector<std::string> *>& values,
        int index, bool standalone, bool wildcards) {
    for (int i = 0; i < values[index]->size(); i++) {
        std::string word = parts[index] + (*values[index])[i] + suffix;
        if (index > 0) {
            Permute(word, parts, values, index - 1, standalone, wildcards);
        } else {
            AddWord(word, standalone, wildcards);
        }
    }
}

void BadWords::AddWord(const std::string& word, bool standalone,
        bool wildcards) {
    // Add this word to the graph, and expand the graph as necessary.
    int current_index = 0;
    for (int i = 0; i < word.size(); i++) {
        char ch = word[i];
        if (ch >= 'A' && ch <= 'Z') {
            ch += 'a' - 'A';
        }
        if (ch < 'a' || ch > 'z') {
            continue;
        }
        int char_index = ch - 'a';
        dword *next_index = &nodes_[current_index].indexes[char_index];
        if (i == word.size() - 1)  {
            dword ff = kfMatMatch;
            if (wildcards) {
                ff |= kfMatWildcards;
            }
            if (standalone) {
                ff |= kfMatStandalone;
            }
            *next_index |= ff;
            continue;
        }
        if ((*next_index & kfMatIndex) != 0) {
            current_index = (*next_index & kfMatIndex);
            continue;
        }
        *next_index |= (nodes_.size() & kfMatIndex);
        current_index = nodes_.size();
        nodes_.push_back(MatchNode());
    }
    LOG() << "added: " << word << " total nodes:" << nodes_.size();
}

std::vector<std::string> BadWords::Split(const std::string& s, char ch) {
    std::vector<std::string> parts;
    const char *next = s.c_str();
    while (true) {
        const char *last = next;
        next = strchr(next, ch);
        if (next == NULL) {
            parts.push_back(std::string(last));
            return parts;
        }
        parts.push_back(std::string(last, next - last));
        next += 1;
    }
}

bool BadWords::IsStandalone(const char *input, int start, int end) {
    if (start > 0) {
        char ch = input[start - 1];
        if (ch >= 'A' && ch <= 'Z') {
            ch += 'a' - 'A';
        }
        if (ch >= 'a' && ch <= 'z') {
            return false;
        }
    }
    if (input[end] != 0 && input[end + 1] != 0) {
        char ch = input[end + 1];
        if (ch >= 'A' && ch <= 'Z') {
            ch += 'a' - 'A';
        }
        if (ch >= 'a' && ch <= 'z') {
            return false;
        }
    }
    return true;
}

bool BadWords::IsMatch(const char *input, const MatchEntry& entry) {
    // Can't have more wildcard replacements than not
    if (entry.wildcard_replacements >= entry.char_count) {
        return false;
    }

    // Check for standalone if asked
    if (entry.standalone && !IsStandalone(input, entry.start, entry.end)) {
        return false;
    }

    // Don't allow spaces in matches that have wildcards of either type
    // (filler or replacement)
    if (entry.wildcard_count != 0 && entry.last_space >= entry.start &&
            entry.last_space <= entry.end) {
        return false;
    }
    return true;
}

const char *BadWords::Filter(const char *input, int *cch_back) {
    // Filter only if turned on (on is the default).
    if (cch_back != NULL) {
        *cch_back = -1;
    }
    if (!on_ || nodes_.size() == 0) {
        strncpyz(buffer_, input, sizeof(buffer_));
        return buffer_;
    }

    // Traverse the match graph, looking for matches. Handle non-alpha
    // wildcards and insertions, like f*k vs. fu*k.

    std::list<Match> matches;
    std::list<MatchEntry> progress;

    const char *pch = input;
    for (; *pch != 0; pch++) {
        // Ignore whitespace, but track it since it will cancel some matches
        char ch = *pch;

        // Treat newlines specially. This marks the boundary between
        // entered chat. This is tracked.
        if (ch == '\n') {
            std::list<MatchEntry>::iterator it = progress.begin();
            for (; it != progress.end(); it++) {
                it->has_newline = true;
                it->char_index_last = -1;
            }
            continue;
        }

        if (isspace(ch)) {
            std::list<MatchEntry>::iterator it = progress.begin();
            while (it != progress.end()) {
                it->last_space = pch - input;
                if (it->match) {
                    it->char_index_last = -1;
                }
                it++;
            }
            continue;
        }

        if (ch >= 'A' && ch <= 'Z') {
            ch += 'a' - 'A';
        }

        // If it is a letter, match against the match graph.
        if (ch >= 'a' && ch <= 'z') {
            int char_index = ch - 'a';

            // Examine in-progress matches.
            std::list<MatchEntry>::iterator it = progress.begin();
            while (it != progress.end()) {
                // Match a trailing repeated char.
                MatchEntry& entry = *it;
                if (entry.match) {
                    if (entry.char_index_last == char_index) {
                        entry.end = pch - input;
                        if (*(pch + 1) != 0) {
                            it++;
                            continue;
                        }
                    }
                    if (IsMatch(input, entry)) {
                        matches.push_back(Match(entry.start, entry.end));
                    }
                    it = progress.erase(it);
                    continue;
                }
                entry.char_count++;

                // Not matched yet. Is it a match now?
                dword *next_index =
                        &nodes_[entry.current_index].indexes[char_index];
                if (*next_index & kfMatMatch) {
                    if ((!entry.has_newline && entry.wildcard_replacements == 0)
                            || (*next_index & kfMatWildcards) != 0) {
                        // Add a new entry that is the match, ahead of the
                        // iterator, so it gets processed even if this is the
                        // last char.
                        MatchEntry new_entry = entry;
                        new_entry.end = pch - input;
                        new_entry.char_index_last = char_index;
                        new_entry.standalone =
                                (*next_index & kfMatStandalone) != 0;
                        new_entry.standalone |= (new_entry.last_space != -1);
                        new_entry.standalone |=
                                ((new_entry.wildcard_count) != 0);
                        new_entry.match = true;
                        progress.insert(boost::next(it), new_entry);
                    }
                }

                // Follow this char_index to the next node, if there one
                if (*next_index & kfMatIndex) {
                    entry.end = pch - input;
                    entry.current_index = (*next_index & kfMatIndex);
                    entry.char_index_last = char_index;
                    it++;
                    continue;
                }
                // No next node; allow the entry to persist if it is letter
                // doubling.
                if (entry.char_index_last == char_index) {
                    entry.end = pch - input;
                    entry.char_count--;
                    it++;
                    continue;
                }

                // Remove the entry
                it = progress.erase(it);
            }
           
            // Start a new match, if there is one 
            dword node_index = nodes_[0].indexes[char_index] & kfMatIndex;
            if (node_index != 0) {
                MatchEntry entry(pch - input, pch - input, node_index,
                        char_index);
                entry.char_count = 1;
                progress.push_back(entry);
            }
            continue;
        }

        // Not a-z, handle skip and insertion. Insertion is easy, just
        // preserve existing MatchEntrys. Skips require wildcarding:
        // clone existing MatchEntrys, and step them for each next node.
        // Add new match entries for all first char matches.

        std::list<MatchEntry>::iterator it = progress.begin();
        int next_indexes[26];
        int index_count;

        // Start new wildcard match entries only if this is a whitespace
        // boundary. Otherwise it is easy to cause an explosion of match
        // entries by simply typing "***************".

        if (pch == input || isspace(*(pch - 1))) {
            index_count = GetNextCharIndexes(0, next_indexes);
            for (int i = 0; i < index_count; i++) {
                // Push to the front so these entries aren't enumerated again
                dword *next_index = &nodes_[0].indexes[next_indexes[i]];
                MatchEntry entry(pch - input, pch - input,
                        (*next_index & kfMatIndex), -1);
                entry.wildcard_count = 1;
                entry.wildcard_replacements = 1;
                progress.push_front(entry);
            }
        }

        // Handle wildcarding in existing entries
        while (it != progress.end()) {
            // Don't span existing matched entries over wildcard chars.
            MatchEntry& entry = *it;
            if (entry.match) {
                if (IsMatch(input, entry)) {
                    matches.push_back(Match(entry.start, entry.end));
                }
                it = progress.erase(it);
                continue;
            }
            entry.wildcard_count++;

            // Enumerate next nodes, and make new stepped MatchEntrys for each.
            index_count = GetNextCharIndexes(entry.current_index, next_indexes);
            for (int i = 0; i < index_count; i++) {
                dword *next_index =
                        &nodes_[entry.current_index].indexes[next_indexes[i]];
                // Only match if there is a trailing whitespace boundary
                if (*(pch + 1) == 0 || isspace(*(pch + 1))) {
                    if ((*next_index & (kfMatMatch | kfMatWildcards)) ==
                            (kfMatMatch | kfMatWildcards)) {
                        // And there is more string to be evaled
                        if (*(pch + 1) != 0) {
                            // Add a floating MatchEntry, in order to suck up
                            // repeated chars of the match.
                            MatchEntry new_entry = entry;
                            new_entry.end = pch - input;

                            // Standalone, because since there is a wildcard,
                            // the entry is by definition standalone only.
                            new_entry.standalone = true;
                            new_entry.match = true;
                            new_entry.wildcard_replacements++;
                            progress.insert(it, new_entry);
                        } else {
                            // There is a wildcard, so by definition, it is
                            // standalone only.
                            MatchEntry new_entry = entry;
                            new_entry.standalone = true;
                            new_entry.end = pch - input;
                            new_entry.wildcard_replacements++;
                            if (IsMatch(input, new_entry)) {
                                matches.push_back(Match(new_entry.start,
                                        new_entry.end));
                            }
                        }
                    }
                }

                // If there is a child at this char (already know there is)
                if (*next_index & kfMatIndex) {
                    // And there has been kcWildcardsEntryMax wildcards only
                    if (entry.wildcard_count <= kcWildcardsEntryMax) {
                        // Then add a new match entry to track this path.
                        MatchEntry new_entry = entry;
                        new_entry.current_index = (*next_index & kfMatIndex);
                        new_entry.end = pch - input;
                        new_entry.wildcard_replacements++;
                        progress.insert(it, new_entry);
                    }
                }
            }
            it++;
        }
    }

    if (matches.size() > 1) {
        // See if the matches overlap; if so clean them up.
        bool overlap = false;
        std::list<Match>::iterator it = matches.begin();
        for (; it != matches.end(); it++) {
            std::list<Match>::iterator it_next = boost::next(it);
            if (it_next != matches.end()) {
                if ((*it).end >= (*it_next).start) {
                    overlap = true;
                    break;
                }
            }
        }
        if (overlap) {
            CleanupMatches(matches);
        }
    }

    // Find the earliest starting entry that spans the end.
    // If this overlaps a match, start from the match.
    if (cch_back != NULL) { 
        int earliest = -1;
        std::list<MatchEntry>::iterator it = progress.begin();
        for (; it != progress.end(); it++) {
            if (earliest == -1 || (*it).start < earliest) {
                earliest = (*it).start;
            }
        }
        if (earliest != -1) {
            std::list<Match>::iterator itm = matches.begin();
            for (; itm != matches.end(); itm++) {
                if (itm->end >= earliest) {
                    earliest = itm->start;
                }
            }
            *cch_back = (pch - input) - earliest;
        }
    }

    // Any matches? If not, return the original line
    if (matches.size() == 0) {
        strncpyz(buffer_, input, sizeof(buffer_));
        return buffer_;
    }

    // Finally, build the new string.
    return BuildResult(input, matches);
}

int BadWords::GetNextCharIndexes(int current_index, int *next_indexes) {
    int count = 0;
    for (int i = 0; i < 26; i++) {
        if (nodes_[current_index].indexes[i] != 0) {
            *next_indexes++ = i;
            count++;
        }
    }
    return count;
}

const char *BadWords::BuildResult(const char *input,
        std::list<Match>& matches) {
    std::ostringstream s;
    const char *in = input;
    int cch = strlen(input);
    std::list<Match>::iterator it = matches.begin();
    for (; it != matches.end(); it++) {
        const Match& match = *it;
        s << std::string(in, match.start - (in - input));
        s << std::string(match.end - match.start + 1, '*');
        in = &input[match.end + 1];
    }
    s << in;
    strncpyz(buffer_, s.str().c_str(), sizeof(buffer_));
    return buffer_;
}

bool CompareMatches(const Match& match_a, const Match& match_b) {
    // Sort so that earlier starting, and longer matches are first
    if (match_a.start < match_b.start) {
        return true;
    }
    if (match_a.start > match_b.start) {
        return false;
    }
    // But between matches that start at the same index, sort matches
    // that are longer first.
    if (match_a.end > match_b.end) {
        return true;
    }
    return false;
}

void BadWords::CleanupMatches(std::list<Match>& matches) {
    // Clean up the match list. First, remove smaller matches that are
    // inside larger matches.
  
    // Sort so that earlier and longer matches are first 
    matches.sort(CompareMatches);

    // Remove matches that overlap with other matches. This is easy since
    // the matches have been sorted. 
    std::list<Match>::iterator it_i = matches.begin();
    while (it_i != matches.end()) {
        std::list<Match>::iterator it_j = boost::next(it_i);
        while (it_j != matches.end()) {
            if ((*it_i).end >= (*it_j).start) {
                it_j = matches.erase(it_j);
                continue;
            }
            it_j++;
        }
        it_i++;
    }
}

} // namespace wi
