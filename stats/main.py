import addgamestats
import auth
import leaderboard
import avatar
import config
import games
import stats
import search
import gamedetail
import message
import admin
import createaccount
import updateaccount
import syncerror
import serverinfo
import sendchat
import drain
import sendcommand
import ratingjob
import about
import blockplayer
import resetplayer
import getgames
import playerdetail
import adjustscore
import adminlog
import hideplayer

from google.appengine.ext import webapp
from google.appengine.ext.webapp.util import run_wsgi_app

def main():
    handlers = [
            (config.ADDGAMESTATS_URL, addgamestats.AddGameStats),
            (config.AUTH_URL, auth.AuthUser),
            (config.LEADERBOARD_URL, leaderboard.Leaderboard),
            (config.AVATAR_URL, avatar.AvatarHandler),
            (config.STATS_URL, stats.Stats),
            (config.GAMES_URL, games.Games),
            (config.SEARCH_URL, search.Search),
            (config.ABOUT_URL, about.About),
            (config.GAMEDETAIL_URL, gamedetail.GameDetail),
            (config.MESSAGE_URL, message.Message),
            (config.CREATEACCOUNT_URL, createaccount.CreateAccount),
            (config.UPDATEACCOUNT_URL, updateaccount.UpdateAccount),
            (config.SERVERINFO_URL, serverinfo.ServerInfo),
            (config.SYNCERROR_URL, syncerror.SyncError),
            (config.ADMIN_URL, admin.Admin),
            (config.SENDCHAT_URL, sendchat.SendChat),
            (config.DRAIN_URL, drain.Drain),
            (config.SENDCOMMAND_URL, sendcommand.SendCommand),
            (config.BLOCKPLAYER_URL, blockplayer.BlockPlayer),
            (config.HIDEPLAYER_URL, hideplayer.HidePlayer),
            (config.RESETPLAYER_URL, resetplayer.ResetPlayer),
            (config.RATINGJOB_URL, ratingjob.RatingJob),
            (config.GETGAMES_URL, getgames.GetGames),
            (config.PLAYERDETAIL_URL, playerdetail.PlayerDetail),
            (config.ADJUSTSCORE_URL, adjustscore.AdjustScore),
            (config.ADMINLOG_URL, adminlog.AdminLog),
    ]
    application = webapp.WSGIApplication(handlers, debug=config.is_debug())
    run_wsgi_app(application)

if __name__ == "__main__":
    main()
