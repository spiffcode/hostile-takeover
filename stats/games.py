import os
import config
import gamestats
import models
import basehandler

from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

SHOW_COUNT_MAX = 100
SHOW_COUNT_INCREMENT = 25

class Games(basehandler.BaseHandler):
    def get(self):
        # Get player name, if any
        player_name = self.request.get('p')

        # Get user name, if any. If none, use player name
        user_name = self.request.get('u')
        if not user_name:
            user_name = player_name

        # All or not
        all = False
        if self.request.get('a') == '1':
            all = True
        if not player_name and not user_name:
            all = True

        # Get the row count and validate
        count_str = self.request.get('count')
        if count_str == '':
            count = SHOW_COUNT_INCREMENT
        else:
            count = int(count_str)
            if count < 0:
                count = 0
            if count > SHOW_COUNT_MAX:
                count = SHOW_COUNT_MAX 

        # Retrieve gamestats
        rows = self.get_gamestats_rows(player_name, user_name, count, all)

        # Figure out the next show url, if any (Show top N)
        show_count = count + SHOW_COUNT_INCREMENT
        if show_count > SHOW_COUNT_MAX:
            show_count = SHOW_COUNT_MAX
        show_url = ''
        if len(rows) == count and show_count > count:
            show_url = self.get_games_url(player_name, user_name, all,
                    show_count)

        # If player_name and user_name are the same, the Games tab is selected
        selected_tab = config.TAB_NONE
        if player_name == user_name:
            selected_tab = config.TAB_GAMES

        # Render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(player_name),
            'selected_tab': selected_tab,
            'rows': rows,
            'chevron_image_url': config.CHEVRON_IMAGE_URL,
            'winner_small_image_url': config.WINNER_SMALL_IMAGE_URL,
            'show_url': show_url,
            'show_count': show_count,
            'all': all,
            'user_name': user_name,
            'selector_url': self.get_games_url(player_name, user_name, not all)
        }

        self.set_caching_headers(config.INFOPAGE_MAX_AGE_SECONDS)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'games.xhtml')
        self.response.out.write(template.render(path, template_values))

    def get_gamestats_rows(self, player_name, user_name, count, all):
        q = models.GameStatsModel.all()
        if not all and user_name:
            key_name = models.playermodel_key(user_name).name()
            q.filter('player_key_names = ', key_name)
        q.order('-date')
        results = q.fetch(count)
        return [gamestats.GameStats(obj.json, player_name) for obj in results]

    def get_games_url(self, player_name, user_name, all, count = -1):
        d = {}
        if player_name:
            d['p'] = player_name
        if user_name and user_name != player_name:
            d['u'] = user_name
        if all:
            d['a'] = 1
        if count > 0:
            d['count'] = count
        keys = d.keys()
        keys.sort()
        a = '&'.join(['%s=%s' % (k, d[k]) for k in keys])
        if a:
            return '%s?%s' % (config.GAMES_URL, a)
        return config.GAMES_URL
