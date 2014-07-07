import os
import config
import models
import gamestats
import message
import addgamestats
import basehandler

from wrap import PropWrap
from wrap import DictWrap

from google.appengine.ext import webapp
from google.appengine.ext.webapp import template
from google.appengine.ext import db

class GameDetail(basehandler.BaseHandler):
    def get(self):
        # If a game key is specified, use that. If no game key and a player
        # is specified, use the last game of that player. If neither, show
        # a message to the user. The common error case is a non-logged in
        # player, who hasn't specified a game.

        # Get player name, if any
        player_name = self.request.get('p')

        # Get game key. If missing, get it from the player's last game
        key_name = self.request.get('g')
        if key_name == '':
            # No game key name. Is there a player?
            if player_name == '':
                message.show(self, message.GAME_NOT_FOUND)
                return
            obj = models.PlayerModel.get(models.playermodel_key(player_name))
            if not obj:
                message.show(self, message.PLAYER_NOT_FOUND)
                return
            key_name = obj.last_game_key_name
            if key_name == '':
                message.show(self, message.PLAYER_NOT_PLAYED_GAME)
                return
        g, game_obj = gamestats.load_from_key_name(key_name, player_name,
                load_players=True)
        if not g:
            message.show(self, message.GAME_NOT_FOUND)
            return

        if self.request.get('j') == '1':
            self.response.headers['Content-Type'] = 'text/plain'
            self.response.out.write(g.json)
            return

        if self.request.get('i') == '1':
            self.response.headers['Content-Type'] = 'text/plain'
            for player_stat in g.player_stats:
                ip = 'unknown'
                if 'ip' in player_stat.__dict__ and len(player_stat.ip) != 0:
                    ip = player_stat.ip
                did = 'unknown'
                if 'did' in player_stat.__dict__ and len(player_stat.did) != 0:
                    did = player_stat.did
                winner = ''
                if player_stat.win_result == gamestats.PLAYER_RESULT_WIN:
                    winner = ' (winner)'
                self.response.out.write('Player: %s ip: %s did: %s %s\n' % (player_stat.name, ip, did, winner))
            if game_obj.dids:
                self.response.out.write('\n')
                self.response.out.write(game_obj.dids)
            return

        # Render the template and serve the response
        template_values = {
            'tabs': config.get_tabs(player_name),
            'selected_tab': config.TAB_NONE,
            'gamestats': g,
            'units_built_sums': self.get_units_built_sums(g),
            'computer_default_rating': gamestats.COMPUTER_DEFAULT_RATING,
            'anonymous_default_rating': gamestats.ANONYMOUS_DEFAULT_RATING,
            'player_default_rating': gamestats.PLAYER_DEFAULT_RATING,
            'computer_avatar_url': config.COMPUTER_AVATAR_URL,
            'anonymous_avatar_url': config.ANONYMOUS_AVATAR_URL,
            'winner_image_url': config.WINNER_IMAGE_URL,
            'chevron_image_url': config.CHEVRON_IMAGE_URL,
        }

        self.set_caching_headers(config.INFOPAGE_MAX_AGE_SECONDS)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'gamedetail.xhtml')
        self.response.out.write(template.render(path, template_values))

    def get_units_built_sums(self, g):
        units_built_sums = [0] * config.BUILT_COUNTS_MAX
        for i in xrange(config.BUILT_COUNTS_MAX):
            for player_stat in g.player_stats:
                units_built_sums[i] += player_stat.winstats.built_counts[i]
        return units_built_sums

