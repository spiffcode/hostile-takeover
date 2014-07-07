#ifndef __LOBBY__
#define __LOBBY__

#include "game/ht.h"
#include "game/loginhandler.h"
#include "game/gameform.h"
#include <string>

namespace wi {

const dword knConnectResultSuccess = 0;
const dword knConnectResultError = 1;
const dword knConnectResultAppStop = 2;

STARTLABEL(ConnectResults)
    LABEL(knConnectResultSuccess)
    LABEL(knConnectResultError)
    LABEL(knConnectResultAppStop)
ENDLABEL(ConnectResults)

const dword knShellResultSuccess = 0;
const dword knShellResultError = 1;
const dword knShellResultAppStop = 2;

STARTLABEL(ShellResults)
    LABEL(knShellResultSuccess)
    LABEL(knShellResultError)
    LABEL(knShellResultAppStop)
ENDLABEL(ShellResults)

const dword knBeginResultSuccess = 0;
const dword knBeginResultError = 1;
const dword knBeginResultAppStop = 2;
const dword knBeginResultTransportClosed = 3;

STARTLABEL(BeginResults)
    LABEL(knBeginResultSuccess)
    LABEL(knBeginResultError)
    LABEL(knBeginResultAppStop)
    LABEL(knBeginResultTransportClosed)
ENDLABEL(BeginResults)

class Lobby {
public:
    Lobby();
    ~Lobby();

    dword Shell(const PackId *ppackidFind);
    
private:
    dword Connect(std::string *server_name);
    void Disconnect();
    dword ShellHandler(const PackId *ppackidFind,
            const std::string& server_name);
    dword BeginGame(const GameInfo& info, bool creator, Chatter& chatter);
};

} // namespace wi

#endif // __LOBBY__
