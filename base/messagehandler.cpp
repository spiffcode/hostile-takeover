#include "base/log.h"
#include "base/messagehandler.h"
#include "base/thread.h"

namespace base {

MessageHandler::MessageHandler(Thread& thread) : thread_(thread),
        disposed_(false) {
}

MessageHandler::MessageHandler() : thread_(Thread::current()),
        disposed_(false) {
}

MessageHandler::~MessageHandler() {
    disposed_ = true;
    thread_.Clear(this);
    thread_.ClearDispose(this);
}

void MessageHandler::Dispose() {
    if (!disposed_) {
        disposed_ = true;
        thread_.Dispose(this);
        LOG() << base::Log::Format("0x%08lx ", this) << "disposing";
    } else {
        LOG() << base::Log::Format("0x%08lx ", this)
                << "tried to dispose but already disposed";
    }
}

} // namespace base
