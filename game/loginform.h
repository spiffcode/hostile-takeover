#ifndef __LOGINFORM_H__
#define __LOGINFORM_H__

#include "game/ht.h"
#include "game/loginhandler.h"
#include "mpshared/messages.h"
#include <string>

namespace wi {

const int knLoginAskNone = 0;
const int knLoginAskPlayerName = 1;
const int knLoginAskPassword = 2;

class LoginForm : public ShellForm, ITransportCallback {
public:
    LoginForm(LoginHandler& handler);
    ~LoginForm();

    static bool DoForm(LoginHandler& handler, bool fAutoLogin);
	bool DoModal(int *pnResult, bool fAutoLogin);

    // Form callbacks
    virtual void OnControlSelected(word idc);
    virtual bool OnFilterEvent(Event *pevt);

private:
    void ReflectHandlerState();
    bool AttemptLogin();

    // ITransportCallback
    virtual void OnStatusUpdate(char *pszStatus);
    virtual void OnConnectionClose();
    virtual void OnShowMessage(const char *message);

    LoginHandler& handler_;
    int ask_;
    std::string message_;
};

} // namespace wi

#endif // __LOGINFORM_H__
