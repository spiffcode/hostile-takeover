#include "serviceurls.h"

namespace wi {

#ifdef DEBUG
const char *kszIndexUrl = "http://localhost/wi/index";
const char *kszPackInfoUrl = "http://localhost/wi/info";
const char *kszPackUrl = "http://localhost/wi/pack";
const char *kszAuthUrl = "http://localhost:8080/api/auth";
const char *kszRegisterUrl = "http://localhost:8080/accounts/createaccount";
const char *kszUpdateAccountUrl = "http://localhost:8080/accounts/updateaccount";
const char *kszLeaderboardUrl = "http://localhost:8080/stats/leaderboard";
const char *kszServerInfoUrl = "http://localhost:8080/api/serverinfo";
const char *kszSyncErrorUploadUrl = "<REPLACE ME: sync error upload test URL>";
#else
// TODO: Hostile Takeover should have its own server hosting its Mission Packs.
// In the meantime grab them from the Warfare Incorporated server.
const char *kszIndexUrl = "http://content.warfareincorporated.com/wi/index";
const char *kszPackInfoUrl = "http://content.warfareincorporated.com/wi/info";
const char *kszPackUrl = "http://content.warfareincorporated.com/wi/pack";

// TODO: Hostile Takeover needs multiplayer accounts, leaderboard, and server status functionality.
const char *kszAuthUrl = "https://<GAE APPNAME>.appspot.com/api/auth";
const char *kszRegisterUrl = "https://<GAE APPNAME>.appspot.com/accounts/createaccount";
const char *kszUpdateAccountUrl = "https://<GAE APPNAME>.appspot.com/accounts/updateaccount";
const char *kszLeaderboardUrl = "http://<GAE APPNAME>.appspot.com/stats/leaderboard";
const char *kszServerInfoUrl = "http://<GAE APPNAME>.appspot.com/api/serverinfo";
const char *kszSyncErrorUploadUrl = "http://<GAE APPNAME>.appspot.com/api/syncerror";
#endif

} // namespace wi
