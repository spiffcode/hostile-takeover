import os
import cgi
import models
import urllib
import config
import player
import basehandler
import playerdetail

from google.appengine.ext.webapp import template

SHOW_COUNT_MAX = 100
SHOW_COUNT_INCREMENT = 25

class Leaderboard(basehandler.BaseHandler):
    def get(self):
        # Save this action
        self.save_action()

        # Get player name, if any
        player_name = self.request.get('p')

        # Get the row count and validate
        count_str = self.request.get('count')
        if count_str == '':
            count = 25
        else:
            count = int(count_str)
            if count < 0:
                count = 0
            if count > SHOW_COUNT_MAX:
                count = SHOW_COUNT_MAX 

        # Get row data

        q = models.PlayerModel.all()
        q.filter('blocked =', False)
        q.filter('hidden =', False)
        q.order('-rating')
        results = q.fetch(count)
        players = [player.Player(p, player_name) for p in results]

        # Get game data with one call
        game_keys = []
        for p in players:
            if p.last_game_key_name:
                key_name = p.last_game_key_name
                key = models.gamestatsmodel_key_from_name(key_name)
                game_keys.append(key)
        game_objs = models.GameStatsModel.get(game_keys)
        game_map = {}
        for game_obj in game_objs:
            game_map[game_obj.key().name()] = game_obj

        rows = []
        for index in xrange(len(players)):
            p =  players[index]
            if p.rating == 0:
                break
            row = {}
            row['profile_url'] = p.get_player_stats_url()
            row['rank'] = index + 1
            row['name'] = p.name
            row['avatar_url'] = p.get_avatar_url()
            row['game_count'] = p.game_count
            if game_map.has_key(p.last_game_key_name):
                title = game_map[p.last_game_key_name].mission_title
                row['last_mission_name'] = title
            else:
                row['last_mission_name'] = None
            row['player_rating'] = p.rating
            rows.append(row)

        # Figure out the next show url, if any (Show top N)
        show_count = count + SHOW_COUNT_INCREMENT
        if show_count > SHOW_COUNT_MAX:
            show_count = SHOW_COUNT_MAX
        show_url = ''
        if len(rows) == count and show_count > count:
            show_url = '%s?count=%d' % (config.LEADERBOARD_URL, show_count)
            if player_name != '':
                show_url = '%s&p=%s' % (show_url, player_name)

        # Render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(player_name),
            'selected_tab': config.TAB_LEADERBOARD,
            'rows': rows,
            'chevron_image_url': config.CHEVRON_IMAGE_URL,
            'show_count' : show_count,
            'show_url' : show_url,
        }
        
        self.set_caching_headers(config.INFOPAGE_MAX_AGE_SECONDS)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'leaderboard.xhtml')
        self.response.out.write(template.render(path, template_values))

    def save_action(self):
        try:
            p = self.request.get('p')
            anon = False if p else True
            did = self.request.get('d')
            ip = self.request.remote_addr
            playerdetail.save(p, anon, did, ip, dict(action='leaderboard'))
        except:
            pass
