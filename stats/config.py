import os

# Misc config items

def is_debug():
    try:
        return os.environ['SERVER_SOFTWARE'].split('/')[0] == 'Development'
    except:
        return False

# Caching timeout is the same for all info pages

INFOPAGE_MAX_AGE_SECONDS = 5 * 60

# Must match mpshared/side.h:kcSides

SIDE_COUNT_MAX = 5

# Must match mpshared/side.h:kutMax. Changing it means upgrading the db or
# at least handling the length differences gracefully

BUILT_COUNTS_MAX = 23

ADDGAMESTATS_URL = '/api/addgamestats'
AUTH_URL = '/api/auth'
SERVERINFO_URL = '/api/serverinfo'
SYNCERROR_URL = '/api/syncerror'
SENDCOMMAND_URL = '/api/sendcommand'
RATINGJOB_URL = '/api/ratingjob'
GETGAMES_URL = '/api/getgames'
LEADERBOARD_URL = '/stats/leaderboard'
AVATAR_URL = '/stats/avatar'
STATS_URL = '/stats/stats'
GAMES_URL = '/stats/games'
SEARCH_URL = '/stats/search'
ABOUT_URL = '/stats/about'
GAMEDETAIL_URL = '/stats/gamedetail'
MESSAGE_URL = '/stats/message'
ADMIN_URL = '/private/admin'
SENDCHAT_URL = '/private/sendchat'
DRAIN_URL = '/private/drain'
BLOCKPLAYER_URL = '/private/blockplayer'
HIDEPLAYER_URL = '/private/hideplayer'
RESETPLAYER_URL = '/private/resetplayer'
CREATEACCOUNT_URL = '/accounts/createaccount'
UPDATEACCOUNT_URL = '/accounts/updateaccount'
PLAYERDETAIL_URL = '/private/playerdetail'
ADJUSTSCORE_URL = '/private/adjustscore'
ADMINLOG_URL = '/private/adminlog'

CHEVRON_IMAGE_URL = '/images/list-arrow.gif'
UNDER_CONSTRUCTION_IMAGE_URL = '/images/under_construction.png'
WINNER_IMAGE_URL = '/images/star.png'
WINNER_SMALL_IMAGE_URL = '/images/star_small.png'
ANONYMOUS_AVATAR_URL = '/images/default_avatar.jpg'
COMPUTER_AVATAR_URL = '/images/computer_avatar.png'

# Keep in sync with server/secrets.h
# Should be a loaded file rather than maintained like this.
ADDGAMESTATS_SECRET = 'REPLACEME_ADDGAMESTATSSECRET'
ADDUSER_SECRET = 'REPLACEME_ADDUSERSECRET'
AUTH_GOOD_SECRET = 'REPLACEME_AUTHGOODSECRET'
AUTH_BAD_SECRET = 'REPLACEME_AUTHBADSECRET'
SERVERINFO_SECRET = 'REPLACEME_SERVERINFOSECRET'
SENDCOMMAND_SECRET = 'REPLACEME_SENDCOMMANDSECRET'

TAB_NONE = -1
TAB_LEADERBOARD = 0
TAB_STATS = 1
TAB_GAMES = 2
TAB_SEARCH = 3
TAB_ABOUT = 4
TAB_ADMIN = 5

import accounts
def get_tabs(player, account=accounts.account()):
    tabs = []
    tabs.append({ 'title': 'Leaderboard', 'id' : TAB_LEADERBOARD,
            'url' : LEADERBOARD_URL })
    if player:
        tabs.append({ 'title': 'Stats', 'id' : TAB_STATS, 'url' : STATS_URL })
    tabs.append({ 'title': 'Games', 'id' : TAB_GAMES, 'url' : GAMES_URL })
    tabs.append({ 'title': 'Search', 'id' : TAB_SEARCH, 'url' : SEARCH_URL })
    tabs.append({ 'title': 'About', 'id' : TAB_ABOUT, 'url' : ABOUT_URL })
    if account and account.ADMIN_LINK_ACCESS_RIGHT:
        tabs.append({ 'title': 'Admin', 'id' : TAB_ADMIN, 'url' : ADMIN_URL })
    if player != '':
        for tab in tabs:
            tab['url'] = '%s?p=%s' % (tab['url'], player)
    return tabs

