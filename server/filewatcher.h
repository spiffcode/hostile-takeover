#ifndef __FILEWATCHER_H__
#define __FILEWATCHER_H__

#include <string>
#include "base/messagequeue.h"
#include "base/thread.h"
#include "base/sigslot.h"

namespace wi {

// Signals the creator thread of a file modified time change. Watches on a
// worker thread.

class ThreadedFileWatcher : base::MessageHandler {
public:
    ThreadedFileWatcher(const std::string& filename,
            int interval_seconds = 60 * 2);
    const std::string& filename() { return filename_; }
    base::signal1<ThreadedFileWatcher *> SignalOnFileUpdated;

private:
    time_t GetFileTime();
    void ThreadStart(void *pv);
    virtual void OnMessage(base::Message *pmsg);

    base::Thread watcher_thread_;
    std::string filename_;
    int interval_seconds_;
    time_t mtime_;
};

} // namespace wi

#endif // __FILEWATCHER_H__
