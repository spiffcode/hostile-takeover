import os
import config
import models
import player
import gamestats
import message
import basehandler
import accounts

from google.appengine.ext.webapp import template

class Stats(basehandler.BaseHandler):
    def get(self):
        # player_name is the player that is browsing. The tabs will be
        # populated with query args with this player name
        player_name = self.request.get('p')

        # stats_player is the player to get stats on. If 'u' isn't set,
        # use player_name.
        stats_player = self.request.get('u')
        if stats_player == '':
            stats_player = player_name
        
        p = player.load_from_name(stats_player, player_name)
        if not p:
            message.show(self, message.PLAYER_NOT_FOUND)
            return

        selected_tab = config.TAB_NONE
        if p.is_viewer_also_player():
            selected_tab = config.TAB_STATS

        # get player detail url
        detail_url = ''
        account = accounts.account()
        if account and account.SEE_PLAYER_INFO_ACCESS_RIGHT:
            if player_name:
                detail_url = '%s?p=%s&q=%s' % (config.PLAYERDETAIL_URL, player_name, stats_player)
            else:
                detail_url = '%s?q=%s' % (config.PLAYERDETAIL_URL, stats_player)

        # Fill in the template
        template_values = {
            'tabs': config.get_tabs(player_name, account),
            'selected_tab': selected_tab,
            'player': p,
            'player_default_rating': gamestats.PLAYER_DEFAULT_RATING,
            'detail_url': detail_url
        }

        self.set_caching_headers(config.INFOPAGE_MAX_AGE_SECONDS)
        self.response.headers['Content-Type'] = 'application/xhtml+xml'
        path = os.path.join(os.path.dirname(__file__), 'stats.xhtml')
        self.response.out.write(template.render(path, template_values))
