#include "inc/basictypes.h"
#include "base/tick.h"
#include <sys/time.h>
#include <time.h>

namespace base {

void GetTimeOfDay(int *sec, int *usec) {
    struct timeval tv;
    gettimeofday(&tv, 0);

    static bool s_init;
    static dword s_sec;
    if (!s_init) {
        s_init = true;
        s_sec = tv.tv_sec - 1; // so it's never zero
    }

    *sec = tv.tv_sec - s_sec;
    *usec = tv.tv_usec;
}

long64 GetTickCount() {
    int sec, usec;
    GetTimeOfDay(&sec, &usec);
    return ((long64)sec) * 100 + usec / 10000;
}

long64 GetMillisecondCount() {
    int sec, usec;
    GetTimeOfDay(&sec, &usec);
    return ((long64)sec) * 1000 + usec / 1000;
}

dword GetSecondsUnixEpocUTC() {
    time_t t = time(NULL);
    return (dword)t;
}

} // namespace base
