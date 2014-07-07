import os
import datetime
import models
import config
import json

from google.appengine.ext import webapp
from google.appengine.ext import db

class GetGames(webapp.RequestHandler):
    def get(self):
        last_end_utc_str = self.request.get('end_utc')
        if last_end_utc_str:
            last_end_utc = int(last_end_utc_str)
        else:
            last_end_utc = 0

        count_str = self.request.get('count')
        if count_str:
            count = int(count_str)
        else:
            count = 1000

        q = db.GqlQuery('SELECT * FROM GameStatsModel WHERE end_utc >= :1 ORDER BY end_utc', last_end_utc)
        results = q.fetch(count)

        self.response.headers['Content-Type'] = 'text/plain'

        filtered_list = []
        for o in results:
            if o.old_ratings:
                ratings_json = json.loads(o.old_ratings)
            else:
                ratings_json = {}
            stats_json = json.loads(o.json)
            filtered_list.append([ratings_json, stats_json])

        self.response.out.write(json.dumps(filtered_list))
