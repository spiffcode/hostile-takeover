import config
from hashlib import md5
import gamestats

from google.appengine.ext import webapp

class AddGameStats(webapp.RequestHandler):
    def post(self):
        hash = self.request.body[:32]
        j = self.request.body[32:]
        m = md5(j + config.ADDGAMESTATS_SECRET)
        if m.hexdigest() != hash:
            return

        # Update player_stats without dids
        g = gamestats.GameStats(j, load_players=True)

        # Don't save games that only have anonymous players
        if g.get_nonanonymous_human_count() != 0:
            g.save()
