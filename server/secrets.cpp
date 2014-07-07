#include "secrets.h"

// Keep in sync with stats/config.py secrets
// Secrets should be in a shared file rather than maintained this way

namespace wi {

// For updating the leaderboard with this server's current state
const char *kszServerInfoSecret = "REPLACEME_SERVERINFOSECRET";

// For posting game results to leaderboard
const char *kszStatSecret = "REPLACEME_ADDGAMESTATSSECRET";

// For validating auth tokens
const char *kszTokenAuthSecret = "REPLACEME_AUTHGOODSECRET";

}
