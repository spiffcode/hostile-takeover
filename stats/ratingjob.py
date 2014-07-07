import os
import time
import models
import config
import random

from google.appengine.ext import webapp
from google.appengine.api import users
from google.appengine.ext import db

# Adjust ratings on this interval
CHECK_INTERVAL = 1 * 60 * 60 * 24 * 7

# A player's rating is stale if the last game is this old
STALE_DELTA = 1 * 60 * 60 * 24 * 7

# Delta to adjust a stale rating
RATING_DELTA = 20

# Ten years
TEN_YEARS = 1 * 60 * 60 * 24 * 365 * 10

class RatingJob(webapp.RequestHandler):
    def get(self):
        now_utc = long(time.time())
        def txn(key):
            p = models.PlayerModel.get(key)
            if p.rating <= 1500:
                p.rating_check_utc = now_utc + random.randint(0, TEN_YEARS)
            elif p.last_game_utc > now_utc - STALE_DELTA:
                p.rating_check_utc = next_rating_check_utc(p.last_game_utc)
            else:
                rating = p.rating - RATING_DELTA
                if rating < 1500:
                    rating = 1500
                p.rating = rating
                p.rating_check_utc = now_utc
            p.put()

        q = db.GqlQuery('SELECT __key__ FROM PlayerModel WHERE rating_check_utc < :1', now_utc - CHECK_INTERVAL)

        for key in q.fetch(100):
            db.run_in_transaction(txn, key)

def next_rating_check_utc(end_utc):
    return end_utc + STALE_DELTA - CHECK_INTERVAL
