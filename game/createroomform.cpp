#include "game/createroomform.h"

namespace wi {

CreateRoomForm::CreateRoomForm() : ask_(knCreateRoomAskNone) {
}

CreateRoomForm::~CreateRoomForm() {
}

bool CreateRoomForm::DoForm(char *roomname, int cbRoomname, char *password, int cbPassword) {
    CreateRoomForm *pfrm = (CreateRoomForm *)gpmfrmm->LoadForm(gpiniForms,
            kidfCreateRoom, new CreateRoomForm());
    if (pfrm == NULL) {
        return false;
    }
    if (!pfrm->DoModal()) {
        delete pfrm;
        return false;
    }
    pfrm->GetRoomname(roomname, cbRoomname);
    pfrm->GetPassword(password, cbPassword);
    delete pfrm;
    return true;
}

bool CreateRoomForm::DoModal(int *pnResult, Sfx sfxShow, Sfx sfxHide) {
    HideShowPasswordFields();
    return ShellForm::DoModal(pnResult, sfxShow, sfxHide);
}

void CreateRoomForm::HideShowPasswordFields() {
    CheckBoxControl *pcbc = (CheckBoxControl *)GetControlPtr(kidcPrivate);
    Control *pctl = GetControlPtr(kidcPasswordLabel);
    pctl->Show(pcbc->IsChecked());
    pctl = GetControlPtr(kidcPassword);
    pctl->Show(pcbc->IsChecked());
    pctl = GetControlPtr(kidcPasswordPanel);
    pctl->Show(pcbc->IsChecked());
}

void CreateRoomForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcPrivate:
        HideShowPasswordFields();
        break;

    case kidcRoomNamePanel:
    case kidcRoomName:
        {
            ask_ = knCreateRoomAskRoomname;
            char roomname[kcbRoomname];
            GetRoomname(roomname, sizeof(roomname));
            HostInitiateAsk("Enter Room Name", kcbRoomname - 1, roomname);
        }
        break;

    case kidcPasswordPanel:
    case kidcPassword:
        {
            ask_ = knCreateRoomAskPassword;
            char password[kcbPassword];
            GetPassword(password, sizeof(password));
            HostInitiateAsk("Enter Password", kcbPassword - 1, password,
                    knKeyboardAskDefault, true);
        }
        break;

    default:
        ShellForm::OnControlSelected(idc);
        break;
    }
}

bool CreateRoomForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType != askStringEvent) {
        return false;
    }

    char s[512];
    HostGetAskString(s, sizeof(s));

    switch (ask_) {
    case knCreateRoomAskRoomname:
        {
            roomname_ = s;
            EditControl *pedc = (EditControl *)GetControlPtr(kidcRoomName);
            pedc->SetText(s);
        }
        break;
        
    case knCreateRoomAskPassword:
        {
            password_ = s;
            for (int i = 0; i < sizeof(s); i++) {
                if (s[i] == 0) {
                    break;
                }
                s[i] = '*';
            }
            EditControl *pedc = (EditControl *)GetControlPtr(kidcPassword);
            pedc->SetText(s);
        }
        break;
    }
    return true;
}

void CreateRoomForm::GetRoomname(char *roomname, int cb) {
    strncpyz(roomname, roomname_.c_str(), cb);
}

void CreateRoomForm::GetPassword(char *password, int cb) {
    strncpyz(password, password_.c_str(), cb);
}

} // namespace wi
