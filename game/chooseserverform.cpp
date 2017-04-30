#include "game/chooseserverform.h"
#include "game/simplerequest.h"
#include "game/serviceurls.h"
#include "game/loginhandler.h"
#include "mpshared/constants.h"
#include "base/format.h"
#include "base/thread.h"
#include "base/misc.h"
#include "yajl/wrapper/jsonbuilder.h"
#include "game/xtransport.h"
#include <algorithm>
#include <map>

namespace wi {

bool ServerInfoSort(const ServerInfo& info1, const ServerInfo& info2) {
    return info1.sort_key < info2.sort_key;
}

dword ChooseServerForm::DoForm(Transport **pptra, std::string *server_name) {
    ChooseServerForm *pfrm = (ChooseServerForm *)gpmfrmm->LoadForm(gpiniForms,
            kidfChooseServer, new ChooseServerForm());
    if (pfrm == NULL) {
        return knChooseServerResultCancel;
    }

    int result = 0;
    pfrm->DoModal(&result);
    Transport *ptra = pfrm->transport();
    std::string name = pfrm->server_name();
    delete pfrm;
   
    if (gevm.IsAppStopping()) {
        return knChooseServerResultAppStop;
    }
 
    if (result == kidcCancel || ptra == NULL) {
        return knChooseServerResultCancel;
    }

    *pptra = ptra;
    *server_name = name;
    return knChooseServerResultConnect;
}

bool ChooseServerForm::DoModal(int *pnResult) {
    // Hide Connect button until there is something to connect to
    LabelControl *plbl = (LabelControl *)GetControlPtr(kidcOk);
    plbl->Show(false);

    // Position the columns
    PositionColumns();
    Refresh(false);

    // If there is a single entry this client can connect to, use it without
    // presenting UI
    if (infos_.size() == 1) {
        Control *pctl = GetControlPtr(kidcOk);
        if (pctl->GetFlags() & kfCtlVisible) {
            const ServerInfo& info = infos_[0];
            if (Connect(info) == knTransportOpenResultSuccess) {
                *pnResult = kidcOk;
                return true;
            }
        }
    }

    return ShellForm::DoModal(pnResult); 
}

void ChooseServerForm::OnZipDone() {
    if (errors_) {
        HtMessageBox(kfMbWhiteBorder, "Service Message", errorstr_.c_str());
    }
}

void ChooseServerForm::PositionColumns() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcServerList);
    Rect rcList;
    plstc->GetRect(&rcList);
    Font *pfnt = plstc->GetFont();

    LabelControl *plblPlayers = (LabelControl *)GetControlPtr(kidcNumPlayers);
    int cxNumPlayers = pfnt->GetTextExtent(plblPlayers->GetText());
    int xNumPlayers = rcList.right - cxNumPlayers;
    LabelControl *plblStatus = (LabelControl *)GetControlPtr(kidcServerStatus);
    int cxStatus = pfnt->GetTextExtent(plblStatus->GetText());
    int xStatus = xNumPlayers - 40 - cxStatus;
    int xLocation = rcList.left + (xStatus - rcList.left) / 2;
    int xName = rcList.left;
    
    plstc->SetTabStops(xName - rcList.left, xLocation - rcList.left,
            xStatus + cxStatus / 2 - rcList.left,
            xNumPlayers + cxNumPlayers / 2 - rcList.left);
    plstc->SetTabFlags(0, 0, kfLstTabCenterOn, kfLstTabCenterOn);

    word ids[] = { kidcServerName, kidcServerLocation, kidcServerStatus,
            kidcNumPlayers };
    int ax[] = { xName, xLocation, xStatus, xNumPlayers };

    for (int i = 0; i < ARRAYSIZE(ids); i++) {
        LabelControl *plbl = (LabelControl *)GetControlPtr(ids[i]);
        Rect rcCtl;
        plbl->GetRect(&rcCtl);
        plbl->SetPosition(ax[i], rcCtl.top);
    }
}

void ChooseServerForm::Refresh(bool show_errors) {
    bool success = RequestInfos();

    ListControl *plstc = (ListControl *)GetControlPtr(kidcServerList);
    plstc->Clear();

    if (infos_.size() == 0) {
        if (success) {
            errorstr_ = "The multiplayer service is down for maintenance. Try again later.";
        }
        if (show_errors) {
            HtMessageBox(kfMbWhiteBorder, "Service Message",
                errorstr_.c_str());
        } else {
            errors_ = true;
        }
    } else {
        for (int i = 0; i < infos_.size(); i++) {
            const ServerInfo& info = infos_[i];
            const char *s = base::Format::ToString("%s\t%s\t%s\t%d",
                info.name.c_str(), info.location.c_str(), info.status.c_str(),
                info.player_count);
            plstc->Add(s);
        }
        plstc->Select(0, true);
    }
    ShowHide();
}

void ChooseServerForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcOk:
        {
            ListControl *plstc = (ListControl *)GetControlPtr(kidcServerList);
            int index = plstc->GetSelectedItemIndex();
            if (index >= 0 && index < infos_.size()) {
                dword result = Connect(infos_[index]);
                if (result == knTransportOpenResultSuccess) {
                    EndForm(kidcOk);
                    break;
                }
                ShowTransportError(result);
            }
        }
        break;

    case kidcRefresh:
        Refresh();
        break;

    case kidcCancel:
        EndForm(kidcCancel);
        break;
    }
}

dword ChooseServerForm::Connect(const ServerInfo& info) {
    // Note for the moment, this only supports XTransport. This is done
    // this way for the best user experience: so that a Connect can be
    // attempted while this form is up, and if it fails the form stays up
    // so the list can be refreshed, or another server tried. Bringing up
    // this form is expensive because of the ServerInfo query.

    TransportWaitingUI twui("Contacting Service");
    Transport *ptra = (Transport *)new XTransport(info.address);
    dword result = ptra->Open();
    if (result != knTransportOpenResultSuccess) {
        delete ptra;
        return result;
    }
    connected_ = info;
    transport_ = ptra;
    return result;
}

void ChooseServerForm::ShowTransportError(dword result) {
    const char *message = NULL;
    switch(result) {
    case knTransportOpenResultFail: 
        message = "Failure accessing network.";
        break;

    case knTransportOpenResultNoNetwork:
        message = "Please check for network connectivity.";
        break;

    case knTransportOpenResultCantConnect: 
        message = "Could not connect to game server.";
        break;

    case knTransportOpenResultNotResponding:
        message = "Timed out waiting for Game server to respond.";
        break;

    case knTransportOpenResultProtocolMismatch: 
        message = "This server requires that you upgrade to the latest Hostile Takeover before continuing.";
        break;

    case knTransportOpenResultServerFull:
        message = "This server is full. Please try another server.";
        break;
    }

    if (message != NULL) {
        HtMessageBox(kfMbWhiteBorder, "Service Message", message);
    }
}

void ChooseServerForm::ShowHide() {
    ListControl *plstc = (ListControl *)GetControlPtr(kidcServerList);
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcOk);
    int index = plstc->GetSelectedItemIndex();
    if (index < 0 || index >= infos_.size()) {
        pbtn->Show(false);
        return;
    }
    ServerInfo& info = infos_[index];
    if (strcmp(info.status.c_str(), "ok") != 0) {
        pbtn->Show(false);
        return;
    }
    pbtn->Show(true);
}

void ChooseServerForm::OnControlNotify(word idc, int nNotify) {
    if (idc == kidcServerList && nNotify == knNotifySelectionChange) {
        ShowHide();
    }
    ShellForm::OnControlNotify(idc, nNotify);
}

bool ChooseServerForm::RequestInfos() {
    // Default error string
    errorstr_ = "Invalid Response From Service";

    TransportWaitingUI twui("Contacting Service");
    std::string url = GetServiceUrl();
    SimpleRequest req;
    char errorstr[1024];
    if (!req.Get(url.c_str(), NULL, 0, errorstr, sizeof(errorstr))) {
        errorstr_ = errorstr;
        return false;
    }

    // Parse json
    json::JsonBuilder builder;
    builder.Start();
    if (!builder.Update((const char *)req.body()->Data(),
            req.body()->Length())) {
        return false;
    }
    json::JsonObject *obj = builder.End();
    if (obj == NULL) {
        return false;
    }

    // Pull out infos and weed out the ones not appropriate for this client
    json::JsonMap *map = (json::JsonMap *)obj;
    json::JsonArray *infos = (json::JsonArray *)map->GetObject("infos");
    if (infos == NULL) {
        delete obj;
        return false;
    }
    std::map<std::string, ServerInfo> infomap;
    for (int i = 0; i < infos->GetCount(); i++) {
        json::JsonMap *json_map = (json::JsonMap *)infos->GetObject(i);
        ServerInfo info;
        info.sort_key = json_map->GetInteger("sort_key");
        info.name = json_map->GetString("name");
        info.location = json_map->GetString("location");
        info.address = base::SocketAddress(json_map->GetString("address"));
        info.protocol = json_map->GetInteger("protocol");
        info.status = json_map->GetString("status");
        info.player_count = json_map->GetInteger("player_count");
        info.type = json_map->GetString("type");
        info.disallow = json_map->GetString("disallow");

        // If production client, only show production servers.
#if !defined(DEBUG) && !defined(BETA_TIMEOUT)
        if (strcmp(info.type.c_str(), "production") != 0) {
            continue;
        }
#endif

        // Don't show old protocol servers to newer clients
        if (info.protocol < kdwProtocolCurrent) {
            continue;
        }

        // Don't show servers to disallowed client platforms
#if defined(__MACOSX__)
        if (info.disallow.find("mac") != std::string::npos) {
            continue;
        }
#elif defined(DARWIN)
        if (info.disallow.find("darwin") != std::string::npos) {
            continue;
        }
#elif defined(__LINUX__)
        if (info.disallow.find("linux") != std::string::npos) {
            continue;
        }
#elif defined(__ANDROID__)
        if (info.disallow.find("android") != std::string::npos) {
            continue;
        }
#elif defined(IPHONE) || defined(__IPHONEOS__)
        if (info.disallow.find("iphone") != std::string::npos) {
            continue;
        }
#endif

        // If there is an info with this name that matches the protocol,
        // keep it and discard the new info
        std::map<std::string, ServerInfo>::iterator it =
                infomap.find(info.name);
        if (it != infomap.end()) {
            if (it->second.protocol == kdwProtocolCurrent) {
                continue;
            }

            // Otherwise delete this so the new info can be added. This
            // makes upgrading seamless.
            infomap.erase(it);
        }

        // Add new info into the map
        infomap.insert(std::map<std::string, ServerInfo>::value_type(
                info.name, info));
    }
    delete obj;

    // Now read out the infos from the map, and sort them by sort_key
    infos_.clear();
    std::map<std::string, ServerInfo>::iterator it = infomap.begin();
    while (it != infomap.end()) {
        infos_.push_back(it->second);
        it++;
    }
    std::stable_sort(infos_.begin(), infos_.end(), ServerInfoSort);

    return true;
}

std::string ChooseServerForm::GetServiceUrl() {
    // player, if exists
    // device id, hashed
    // protocol version

    std::string deviceid(base::StringEncoder::QueryEncode(gszDeviceId));
    std::string os(base::StringEncoder::QueryEncode(HostGetPlatformString()));

    const char *url;
    LoginHandler handler;
    if (handler.anonymous()) {
        url = base::Format::ToString("%s?x=%d&d=%s&o=%s", kszServerInfoUrl,
                kdwProtocolCurrent, deviceid.c_str(), os.c_str());
    } else {
        std::string player(base::StringEncoder::QueryEncode(
                handler.username()));
        url = base::Format::ToString("%s?x=%d&p=%s&d=%s&o=%s", kszServerInfoUrl,
                kdwProtocolCurrent, player.c_str(), deviceid.c_str(), os.c_str());
    }

    return url;
}

}
