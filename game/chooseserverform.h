#ifndef __CHOOSESERVERFORM_H__
#define __CHOOSESERVERFORM_H__

#include "game/ht.h"
#include "base/socketaddress.h"
#include "yajl/wrapper/jsontypes.h"
#include <string>
#include <vector>

namespace wi {

struct ServerInfo {
    int sort_key;
    std::string name;
    std::string location;
    base::SocketAddress address;
    int protocol;
    std::string status;
    int player_count;
    std::string type;
    std::string disallow;
};

const dword knChooseServerResultConnect = 0;
const dword knChooseServerResultCancel = 1;
const dword knChooseServerResultAppStop = 2;

class ChooseServerForm : public ShellForm {
public:
    ChooseServerForm() : transport_(NULL), errors_(false) {}

    Transport *transport() { return transport_; }
    const std::string &server_name() { return connected_.name; }

    static dword DoForm(Transport **pptra, std::string *server_name);

	// Form overrides
    virtual bool DoModal(int *pnResult);
    virtual void OnControlSelected(word idc);
    virtual void OnControlNotify(word idc, int nNotify);

    // ShellForm overrides
    virtual void OnZipDone();

private:
    void ShowHide();
    bool RequestInfos();
    void PositionColumns();
    void Refresh(bool show_errors = true);
    std::string GetServiceUrl();
    dword Connect(const ServerInfo& info);
    void ShowTransportError(dword error);

    std::vector<ServerInfo> infos_;
    ServerInfo connected_;
    std::string errorstr_;
    Transport *transport_;
    bool errors_;
};

} // namespace wi

#endif // __CHOOSESERVERFORM_H__



