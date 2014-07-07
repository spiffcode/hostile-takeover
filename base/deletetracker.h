#ifndef __DELETETRACKER_H__
#define __DELETETRACKER_H__

#include "inc/basictypes.h"
#include "base/log.h"

namespace base {

class DeleteTracker;

class DeleteRecord {
public:
    DeleteRecord(DeleteTracker *tracker);
    ~DeleteRecord();
    bool deleted() { return deleted_; }

private:
    bool deleted_;
    DeleteRecord *pdrNext_;
    DeleteTracker *tracker_;

    friend class DeleteTracker;
};

class DeleteTracker {
public:
    DeleteTracker();
    virtual ~DeleteTracker();
    void Register(DeleteRecord *pdr);
    void Unregister(DeleteRecord *pdr);

private:
    DeleteRecord *pdrFirst_;
};

} // namespace base

#endif // __DELETETRACKER_H__
