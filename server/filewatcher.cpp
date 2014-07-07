#include "base/thread.h"
#include "server/filewatcher.h"
#include <sys/stat.h>

namespace wi {

ThreadedFileWatcher::ThreadedFileWatcher(const std::string& filename,
        int interval_seconds) : filename_(filename),
        interval_seconds_(interval_seconds) {
    // Set mtime_ to 0 so the signal gets raised right away.
    mtime_ = 0;
    watcher_thread_.Start(this, &ThreadedFileWatcher::ThreadStart);
}

void ThreadedFileWatcher::ThreadStart(void *pv) {
    // Running on watcher_thread_, notifying thread_
    while (!watcher_thread_.IsStopping()) {
        time_t mtime = GetFileTime();
        if (mtime != mtime_) {
            mtime_ = mtime;
            thread_.Post(1, this);
        }
        watcher_thread_.RunLoop(interval_seconds_ * 100);
    }
}

time_t ThreadedFileWatcher::GetFileTime() {
    struct stat s;
    memset(&s, 0, sizeof(s));
    if (stat(filename_.c_str(), &s) == 0) {
        return s.st_mtime;
    }
    return 0;
}

void ThreadedFileWatcher::OnMessage(base::Message *pmsg) {
    // Running on thread_
    SignalOnFileUpdated(this);
}

} // namespace wi
