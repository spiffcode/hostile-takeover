#include "game/loginhandler.h"
#include "game/simplerequest.h"
#include "game/serviceurls.h"
#include "game/httpservice.h"
#include "mpshared/messages.h"
#include "base/base64.h"
#include <ctype.h>

namespace wi {

const dword knGetTokenResultSuccess = 0;
const dword knGetTokenResultNoPassword = 1;
const dword knGetTokenResultTokenInvalid = 2;
const dword knGetTokenResultCreateRequestError = 3;
const dword knGetTokenResultServiceUnavailable = 4;

LoginHandler::LoginHandler() : loggedin_(false) {
    InitFromGlobals();
}

LoginHandler::~LoginHandler() {
}

void LoginHandler::InitFromGlobals() {
    strncpyz(token_, gszToken, sizeof(token_));
    strncpyz(password_, gszPassword, sizeof(password_));
    strncpyz(username_, gszUsername, sizeof(username_));
#if 1 // Always show non-anonymous login
    SetAnonymous(0);
#else
    SetAnonymous(gfAnonymous);
#endif
}

void LoginHandler::SaveToGlobals() {
    strncpyz(gszToken, token_, sizeof(gszToken));
    strncpyz(gszPassword, password_, sizeof(gszPassword));
    strncpyz(gszUsername, username_, sizeof(gszUsername));
    gfAnonymous = anonymous_;
    ggame.SavePreferences();
}

void LoginHandler::SetAnonymous(bool anonymous) {
    anonymous_ = anonymous;
}

void LoginHandler::SetUsername(const char *username) {
    strncpyz(username_, username, sizeof(username_));
}

void LoginHandler::SetPassword(const char *password) {
    strncpyz(password_, password, sizeof(password_));
}

void LoginHandler::GetPlayerName(char *psz, int cb) {
    if (anonymous_) {
        if (gptra != NULL) {
            strncpyz(psz, gptra->GetAnonymousUsername(), cb);
        } else {
            strncpyz(psz, "anonymous", cb);
        }
    } else {
        strncpyz(psz, username_, cb);
    }
}

dword LoginHandler::Login() {
    if (gptra == NULL) {
        return knLoginResultFail;
    }

    if (!anonymous_ && token_[0] == 0) {
        switch (GetToken()) {
        case knGetTokenResultSuccess:
            break;
        case knGetTokenResultNoPassword:
            return knLoginResultNoPassword;
        case knGetTokenResultTokenInvalid:
        case knGetTokenResultCreateRequestError:
            return knLoginResultFail;
        case knGetTokenResultServiceUnavailable:
            return knLoginResultAuthDown;
        }
    }

    dword result = gptra->Login(username_, token_);

    // If successful, return and keep the token
    if (result == knLoginResultSuccess ||
            result == knLoginResultAnonymousSuccess) {
        loggedin_ = true;
        SaveToGlobals();
        return result;
    }

    // Otherwise don't save the token
    token_[0] = 0;

    // If token stale, try again otherwise return
    if (result != knLoginResultStaleToken) {
        return result;
    }

    // Get a new token
    if (!GetToken()) {
        switch (GetToken()) {
        case knGetTokenResultSuccess:
            break;
        case knGetTokenResultNoPassword:
            return knLoginResultNoPassword;
        case knGetTokenResultTokenInvalid:
        case knGetTokenResultCreateRequestError:
            return knLoginResultFail;
        case knGetTokenResultServiceUnavailable:
            return knLoginResultAuthDown;
        }
    }

    result = gptra->Login(username_, token_);

    // If successful, return and keep the token
    if (result == knLoginResultSuccess ||
            result == knLoginResultAnonymousSuccess) {
        loggedin_ = true;
        SaveToGlobals();
        return result;
    }

    // Otherwise don't save the token
    token_[0] = 0;

    // If this token is stale for some reason, there is a bug.
    // Convert into "fail".
    if (result == knLoginResultStaleToken) {
        return knLoginResultFail;
    }

    return result;
}

dword LoginHandler::SignOut() {
    loggedin_ = false;
    token_[0] = 0;

    if (!anonymous_) {
        // Sign out user - erase password, keep username
        password_[0] = 0;
    }
    SaveToGlobals();
    if (gptra == NULL) {
        return knSignOutResultFail;
    }
    return gptra->SignOut();
}

dword LoginHandler::GetToken() {
    // Wipe out the current token, if there is one
    token_[0] = 0;

    // Don't send a request for a token if the user has no password
    if (password_[0] == 0) {
        return knGetTokenResultNoPassword;
    }

    // Ask the authentication server for an auth token. Use https
    // so ok to send password cleartext. Encode base64 to avoid
    // url encoding parameters.

    const char *s = base::Format::ToString("%s#%s", username_, password_);
    char input[sizeof(username_) + sizeof(password_)];
    strncpyz(input, s, sizeof(input));
    int cchInput = (int)strlen(input);
    input[strlen(username_)] = 0;
    char output[(sizeof(username_) + sizeof(password_)) * 2];
    int cb = base::base64encode((const byte *)input, cchInput, (byte *)output,
            sizeof(output));
    if (cb == -1) {
        return knGetTokenResultCreateRequestError;
    }
    output[cb] = 0;

    // Submit a simple blocking request to get the token
    
    SimpleRequest req(gphttp);
    std::string d = base::StringEncoder::QueryEncode(gszDeviceId);
    std::string o(base::StringEncoder::QueryEncode(HostGetPlatformString()));
    const char *url = base::Format::ToString("%s?a=%s&d=%s&o=%s", kszAuthUrl, output,
        d.c_str(), o.c_str());
    req.SetTimeout(60);

    char result[kcbTokenMax];
    if (!req.Get(url, result, sizeof(result))) {
        return knGetTokenResultServiceUnavailable;
    }
    int cchResult = (int)strlen(result);

    // Strip whitespace from the start and end.
    char *start = result;
    for (; start < &result[cchResult]; start++) {
        if (!isspace(*start)) {
            break;
        }
    }
    char *end = &result[cchResult - 1];
    for (; end > start; end--) {
        if (!isspace(*end)) {
            end++;
            break;
        }
    }
   
    if (end <= start) { 
        return knGetTokenResultTokenInvalid;
    }
    strncpyz(token_, start, (int)(end - start + 1));
    return knGetTokenResultSuccess;
}

bool LoginHandler::ShouldAttemptLogin() {
    if (anonymous_) {
        return true;
    }
    return username_[0] != 0 && password_[0] != 0;
}

const char *LoginHandler::StatsUsername() {
    // Return the username to use for stats. Only return username if:
    // 1. The user is not anonymous
    // 2. It has been used to login successfully before
    if (anonymous_) {
        return "";
    }
    if (token_[0] == 0) {
        return "";
    }
    return username_;
}

} // namespace wi
