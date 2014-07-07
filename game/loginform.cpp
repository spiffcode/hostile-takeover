#include "game/loginform.h"
#include "game/serviceurls.h"

namespace wi {

LoginForm::LoginForm(LoginHandler& handler) : handler_(handler),
        ask_(knLoginAskNone) {
}

LoginForm::~LoginForm() {
}

bool LoginForm::DoForm(LoginHandler& handler, bool fAutoLogin) {
    LoginForm *pfrm = (LoginForm *)gpmfrmm->LoadForm(gpiniForms, kidfLogin,
            new LoginForm(handler));
    if (pfrm == NULL) {
        return false;
    }

    int result = 0;
    pfrm->DoModal(&result, fAutoLogin);
    delete pfrm;
    return result == kidcLogin;
}

bool LoginForm::DoModal(int *pnResult, bool fAutoLogin) {
    // Reflect the state of LoginHandler in the UI
    ReflectHandlerState();

    // If enough credentials to attempt login, then do it and return
    // if successful. Always show if anonymous login
    if (fAutoLogin) {
        if (!handler_.anonymous() && handler_.ShouldAttemptLogin()) {
            if (AttemptLogin()) {
                *pnResult = kidcLogin;
                return true;
            }
        }
    }

    // Show the form
    return ShellForm::DoModal(pnResult);
}

void LoginForm::ReflectHandlerState() {
    // Initialize controls
    EditControl *pedc = (EditControl *)GetControlPtr(kidcPlayerName);
    char name[kcbPlayerName*2];
    handler_.GetPlayerName(name, sizeof(name));
    pedc->SetText(name);
    pedc = (EditControl *)GetControlPtr(kidcPassword);
    char password[kcbPassword];
    strncpyz(password, handler_.password(), sizeof(password));
    for (int i = 0; i < sizeof(password); i++) {
        if (password[i] == 0) {
            break;
        }
        password[i] = '*';
    }
    pedc->SetText(password);
    CheckBoxControl *pcbc = (CheckBoxControl *)GetControlPtr(kidcAnonymous);
    pcbc->SetChecked(handler_.anonymous());

    // Hide the login button until it is deemed ok to attempt
    // login
    ButtonControl *pbtn = (ButtonControl *)GetControlPtr(kidcLogin);
    pbtn->Show(handler_.ShouldAttemptLogin());

    // Hide input panel and password controls if login anonymously selected
    bool fShow = !handler_.anonymous();
    Control *pctl = GetControlPtr(kidcPlayerNamePanel);
    pctl->Show(fShow);
    pctl = GetControlPtr(kidcPasswordLabel);
    pctl->Show(fShow);
    pctl = GetControlPtr(kidcPassword);
    pctl->Show(fShow);
    pctl = GetControlPtr(kidcPasswordPanel);
    pctl->Show(fShow);
    pctl = GetControlPtr(kidcForums);
    pctl->Show(fShow);

    // Arrange buttons
    word aid0[] = { kidcLogin, kidcRegister, kidcUpdateAccount,
            kidcCancel, 0 };
    word aid1[] = { kidcLogin, kidcCancel, 0 };

    word *aidShow;
    word *aidHide;
    if (handler_.anonymous()) {
        aidShow = aid1;
        aidHide = aid0;
    } else {
        aidShow = aid0;
        aidHide = aid1;
    }

    // Hide first
    for (int i = 0; aidHide[i] != 0; i++) {
        pctl = GetControlPtr(aidHide[i]);
        pctl->Show(false);
    }

    // Calc width of visible buttons
    int cx = 0;
    for (int i = 0; aidShow[i] != 0; i++) {
        pctl = GetControlPtr(aidShow[i]);
        Rect rc;
        pctl->GetRect(&rc);
        cx += rc.Width();
        if (aidShow[i + 1] != 0) {
            cx += 10;
        }
    }

    // Lay them out
    int x = (m_rc.Width() - cx) / 2;
    for (int i = 0; aidShow[i] != 0; i++) {
        pctl = GetControlPtr(aidShow[i]);
        Rect rc;
        pctl->GetRect(&rc);
        pctl->SetPosition(x, rc.top);
        x += rc.Width() + 10;
        pctl->Show(true);
    }
}

void LoginForm::OnControlSelected(word idc) {
    switch (idc) {
    case kidcAnonymous:
        {
            CheckBoxControl *pcbc = (CheckBoxControl *)
                    GetControlPtr(kidcAnonymous);
            handler_.SetAnonymous(pcbc->IsChecked());
            ReflectHandlerState();
        }
        break;

    case kidcLogin:
        AttemptLogin();
        if (handler_.loggedin()) {
            EndForm(kidcLogin);
        }
        break;

    case kidcPlayerNamePanel:
    case kidcPlayerName:
        // This brings up native UI asking for the player's name.
        // This UI is modeless and executes on the main thread. This
        // form will get notified when this native UI goes away, at
        // which point it can get the string.
        if (!handler_.anonymous()) {
            ask_ = knLoginAskPlayerName;
            HostInitiateAsk("Enter Player Name", kcbPlayerName - 1,
                    handler_.username());
        }
        break;

    case kidcPasswordPanel:
    case kidcPassword:
        if (!handler_.anonymous()) {
            ask_ = knLoginAskPassword;
            HostInitiateAsk("Enter Password", kcbPassword - 1,
                    handler_.password(), knKeyboardAskDefault,
                    true);
        }
        break;

    case kidcRegister:
        HostInitiateWebView("", kszRegisterUrl);
        //HostOpenUrl(kszRegisterUrl);
        break;

    case kidcUpdateAccount:
        HostInitiateWebView("", kszUpdateAccountUrl);
        //HostOpenUrl(kszUpdateAccountUrl);
        break;

    case kidcCancel:
        EndForm(kidcCancel);
        break;
    }
}

bool LoginForm::AttemptLogin() {
    // Want the form to be visible when doing this since Login is
    // synchronous.
    Show(true);

    // Attempt to login. Note WaitingUI forces a synchronous repaint
    dword result;
    {
        TransportWaitingUI twui("Logging in...");
        result = handler_.Login();
    }

    // Show messages here as necessary
    switch (result) {
    case knLoginResultSuccess:
    case knLoginResultAnonymousSuccess:
        return true;

    case knLoginResultNoPassword:
        HtMessageBox(kfMbWhiteBorder, "Login Failure",
                "The password field is empty. Please enter your password and try again.");
        return false;

    case knLoginResultAuthDown:
        HtMessageBox(kfMbWhiteBorder, "Login Failure",
                "The player authentication server is not responding. Please try to login as an anonymous player.");
        return false;

    case knLoginResultFail:
    default:
        if (!handler_.anonymous()) {
            HtMessageBox(kfMbWhiteBorder, "Login Failure",
                    "Your login attempt failed. Please check your username and password, or log in anonymously.");
        } else {
            HtMessageBox(kfMbWhiteBorder, "Login Failure",
                    "An unknown error occured while logging in. Please try again.");
        }
        return false;
    }
}

void LoginForm::OnStatusUpdate(char *pszStatus) {
}

void LoginForm::OnConnectionClose() {
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = connectionCloseEvent;
    gevm.PostEvent(&evt);
}

void LoginForm::OnShowMessage(const char *message) {
    message_ = message;
    Event evt;
    memset(&evt, 0, sizeof(evt));
    evt.idf = m_idf;
    evt.eType = showMessageEvent;
    gevm.PostEvent(&evt);
}

bool LoginForm::OnFilterEvent(Event *pevt) {
    if (pevt->eType == askStringEvent) {
        char s[512];
        HostGetAskString(s, sizeof(s));

        switch (ask_) {
        case knLoginAskPlayerName:
            handler_.SetUsername(s);
            ReflectHandlerState();
            break;
            
        case knLoginAskPassword:
            handler_.SetPassword(s);
            ReflectHandlerState();
            break;
        }
        return true;
    }

    if (pevt->eType == connectionCloseEvent) {
        HtMessageBox(kfMbWhiteBorder, "Comm Problem", "The server has closed your connection.");
        EndForm(kidcCancel);
        return true;
    }

    if (pevt->eType == showMessageEvent) {
        HtMessageBox(kfMbWhiteBorder, "Server Message", message_.c_str());
        message_ = "";
        return true;
    }

    return false;
}


} // namespace wi
