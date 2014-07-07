#ifndef __TRACKER_H__
#define __TRACKER_H__

#include "base/messagehandler.h"
#include "base/socketaddress.h"
#include <map>

namespace wi {

class Tracker : base::MessageHandler {
public:
    Tracker();
    void Add(const char *s, long64 tExpires);
    void Remove(const char *s);
    bool Find(const char *s, long64 *tExpires = NULL);
    void Clear();

private:
    virtual void OnMessage(base::Message *pmsg);

    typedef std::map<std::string, long64> TrackMap;
    TrackMap map_;
};

} // namespace wi

#endif // __TRACKER_H__
