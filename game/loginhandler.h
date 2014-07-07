#ifndef __LOGINHANDLER_H__
#define __LOGINHANDLER_H__

#include "game/ht.h"

namespace wi {

class LoginHandler {
public:
    LoginHandler();
    ~LoginHandler();

    dword Login();
    dword SignOut();
    
    void SetUsername(const char *username);
    const char *username() { return username_; }
    void SetPassword(const char *password);
    const char *password() { return password_; }
    void SetAnonymous(bool anonymous);
    bool anonymous() { return anonymous_; }
    bool loggedin() { return loggedin_; }
    bool ShouldAttemptLogin();
    void GetPlayerName(char *psz, int cb);
    void SaveToGlobals();
    const char *StatsUsername();

private:
    void InitFromGlobals();
    dword GetToken();

    char username_[kcbPlayerName];
    char password_[kcbPlayerName];
    char token_[kcbTokenMax];
    bool anonymous_;
    bool loggedin_;
};

}

#endif // __LOGINHANDLER_H__
