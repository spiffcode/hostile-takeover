#include "base/deletetracker.h"

namespace base {

// DeleteRecord methods

DeleteRecord::DeleteRecord(DeleteTracker *tracker) {
    tracker_ = tracker;
    pdrNext_ = NULL;
    deleted_ = false;
    tracker_->Register(this);
}

DeleteRecord::~DeleteRecord() {
    if (!deleted_) {
        tracker_->Unregister(this);
    }
}

// DeleteTracker methods

DeleteTracker::DeleteTracker() {
    pdrFirst_ = NULL;
}

DeleteTracker::~DeleteTracker() {
    for (DeleteRecord *pdr = pdrFirst_; pdr != NULL; pdr = pdr->pdrNext_) {
        pdr->deleted_ = true;
    }
}

void DeleteTracker::Register(DeleteRecord *pdr) {
    pdr->pdrNext_ = pdrFirst_;
    pdrFirst_ = pdr;
}

void DeleteTracker::Unregister(DeleteRecord *pdr) {
#ifdef LOGGING
    if (pdr != pdrFirst_) {
        LOG() << "Error!";
    }
#endif
    if (pdr == pdrFirst_) {
        pdrFirst_ = pdr->pdrNext_;
    }
}

} // namespace base
